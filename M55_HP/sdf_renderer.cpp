/* Tile orchestration. All scene evaluations and shading execute in the PTE.
 * CPU prepares camera rays, iteration states, the analytic floor checker,
 * and converts final RGB to the panel's RGB565 format. Shader formulas are
 * MIT (Inigo Quilez): see shadertoy.txt for the complete copyright notice.
 */
#include "sdf_renderer.h"
#include "model_runtime.h"
#include "model_io.h"
#include <algorithm>
#include <cmath>
#include <cstring>

volatile SdfMetrics sdf_metrics;
volatile uint32_t sdf_half_resolution=1;
volatile uint32_t sdf_animate=1;
volatile float sdf_time=0;

namespace {
static_assert(SDF_ROTATION_DEGREES==0 || SDF_ROTATION_DEGREES==90,
              "Supported rotations are 0 and 90 degrees clockwise");
constexpr unsigned image_width=SDF_ROTATION_DEGREES==90?SDF_DISPLAY_HEIGHT:SDF_DISPLAY_WIDTH;
constexpr unsigned image_height=SDF_ROTATION_DEGREES==90?SDF_DISPLAY_WIDTH:SDF_DISPLAY_HEIGHT;
constexpr unsigned framebuffer_index(unsigned x,unsigned y)
{
    // Clockwise: logical (x,y) -> panel (display_width-1-y,x).
    return SDF_ROTATION_DEGREES==90?x*SDF_DISPLAY_WIDTH+SDF_DISPLAY_WIDTH-1-y:
                                  y*SDF_DISPLAY_WIDTH+x;
}
static_assert(framebuffer_index(0,0)==(SDF_ROTATION_DEGREES==90?SDF_DISPLAY_WIDTH-1:0));
static_assert(framebuffer_index(image_width-1,image_height-1)==
              (SDF_ROTATION_DEGREES==90?(SDF_DISPLAY_HEIGHT-1)*SDF_DISPLAY_WIDTH:
                                       SDF_DISPLAY_WIDTH*SDF_DISPLAY_HEIGHT-1));
struct V { float x,y,z; };
V operator+(V a,V b) { return {a.x+b.x,a.y+b.y,a.z+b.z}; }
V operator-(V a,V b) { return {a.x-b.x,a.y-b.y,a.z-b.z}; }
V operator*(V a,float k) { return {a.x*k,a.y*k,a.z*k}; }
float dot(V a,V b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
V unit(V a) { return a*(1/std::sqrt(std::max(dot(a,a),1e-20f))); }
V cross(V a,V b) { return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x}; }
void put(float *p,V a) { p[0]=a.x;p[1]=a.y;p[2]=a.z; }
V get(const float *p) { return {p[0],p[1],p[2]}; }
float safe(float x) { return std::fabs(x)<1e-7f?std::copysign(1e-7f,x):x; }
float clip(float x,float a=0,float b=1) { return std::min(std::max(x,a),b); }
constexpr unsigned capacity=std::max(SDF_FULL_WIDTH*SDF_FULL_HEIGHT,SDF_HALF_WIDTH*SDF_HALF_HEIGHT);
struct Pixel { V rd,rdx,rdy,p,n;float fallback,m,t,occ,sun,sky,checker; };
alignas(32) Pixel pixels[capacity];
alignas(32) float state[capacity*15];
alignas(32) float rgb[capacity*3];
alignas(32) float enlarged[SDF_HALF_WIDTH*SDF_HALF_HEIGHT*12];
V ro,cu,cv,cw;

V ray(int x,int y,unsigned width,unsigned height,int dx=0,int dy=0)
{
    const float fx=std::clamp(x,0,int(width)-1)+.5f+dx;
    const float fy=height-(std::clamp(y,0,int(height)-1)+.5f)+dy;
    V q=unit({(2*fx-width)/height,(2*fy-height)/height,2.5f});
    return cu*q.x+cv*q.y+cw*q.z;
}

int invoke(const char *name,unsigned h,unsigned w,unsigned channels,float *out,unsigned elements)
{
    board_service_input();
    uint32_t start=board_millis();
    int error=model_call(name,state,h,w,channels,out,elements);
    sdf_metrics.inference_ms+=board_millis()-start;
    ++sdf_metrics.model_calls;
    if (error) sdf_metrics.error=error;
    return error;
}

bool any_active(unsigned n)
{
    for (unsigned i=0;i<n;++i) if (state[10*i+9]>.5f) return true;
    return false;
}

float checker(const Pixel &p)
{
    const V a=p.rd*(ro.y/safe(p.rd.y))-p.rdx*(ro.y/safe(p.rdx.y));
    const V b=p.rd*(ro.y/safe(p.rd.y))-p.rdy*(ro.y/safe(p.rdy.y));
    auto axis=[](float q,float ax,float bx) {
        const float w=3*(std::fabs(ax)+std::fabs(bx))+.001f;
        auto f=[](float x) { return x-std::floor(x); };
        return 2*(std::fabs(f((3*q-.5f*w)*.5f)-.5f)-std::fabs(f((3*q+.5f*w)*.5f)-.5f))/w;
    };
    return .5f-.5f*axis(p.p.x,a.x,b.x)*axis(p.p.z,a.z,b.z);
}

int render_tile(unsigned image_w,unsigned image_h,int left,int top,unsigned w,unsigned h,bool half)
{
    const unsigned n=w*h;
    const char *march=half?"march_half":"march_full";
    const char *normals=half?"normals_half":"normals_full";
    const char *ao=half?"ao_half":"ao_full";
    const char *shadow=half?"shadow_half":"shadow_full";
    const char *shade=half?"shade_half":"shade_full";
    for (unsigned y=0;y<h;++y) for (unsigned x=0;x<w;++x) {
        const unsigned i=y*w+x;
        Pixel &p=pixels[i];
        p.rd=ray(left+int(x),top+int(y),image_w,image_h);
        p.rdx=ray(left+int(x),top+int(y),image_w,image_h,1,0);
        p.rdy=ray(left+int(x),top+int(y),image_w,image_h,0,1);
        const float tp=-ro.y/safe(p.rd.y);
        p.fallback=tp>0?tp:-1;
        p.m=tp>0?1:-1;
        float limit=tp>0?std::min(tp,20.f):20.f;
        V origin=ro-V{0,.4f,-.5f};
        const float dir[3]={p.rd.x,p.rd.y,p.rd.z};
        const float ori[3]={origin.x,origin.y,origin.z},rad[3]={2.5f,.41f,3};
        float near=-1e30f,far=1e30f;
        for (unsigned k=0;k<3;++k) {
            float inv=1/safe(dir[k]),a=-ori[k]*inv,b=std::fabs(inv)*rad[k];
            near=std::max(near,a-b);far=std::min(far,a+b);
        }
        bool live=near<far&&far>0&&near<limit;
        float t=live?std::max(near,1.f):1.f;
        float *s=state+10*i;
        put(s,live?ro+p.rd*t:V{0,.5f,0});put(s+3,p.rd);
        s[6]=t;s[7]=live?std::min(far,limit):1.f;s[8]=p.m;s[9]=live?1.f:0.f;
    }
    for (unsigned step=0;step<70&&any_active(n);++step) {
        if (int e=invoke(march,h,w,10,state,n*10)) return e;
        ++sdf_metrics.march_calls;
    }
    bool objects=false;
    for (unsigned i=0;i<n;++i) {
        Pixel &p=pixels[i];p.m=state[10*i+8];p.t=p.m<1.5f?p.fallback:state[10*i+6];
        p.p=ro+p.rd*p.t;
        objects|=p.m>=1.5f;
        put(state+3*i,p.m>=1.5f?p.p:V{0,.5f,0});
    }
    if (objects) {
        if (int e=invoke(normals,h,w,3,state,n*3)) return e;
    }
    for (unsigned i=0;i<n;++i) {
        Pixel &p=pixels[i];p.n=p.m>=1.5f?get(state+3*i):V{0,1,0};
        p.checker=checker(p);
    }
    for (unsigned i=0;i<n;++i) {
        const Pixel &p=pixels[i];float *s=state+10*i;
        put(s,p.m>-.5f?p.p:V{0,.5f,0});put(s+3,p.n);
        s[6]=0;s[7]=.01f;s[8]=1;s[9]=p.m>-.5f?1.f:0.f;
    }
    for (unsigned j=0;j<5&&any_active(n);++j) if (int e=invoke(ao,h,w,10,state,n*10)) return e;
    for (unsigned i=0;i<n;++i) pixels[i].occ=clip(1-3*state[10*i+6])*(.5f+.5f*pixels[i].n.y);
    for (unsigned pass=0;pass<2;++pass) {
        for (unsigned i=0;i<n;++i) {
            const Pixel &p=pixels[i];float *s=state+10*i;
            V pos=p.m>-.5f?p.p:V{0,.5f,0};
            V dir=pass?p.rd-p.n*(2*dot(p.rd,p.n)):unit({-.5f,.4f,-.6f});
            const float tp=(.8f-pos.y)/safe(dir.y);
            put(s,pos+dir*.02f);put(s+3,dir);
            s[6]=.02f;s[7]=tp>0?std::min(tp,2.5f):2.5f;s[8]=1;s[9]=p.m>-.5f?1.f:0.f;
        }
        for (unsigned j=0;j<24&&any_active(n);++j) {
            if (int e=invoke(shadow,h,w,10,state,n*10)) return e;
            ++sdf_metrics.shadow_calls;
        }
        for (unsigned i=0;i<n;++i) {
            const float a=clip(state[10*i+8]);
            (pass?pixels[i].sky:pixels[i].sun)=a*a*(3-2*a);
        }
    }
    for (unsigned i=0;i<n;++i) {
        const Pixel &p=pixels[i];float *s=state+15*i;
        put(s,p.m>-.5f?p.p:V{0,.5f,0});put(s+3,p.n);put(s+6,p.rd);
        s[9]=p.m;s[10]=p.t;s[11]=p.occ;s[12]=p.sun;s[13]=p.sky;s[14]=p.checker;
    }
    return invoke(shade,h,w,15,rgb,n*3);
}

uint16_t rgb565(const float *p)
{
    return (uint16_t(std::lround(clip(p[0])*31))<<11) |
           (uint16_t(std::lround(clip(p[1])*63))<<5) |
           uint16_t(std::lround(clip(p[2])*31));
}
}

extern "C" int sdf_init(void) { return model_init(); }

extern "C" int sdf_render(uint16_t *fb,float time,int half)
{
    sdf_metrics.model_calls=0;sdf_metrics.inference_ms=0;sdf_metrics.march_calls=0;sdf_metrics.shadow_calls=0;sdf_metrics.tiles=0;
    const uint32_t start=board_millis();
    V ta{.25f,-.75f,-.75f};const float phase=.1f*(32+1.5f*time);
    ro=ta+V{4.5f*std::cos(phase),2.2f,4.5f*std::sin(phase)};
    cw=unit(ta-ro);cu=unit(cross(cw,{0,1,0}));cv=cross(cu,cw);
    const unsigned iw=image_width/(half?2:1),ih=image_height/(half?2:1);
    const unsigned w=half?SDF_HALF_WIDTH:SDF_FULL_WIDTH,h=half?SDF_HALF_HEIGHT:SDF_FULL_HEIGHT;
    for (unsigned y=0;y<image_height;y+=SDF_TILE_HEIGHT) {
        for (unsigned x=0;x<image_width;x+=SDF_TILE_WIDTH) {
            int e=render_tile(iw,ih,half?int(x/2)-1:int(x),half?int(y/2)-1:int(y),w,h,half!=0);
            if (e) return e;
            const float *result=rgb;unsigned stride=w;unsigned offset=0;
            if (half) {
                std::memcpy(state,rgb,w*h*3*sizeof(float));
                if ((e=invoke("upscale",h,w,3,enlarged,w*h*12))) return e;
                result=enlarged;stride=w*2;offset=2*stride+2;
            }
            const unsigned valid_w=std::min(unsigned(SDF_TILE_WIDTH),image_width-x);
            const unsigned valid_h=std::min(unsigned(SDF_TILE_HEIGHT),image_height-y);
            for (unsigned yy=0;yy<valid_h;++yy) for (unsigned xx=0;xx<valid_w;++xx)
                fb[framebuffer_index(x+xx,y+yy)]=rgb565(result+3*(offset+yy*stride+xx));
            ++sdf_metrics.tiles;
        }
    }
    sdf_metrics.render_ms=board_millis()-start;++sdf_metrics.frames;
    return 0;
}
