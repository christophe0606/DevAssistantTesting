"""Image-tensor port of the 24 primitives in shadertoy.txt (Xds3zN).

The MIT License
Copyright (c) 2013 Inigo Quilez
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
"""
import math
import torch


def v(*xs):
    return torch.cat(xs, dim=-1)


def c(p, *xs):
    return p.new_tensor(xs)


def dot(a, b):
    return (a*b).sum(dim=-1, keepdim=True)


def length(p):
    # Roundoff can make analytic squared distances very slightly negative.
    return dot(p, p).clamp_min(1e-12).sqrt()


def normalize(p):
    return p / length(p)


def xy(p):
    return p[..., :2]


def xz(p):
    return v(p[..., 0:1], p[..., 2:3])


def box(p, b):
    d = p.abs()-c(p, *b)
    return d.amax(-1, keepdim=True).clamp_max(0)+length(d.clamp_min(0))


def box_frame(p, b, e):
    p = p.abs()-c(p, *b)
    q = (p+e).abs()-e
    x, y, z = p.split(1, -1)
    a, b, d = q.split(1, -1)
    def edge(t):
        return length(t.clamp_min(0))+t.amax(-1, keepdim=True).clamp_max(0)
    return torch.minimum(torch.minimum(edge(v(x,b,d)), edge(v(a,y,d))),edge(v(a,b,z)))


def ellipsoid(p, r):
    r = c(p, *r)
    k0, k1 = length(p/r), length(p/(r*r))
    return k0*(k0-1)/k1


def torus(p, ra, rb):
    return length(v(length(xz(p))-ra,p[...,1:2]))-rb


def capped_torus(p, sc, ra, rb):
    x, y, z = p.split(1,-1)
    x = x.abs()
    k = torch.where(sc[1]*x>sc[0]*y, sc[0]*x+sc[1]*y, length(v(x,y)))
    return (x*x+y*y+z*z+ra*ra-2*ra*k).clamp_min(1e-12).sqrt()-rb


def prism(p, r, h, octagon=False):
    p = p.abs()
    a = xy(p)
    k = (-0.9238795325,0.3826834323,0.4142135623) if octagon else (-0.8660254,0.5,0.57735)
    n = c(p,*k[:2])
    a = a-2*dot(a,n).clamp_max(0)*n
    if octagon:
        n = c(p,-k[0],k[1])
        a = a-2*dot(a,n).clamp_max(0)*n
    a = a-v(a[...,0:1].clamp(-k[2]*r,k[2]*r),torch.full_like(a[...,1:2],r))
    d = v(length(a)*a[...,1:2].sign(),p[...,2:3]-h)
    return d.amax(-1,keepdim=True).clamp_max(0)+length(d.clamp_min(0))


def capsule(p, a, b, r):
    a,b = c(p,*a),c(p,*b)
    pa,ba = p-a,b-a
    h = (dot(pa,ba)/dot(ba,ba)).clamp(0,1)
    return length(pa-ba*h)-r


def round_cone(p,r1,r2,h):
    q = v(length(xz(p)),p[...,1:2])
    b = (r1-r2)/h
    a = math.sqrt(1-b*b)
    k = dot(q,c(p,-b,a))
    d = dot(q,c(p,a,b))-r1
    return torch.where(k<0,length(q)-r1,torch.where(k>a*h,length(q-c(p,0,h))-r2,d))


def round_cone_ab(p,a,b,r1,r2):
    a,b = c(p,*a),c(p,*b)
    ba,pa = b-a,p-a
    l2,rr = dot(ba,ba),r1-r2
    a2,il2 = l2-rr*rr,1/dot(ba,ba)
    y = dot(pa,ba)
    z = y-l2
    x2 = dot(pa*l2-ba*y,pa*l2-ba*y)
    k = math.copysign(rr*rr,rr)*x2
    side = ((x2*a2*il2).clamp_min(1e-12).sqrt()+y*rr)*il2-r1
    da = (x2+y*y*l2).clamp_min(1e-12).sqrt()*il2-r1
    db = (x2+z*z*l2).clamp_min(1e-12).sqrt()*il2-r2
    return torch.where(z.sign()*a2*z*z*l2>k,db,torch.where(y.sign()*a2*y*y*l2<k,da,side))


def tri_prism(p,h):
    k = math.sqrt(3)
    hx = h[0]*0.5*k
    x = p[...,0:1].abs()/hx-1
    y = p[...,1:2]/hx+1/k
    fold = x+k*y>0
    a,b = (x-k*y)*0.5,(-k*x-y)*0.5
    x,y = torch.where(fold,a,x),torch.where(fold,b,y)
    x = x-x.clamp(-2,0)
    d1 = length(v(x,y))*(-y).sign()*hx
    d2 = p[...,2:3].abs()-h[1]
    q = v(d1,d2)
    return length(q.clamp_min(0))+q.amax(-1,keepdim=True).clamp_max(0)


def cylinder(p,r,h):
    d = v(length(xz(p)),p[...,1:2]).abs()-c(p,r,h)
    return d.amax(-1,keepdim=True).clamp_max(0)+length(d.clamp_min(0))


def cylinder_ab(p,a,b,r):
    a,b = c(p,*a),c(p,*b)
    pa,ba = p-a,b-a
    baba,paba = dot(ba,ba),dot(pa,ba)
    x = length(pa*baba-ba*paba)-r*baba
    y = (paba-baba*0.5).abs()-baba*0.5
    x2,y2 = x*x,y*y*baba
    d = torch.where(torch.maximum(x,y)<0,-torch.minimum(x2,y2),torch.where(x>0,x2,0)+torch.where(y>0,y2,0))
    return d.sign()*d.abs().clamp_min(1e-12).sqrt()/baba


def cone(p,co,h):
    q = c(p,h*co[0]/co[1],-h)
    w = v(length(xz(p)),p[...,1:2])
    a = w-q*(dot(w,q)/dot(q,q)).clamp(0,1)
    b = w-q*v((w[...,0:1]/q[0]).clamp(0,1),torch.ones_like(w[...,1:2]))
    s = torch.maximum(-(w[...,0:1]*q[1]-w[...,1:2]*q[0]),-(w[...,1:2]-q[1]))
    return torch.minimum(dot(a,a),dot(b,b)).clamp_min(1e-12).sqrt()*s.sign()


def capped_cone(p,h,r1,r2):
    q = v(length(xz(p)),p[...,1:2])
    k1,k2 = c(p,r2,h),c(p,r2-r1,2*h)
    rad = torch.where(q[...,1:2]<0,r1,r2)
    ca = v(q[...,0:1]-torch.minimum(q[...,0:1],rad),q[...,1:2].abs()-h)
    cb = q-k1+k2*(dot(k1-q,k2)/dot(k2,k2)).clamp(0,1)
    one=torch.ones_like(q[...,0:1])
    s = torch.where((cb[...,0:1]<0)&(ca[...,1:2]<0),-one,one)
    return s*torch.minimum(dot(ca,ca),dot(cb,cb)).clamp_min(1e-12).sqrt()


def capped_cone_ab(p,a,b,ra,rb):
    a,b = c(p,*a),c(p,*b)
    rba = rb-ra
    ba,pa = b-a,p-a
    baba,papa = dot(ba,ba),dot(pa,pa)
    paba = dot(pa,ba)/baba
    x = (papa-paba*paba*baba).clamp_min(1e-12).sqrt()
    cax = (x-torch.where(paba<0.5,ra,rb)).clamp_min(0)
    cay = (paba-0.5).abs()-0.5
    f = ((rba*(x-ra)+paba*baba)/(rba*rba+baba)).clamp(0,1)
    cbx,cby = x-ra-f*rba,paba-f
    one=torch.ones_like(cbx)
    s = torch.where((cbx<0)&(cay<0),-one,one)
    return s*torch.minimum(cax*cax+cay*cay*baba,cbx*cbx+cby*cby*baba).clamp_min(1e-12).sqrt()


def solid_angle(p,co,ra):
    q = v(length(xz(p)),p[...,1:2])
    co = c(p,*co)
    l = length(q)-ra
    m = length(q-co*dot(q,co).clamp(0,ra))
    return torch.maximum(l,m*(co[1]*q[...,0:1]-co[0]*q[...,1:2]).sign())


def octahedron(p,s):
    p = p.abs()
    x,y,z = p.split(1,-1)
    m = x+y+z-s
    q = torch.where(3*x<m,p,torch.where(3*y<m,v(y,z,x),v(z,x,y)))
    a,b,d = q.split(1,-1)
    k = (0.5*(d-b+s)).clamp(0,s)
    # Put the linear branch first. Arm's MatchArgDtypePass otherwise inserts
    # an FP32 cast when the square-root table's dtype has not been updated yet.
    interior = ~((3*x<m)|(3*y<m)|(3*z<m))
    return torch.where(interior,m*0.57735027,length(v(a,b-s+k,d-k)))


def pyramid(p,h):
    x,y,z = p.split(1,-1)
    x,z = x.abs(),z.abs()
    x,z = torch.maximum(x,z)-0.5,torch.minimum(x,z)-0.5
    qx,qy,qz = z,h*y-0.5*x,h*x+0.5*y
    m2 = h*h+0.25
    s = (-qx).clamp_min(0)
    t = ((qy-0.5*z)/(m2+0.25)).clamp(0,1)
    a = m2*(qx+s)**2+qy*qy
    b = m2*(qx+0.5*t)**2+(qy-m2*t)**2
    d2 = torch.where(torch.minimum(qy,-qx*m2-qy*0.5)>0,0,torch.minimum(a,b))
    return ((d2+qz*qz)/m2).clamp_min(1e-12).sqrt()*torch.maximum(qz,-y).sign()


def rhombus(p,la,lb,h,ra):
    p = p.abs()
    x,y,z = p.split(1,-1)
    f = ((la*(la-2*x)-lb*(lb-2*z))/(la*la+lb*lb)).clamp(-1,1)
    q = v(length(v(x-0.5*la*(1-f),z-0.5*lb*(1+f)))*(x*lb+z*la-la*lb).sign()-ra,y-h)
    return q.amax(-1,keepdim=True).clamp_max(0)+length(q.clamp_min(0))


def horseshoe(p,co,r,le,w):
    x,y,z = p.split(1,-1)
    x = x.abs()
    l = length(v(x,y))
    x,y = -co[0]*x+co[1]*y,co[1]*x+co[0]*y
    x,y = torch.where((y>0)|(x>0),x,-l),torch.where(x>0,y,l)
    a = v(x-le,(y-r).abs())
    q = v(length(a.clamp_min(0))+a.amax(-1,keepdim=True).clamp_max(0),z)
    d = q.abs()-c(p,*w)
    return d.amax(-1,keepdim=True).clamp_max(0)+length(d.clamp_min(0))


def scene(p):
    """Dense union: bounding-box branches become image-wide minimum/where layers."""
    d,m = p[...,1:2],torch.zeros_like(p[...,1:2])
    def union(nd,nm):
        nonlocal d,m
        take = nd <= d  # GLSL opU chooses its second argument on ties.
        m = torch.where(take,torch.full_like(m,nm),m)
        d = torch.minimum(d,nd)
    def at(x,y,z):
        return p-c(p,x,y,z)
    union(length(at(-2,.25,0))-.25,26.9)
    q = at(-2,.25,1)
    union(rhombus(v(q[...,0:1],q[...,2:3],q[...,1:2]),.15,.25,.04,.08),17)
    union(capped_torus(at(0,.30,1)*c(p,1,-1,1),(.866025,-.5),.25,.05),25)
    union(box_frame(at(0,.25,0),(.3,.25,.2),.025),16.9)
    union(cone(at(0,.45,-1),(.6,.8),.45),55)
    union(capped_cone(at(0,.25,-2),.25,.25,.1),13.67)
    union(solid_angle(at(0,0,-3),(.6,.8),.4),49.13)
    q = at(1,.3,1)
    union(torus(v(q[...,0:1],q[...,2:3],q[...,1:2]),.25,.05),7.1)
    union(box(at(1,.25,0),(.3,.25,.1)),3)
    union(capsule(at(1,0,-1),(-.1,.1,-.1),(.2,.4,.2),.1),31.9)
    union(cylinder(at(1,.25,-2),.15,.25),8)
    union(prism(at(1,.2,-3),.2,.05),18.4)
    union(pyramid(at(-1,-.6,-3),1),13.56)
    union(octahedron(at(-1,.15,-2),.35),23.56)
    union(tri_prism(at(-1,.15,-1),(.3,.05)),43.5)
    union(ellipsoid(at(-1,.25,0),(.2,.25,.05)),43.17)
    union(horseshoe(at(-1,.25,1),(math.cos(1.3),math.sin(1.3)),.2,.3,(.03,.08)),11.5)
    union(prism(at(2,.2,-3),.2,.05,True),51.8)
    union(cylinder_ab(at(2,.14,-2),(.1,-.1,0),(-.2,.35,.1),.08),31.2)
    union(capped_cone_ab(at(2,.09,-1),(.1,0,0),(-.2,.40,.1),.15,.05),46.1)
    union(round_cone_ab(at(2,.15,0),(.1,0,0),(-.1,.35,.1),.15,.05),51.7)
    union(round_cone(at(2,.20,1),.2,.1,.3),37)
    return d,m
