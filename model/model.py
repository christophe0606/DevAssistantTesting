# Shader formulas: MIT, Inigo Quilez; notice in primitives.py and shadertoy.txt.
"""Recurrent image layers, no data-dependent control flow in exported graphs.

Python/C++ orchestrate iterations, but SDF evaluation, hit masks, ray updates,
normals, AO, shadows, lighting and 2x enlargement are tensor operations.
"""
from dataclasses import dataclass
import json
import math
from pathlib import Path
import torch
from torch import nn
from primitives import c, v, dot, length, normalize, xz, scene

STRIP_ROWS = 8
NORMAL_EPSILON = 0.0005


@dataclass(frozen=True)
class RenderConfig:
    display_width: int = 480
    display_height: int = 800
    rotation_degrees: int = 90
    tile_width: int = 80
    tile_height: int = 8
    normal_epsilon: float = 0.0005

    @classmethod
    def load(cls):
        return cls(**json.loads(Path(__file__).with_name('render_config.json').read_text()))

    def __post_init__(self):
        if self.rotation_degrees not in (0,90):
            raise ValueError('Rotation must be 0 or 90 degrees clockwise')
        if any(n<=0 or n%2 for n in (self.display_width,self.display_height,self.tile_width,self.tile_height)):
            raise ValueError('Display and tile dimensions must be positive even integers')
        if self.tile_width>self.image_width or self.tile_height>self.image_height:
            raise ValueError('Tiles cannot exceed the logical image')

    @property
    def image_width(self):
        return self.display_height if self.rotation_degrees==90 else self.display_width

    @property
    def image_height(self):
        return self.display_width if self.rotation_degrees==90 else self.display_height

    def to_display(self,rgb):
        """Rotate an NHWC logical image into the physical LCD's row order."""
        return torch.rot90(rgb,-1,dims=(1,2)) if self.rotation_degrees==90 else rgb

    def shape(self,half):
        # A 1-pixel halo on all four sides becomes two pixels after 2x resize.
        return ((self.tile_height//2+2,self.tile_width//2+2) if half else (self.tile_height,self.tile_width))


def split(x, widths):
    return torch.split(x, widths, dim=-1)


def normal(p, epsilon=NORMAL_EPSILON):
    # Pack all four tetrahedral samples into the image width. This retains
    # the shader's four SDF evaluations but exports the scene graph once.
    # Concatenating channels before reshaping avoids a rank-five NPU tensor.
    directions = [c(p,*e)*0.5773 for e in [(1,-1,-1),(-1,-1,1),(-1,1,-1),(1,1,1)]]
    samples = v(*(p+epsilon*e for e in directions))
    packed = samples.reshape(*p.shape[:-2],p.shape[-2]*4,3)
    distances = scene(packed)[0].reshape(*p.shape[:-1],4)
    n = torch.zeros_like(p)
    for i,e in enumerate(directions):
        n = n+e*distances[...,i:i+1]
    return normalize(n)


class March(nn.Module):
    """State = position, direction, t, tmax, material, active (10 channels)."""
    def forward(self, state):
        p,rd,t,tmax,m,active = split(state,[3,3,1,1,1,1])
        d,material = scene(p)
        live = (active>0.5)&(t<tmax)
        hit = live & (d.abs()<0.0001*t)
        advance = live & ~hit
        step = torch.where(advance,d,0)
        p = p+rd*step
        t = t+step
        m = torch.where(hit,material,m)
        active = torch.where(advance & (t<tmax),torch.ones_like(active),torch.zeros_like(active))
        return v(p,rd,t,tmax,m,active)


class Normals(nn.Module):
    def __init__(self,epsilon=NORMAL_EPSILON):
        super().__init__()
        self.epsilon=epsilon
    def forward(self, p):
        return normal(p,self.epsilon)


class AO(nn.Module):
    """State = position, normal, occlusion sum, h, scale, active."""
    def forward(self, state):
        p,n,occ,h,scale,active = split(state,[3,3,1,1,1,1])
        d,_ = scene(p+h*n)
        occ = occ+torch.where(active>0.5,(h-d)*scale,0)
        active = torch.where((active>0.5)&(occ<=0.35),torch.ones_like(active),torch.zeros_like(active))
        return v(p,n,occ,h+0.03,scale*0.95,active)


class Shadow(nn.Module):
    """State = position along ray, direction, t, tmax, visibility, active."""
    def forward(self, state):
        p,rd,t,tmax,visibility,active = split(state,[3,3,1,1,1,1])
        h,_ = scene(p)
        visibility = torch.where(active>0.5,torch.minimum(visibility,(8*h/t).clamp(0,1)),visibility)
        step = torch.where(active>0.5,h.clamp(0.01,0.2),0)
        p,t = p+rd*step,t+step
        active = torch.where((active>0.5)&(visibility>=0.004)&(t<=tmax),torch.ones_like(active),torch.zeros_like(active))
        return v(p,rd,t,tmax,visibility,active)


def smoothstep(x,a,b):
    x = ((x-a)/(b-a)).clamp(0,1)
    return x*x*(3-2*x)


class Shade(nn.Module):
    """p, normal, rd, m, t, AO, sun/sky visibility, floor checker (15 ch)."""
    def forward(self, state):
        p,n,rd,m,t,occ,sun,sky,checker = split(state,[3,3,3,1,1,1,1,1,1])
        floor = m<1.5
        n = torch.where(floor,c(p,0,1,0),n)
        ref = rd-2*dot(rd,n)*n
        # Material sine depends only on one of 22 fixed material IDs. Use a
        # constant palette selected by masks rather than a quantized sin LUT.
        material = torch.full_like(p,0.2)
        for mid in [26.9,17,25,16.9,55,13.67,49.13,7.1,3,31.9,8,18.4,13.56,23.56,43.5,43.17,11.5,51.8,31.2,46.1,51.7,37]:
            rgb = c(p,*[0.2+0.2*math.sin(mid*2+k) for k in [0,1,2]])
            material = torch.where((m-mid).abs()<0.015,rgb,material)
        col = torch.where(floor,0.15+0.05*checker,material)
        ks = torch.where(floor,0.4,1.)
        lig = normalize(c(p,-.5,.4,-.6))
        hal = normalize(lig-rd)
        dif = dot(n,lig).clamp(0,1)*sun
        spe = dot(n,hal).clamp(0,1)**16*dif*(.04+.96*(1-dot(hal,lig)).clamp(0,1)**5)
        lin = col*2.2*dif*c(p,1.3,1,.7)+5*spe*c(p,1.3,1,.7)*ks
        dif = (0.5+0.5*n[...,1:2]).clamp(0,1).sqrt()*occ
        spe = smoothstep(ref[...,1:2],-.2,.2)*dif*(.04+.96*(1+dot(n,rd)).clamp(0,1)**5)*sky
        lin = lin+col*.6*dif*c(p,.4,.6,1.15)+2*spe*c(p,.4,.6,1.3)*ks
        dif = dot(n,normalize(c(p,.5,0,.6))).clamp(0,1)*(1-p[...,1:2]).clamp(0,1)*occ
        lin = lin+col*.55*dif*.25+col*.25*(1+dot(n,rd)).clamp(0,1)**2*occ
        fog = 1-torch.exp(-.0001*t*t*t)
        lin = lin*(1-fog)+c(p,.7,.7,.9)*fog
        background = c(p,.7,.7,.9)-rd[...,1:2].clamp_min(0)*.3
        return torch.where(m>-.5,lin,background).clamp(0,1)**.4545


class Upscale(nn.Module):
    def forward(self, rgb):
        return nn.functional.interpolate(rgb.permute(0,3,1,2),scale_factor=2,mode='bilinear',align_corners=False).permute(0,2,3,1)


def rays(width,height,time=0.,mouse_x=0.,row=0,rows=None,col=0,cols=None):
    """Pixel centers with GLSL's bottom-up y; display rows run top-down."""
    rows = rows or height
    cols = cols or width
    ta = torch.tensor([.25,-.75,-.75])
    phase = .1*(32+time*1.5)+7*mouse_x
    ro = ta+torch.tensor([4.5*math.cos(phase),2.2,4.5*math.sin(phase)])
    cw = normalize(ta-ro)
    cu = normalize(torch.linalg.cross(cw,torch.tensor([0.,1.,0.])))
    cv = torch.linalg.cross(cu,cw)
    xx = torch.arange(col,col+cols).clamp(0,width-1)+.5
    yy = height-(torch.arange(row,row+rows).clamp(0,height-1)+.5)
    y,x = torch.meshgrid(yy,xx,indexing='ij')
    def ray(dx,dy):
        q = torch.stack(((2*(x+dx)-width)/height,(2*(y+dy)-height)/height,torch.full_like(x,2.5)),dim=-1)
        q = normalize(q)
        return (q[...,0:1]*cu+q[...,1:2]*cv+q[...,2:3]*cw).unsqueeze(0)
    return ro,ray(0,0),ray(1,0),ray(0,1)


def safe_divisor(x):
    return torch.where(x.abs()<1e-7,torch.where(x<0,-1e-7,1e-7),x)


def initial_state(ro,rd):
    ry = safe_divisor(rd[...,1:2])
    tp = -ro[1]/ry
    m = torch.where(tp>0,1.,-1.)
    tmax = torch.where(tp>0,tp.clamp_max(20),20.)
    inv = 1/safe_divisor(rd)
    n = inv*(ro-c(rd,0,.4,-.5))
    k = inv.abs()*c(rd,2.5,.41,3)
    near,far = (-n-k).amax(-1,keepdim=True),(-n+k).amin(-1,keepdim=True)
    live = (near<far)&(far>0)&(near<tmax)
    t = torch.where(live,near.clamp_min(1),1.)
    tmax = torch.where(live,torch.minimum(far,tmax),1.)
    # Save plane/miss distances separately: a marched miss keeps the plane.
    fallback = torch.where(tp>0,tp,-1.)
    p = torch.where(live,ro+rd*t,c(rd,0,.5,0))
    state = v(p,rd,t,tmax,m,live.to(rd.dtype))
    return state,fallback


def checker_value(ro,rd,rdx,rdy,p):
    dpdx = ro[1]*(rd/safe_divisor(rd[...,1:2])-rdx/safe_divisor(rdx[...,1:2]))
    dpdy = ro[1]*(rd/safe_divisor(rd[...,1:2])-rdy/safe_divisor(rdy[...,1:2]))
    q = 3*xz(p)
    w = 3*(xz(dpdx).abs()+xz(dpdy).abs())+.001
    i = 2*((torch.frac((q-.5*w)*.5).remainder(1)-.5).abs()-(torch.frac((q+.5*w)*.5).remainder(1)-.5).abs())/w
    return .5-.5*i[...,0:1]*i[...,1:2]


def render(width,height,time=0.,mouse_x=0.,invoke=None,row=0,rows=None,col=0,cols=None,normal_epsilon=NORMAL_EPSILON):
    """Float oracle and calibration runner; invoke can supply quantized layers."""
    modules = {'march':March(),'normals':Normals(normal_epsilon),'ao':AO(),'shadow':Shadow(),'shade':Shade()}
    call = invoke or (lambda name,x: modules[name](x))
    ro,rd,rdx,rdy = rays(width,height,time,mouse_x,row,rows,col,cols)
    state,fallback = initial_state(ro,rd)
    for _ in range(70):
        if not bool((state[...,9:10]>.5).any()):
            break  # Strip-wide completion, outside the exported graph.
        state = call('march',state)
    m = state[...,8:9]
    t = torch.where(m<1.5,fallback,state[...,6:7])
    p = ro+rd*t
    # Safe dummy points keep off-scene sky/plane rays out of calibration and
    # avoid wasting entire normal/AO strips when they contain no objects.
    object_mask = m>=1.5
    dummy = c(p,0,.5,0).expand_as(p)
    np = torch.where(object_mask,p,dummy)
    n = call('normals',np) if bool(object_mask.any()) else torch.zeros_like(p)
    n = torch.where(object_mask,n,c(p,0,1,0))
    hit = m>-.5
    p_safe = torch.where(hit,p,dummy)
    ao_state = v(p_safe,n,torch.zeros_like(m),torch.full_like(m,.01),torch.ones_like(m),hit.to(m.dtype))
    for _ in range(5):
        if not bool((ao_state[...,9:10]>.5).any()):
            break  # No remaining AO contributions anywhere in this tile.
        ao_state = call('ao',ao_state)
    occ = (1-3*ao_state[...,6:7]).clamp(0,1)*(.5+.5*n[...,1:2])
    def shadows(direction):
        tp = (.8-p_safe[...,1:2])/safe_divisor(direction[...,1:2])
        limit = torch.where(tp>0,tp.clamp_max(2.5),2.5)
        s = v(p_safe+.02*direction,direction,torch.full_like(m,.02),limit,torch.ones_like(m),hit.to(m.dtype))
        for _ in range(24):
            if not bool((s[...,9:10]>.5).any()):
                break
            s = call('shadow',s)
        x = s[...,8:9].clamp(0,1)
        return x*x*(3-2*x)
    sun = shadows(normalize(c(p,-.5,.4,-.6)).expand_as(p))
    sky = shadows(rd-2*dot(rd,n)*n)
    checker = checker_value(ro,rd,rdx,rdy,p)
    return call('shade',v(p_safe,n,rd,m,t,occ,sun,sky,checker))


@dataclass
class MethodSpec:
    name: str
    module: nn.Module
    samples: list
    activation_bits: int = 16

    @property
    def example(self):
        return self.samples[0]


def get_methods(widths=None,rows=STRIP_ROWS,config=None):
    """Calibrate actual recurrent states at multiple camera poses and rows."""
    result = []
    config = config or RenderConfig.load()
    variants = [(config.image_width,config.image_height,False),(config.image_width//2,config.image_height//2,True)] if widths is None else [(w,max(1,w*config.image_height//config.image_width),False) for w in widths]
    for width,height,half in variants:
        tile_rows,tile_cols = config.shape(half) if widths is None else (rows,width)
        suffix = 'half' if half or widths is not None else 'full'
        modules = {'march':March(),'normals':Normals(config.normal_epsilon),'ao':AO(),'shadow':Shadow(),'shade':Shade()}
        samples = {name:[] for name in modules}
        poses = [(0,0,height//3,0),(12,0,height//2,width//3),(30,.15,height//2,width//2),(0,0,height-tile_rows,width-tile_cols),(0,0,0,0)]
        for time,mouse,row,col in poses:
            counts = {name:0 for name in modules}
            def collect(name,x):
                # Representative initial, middle, and near-surface states.
                if counts[name] in (0,3,12,30,60):
                    samples[name].append((x.detach().clone(),))
                counts[name]+=1
                return modules[name](x)
            with torch.no_grad():
                render(width,height,time,mouse,collect,row,tile_rows,col,tile_cols,config.normal_epsilon)
        # Calibration must include an object-containing tile for normals.
        if not samples['normals']:
            sample=torch.rand(1,tile_rows,tile_cols,3)*c(torch.zeros(1),5,.8,5)+c(torch.zeros(1),-2.5,0,-3)
            samples['normals']=[(sample,)]
        for name,module in modules.items():
            result.append(MethodSpec(name+'_'+suffix,module.eval(),samples[name]))
    tile_rows,tile_cols=config.shape(True) if widths is None else (rows+2,widths[-1])
    shape = (1,tile_rows,tile_cols,3)
    g = torch.Generator().manual_seed(17)
    result.append(MethodSpec('upscale',Upscale().eval(),[(torch.zeros(shape),),(torch.ones(shape),),(torch.rand(shape,generator=g),)],8))
    return result
