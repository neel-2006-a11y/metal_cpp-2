//
//  fluid.metal
//  metal_cpp
//
//  Created by Neel on 19/06/26.
//

#include <metal_stdlib>
using namespace metal;

struct FluidParams{
    int N;
    float diff;
    float dt;
    int b;
};

struct InteractParams {
    uint N;
    int x, y;
    int size; // square dim 2*size + 1
    float val;
};
//void set_bnd(int N, int b, float* x){
//    int i;
//    for(i=1; i<=N; i++){
//        x[IX(0,i,N)] =   b == 1 ? -x[IX(1,i,N)] : x[IX(1,i,N)];
//        x[IX(N+1,i,N)] = b == 1 ? -x[IX(N,i,N)] : x[IX(N,i,N)];
//        x[IX(i,0,N)] =   b == 2 ? -x[IX(i,1,N)] : x[IX(i,1,N)];
//        x[IX(i,N+1,N)] = b == 2 ? -x[IX(i,N,N)] : x[IX(i,N,N)];
//    }
//    x[IX(0,0,N)] = 0.5 * (x[IX(1,0,N)] +x[IX(0,1,N)]);
//    x[IX(0,N+1,N)] = 0.5 * (x[IX(1,N+1,N)] +x[IX(0,N,N)]);
//    x[IX(N+1,0,N)] = 0.5 * (x[IX(N,0,N)] +x[IX(N+1,1,N)]);
//    x[IX(N+1,N+1,N)] = 0.5 * (x[IX(N,N+1,N)] +x[IX(N+1,N,N)]);
//}

kernel void add_val(texture2d<float, access::write> x [[texture(0)]],
                     constant InteractParams& ip [[buffer(2)]],
                     uint2 id [[thread_position_in_grid]]
                     ){
    int i0 = ip.x-ip.size, j0=ip.y-ip.size;
    if((i0 + id.x>0) && (i0 + id.x<=ip.N) && (j0 + id.y>0) && (j0 + id.y<=ip.N))
        x.write(float4(ip.val,0,0,0), ushort2(i0+id.x, j0+id.y));
}

kernel void set_bnd_1(
                      texture2d<float, access::read_write> x [[texture(0)]],
                      constant FluidParams& fluidParams [[buffer(1)]],
                      uint id [[thread_position_in_grid]]
                      ){
    int i = id+1;
    x.write(fluidParams.b==1 ? -x.read(ushort2(1,i)) : x.read(ushort2(1,i)), ushort2(0,i));
    x.write(fluidParams.b==1 ? -x.read(ushort2(fluidParams.N,i)) : x.read(ushort2(fluidParams.N,i)), ushort2(fluidParams.N+1,i));
    x.write(fluidParams.b==2 ? -x.read(ushort2(i,1)) : x.read(ushort2(i,1)), ushort2(i,0));
    x.write(fluidParams.b==2 ? -x.read(ushort2(i,fluidParams.N)) : x.read(ushort2(i,fluidParams.N)), ushort2(i,fluidParams.N+1));
}

kernel void set_bnd_2(
                      texture2d<float, access::read_write> x [[texture(0)]],
                      constant FluidParams& fluidParams [[buffer(1)]]
                      ){
    x.write(0.5 * (x.read(ushort2(1,0)) + x.read(ushort2(0,1))), ushort2(0,0));
    x.write(0.5 * (x.read(ushort2(1,fluidParams.N+1)) + x.read(ushort2(0,fluidParams.N))), ushort2(0,fluidParams.N+1));
    x.write(0.5 * (x.read(ushort2(fluidParams.N,0)) + x.read(ushort2(fluidParams.N+1,1))), ushort2(fluidParams.N+1,0));
    x.write(0.5 * (x.read(ushort2(fluidParams.N,fluidParams.N+1)) + x.read(ushort2(fluidParams.N+1,fluidParams.N))), ushort2(fluidParams.N+1,fluidParams.N+1));
}


kernel void add_source(
                       texture2d<float, access::read_write> x [[texture(0)]],
                       texture2d<float, access::read_write> s [[texture(1)]],
                       constant FluidParams& fluidParams [[buffer(1)]],
                       uint2 id [[thread_position_in_grid]]
                       ){
    int i = id.x+1, j = id.y+1;
    x.write(x.read(ushort2(i,j)) + fluidParams.dt * s.read(ushort2(i,j)),ushort2(i,j));
    s.write(float4(0,0,0,0), ushort2(i,j));
}

//void diffuse(int N, int b, float* x, float* x0, float diff, float dt){
//    int i,j,k;
//    float a = dt * diff * N * N;
//    
//    for(k=0; k<20; k++){
//        for(i=1; i<=N; i++){
//            for(j=1; j<=N; j++){
//                x[IX(i,j,N)] = (x0[IX(i,j,N)] + a*(x[IX(i-1,j,N)]+x[IX(i+1,j,N)]+x[IX(i,j-1,N)]+x[IX(i,j+1,N)]))/(1+4*a);
//            }
//        }
//        set_bnd(N,b,x);
//    }
//}

kernel void diffuse_step(
                         texture2d<float, access::write> x [[texture(0)]],
                         texture2d<float> x_old [[texture(1)]],
                         texture2d<float> x0 [[texture(2)]],
                         constant FluidParams& fluidParams [[buffer(1)]],
                         uint2 id [[thread_position_in_grid]]
                         ){
    int i=id.x+1, j=id.y+1;
    float a = fluidParams.dt * fluidParams.diff * fluidParams.N * fluidParams.N;
    float4 new_x = (x0.read(ushort2(i,j)) + a*(x_old.read(ushort2(i-1,j))+x_old.read(ushort2(i+1,j))+x_old.read(ushort2(i, j-1))+x_old.read(ushort2(i,j+1))))/(1+4*a);
    x.write(float4(new_x), ushort2(i,j));
}

//void advect(int N, int b, float* d, float* d0, float* u, float* v, float dt){
//    int i,j,i0,j0,i1,j1;
//    float x,y,s0,t0,s1,t1,dt0;
//    
//    dt0 = dt*N;
//    for(i=1; i<=N; i++){
//        for(j=1; j<=N; j++){
//            x = i- dt0 * u[IX(i,j,N)];
//            y = j- dt0 * v[IX(i,j,N)];
//            
//            if(x<0.5) x = 0.5; if(x>N+0.5) x = N+0.5; i0 = (int)x; i1 = i0+1;
//            if(y<0.5) y = 0.5; if(y>N+0.5) y = N+0.5; j0 = (int)y; j1 = j0+1;
//            
//            s1 = x-i0; s0 = 1-s1; t1 = y-j0; t0 = 1-t1;
//            
//            d[IX(i,j,N)] = s0 * (t0 * d0[IX(i0,j0,N)] + t1 * d0[IX(i0,j1,N)]) +
//                           s1 * (t0 * d0[IX(i1,j0,N)] + t1 * d0[IX(i1,j1,N)]);
//        }
//    }
//    set_bnd(N,b,d);
//}

kernel void advect(
                   texture2d<float, access::write> d [[texture(0)]],
                   texture2d<float> d0 [[texture(1)]],
                   texture2d<float> u [[texture(2)]],
                   texture2d<float> v [[texture(3)]],
                   constant FluidParams& fluidParams [[buffer(1)]],
                   uint2 id [[thread_position_in_grid]]
                   ){
    int i=id.x+1, j=id.y+1;
    float dt0 = fluidParams.dt * fluidParams.N;
    float x = i - dt0 * u.read(id).x;
    float y = j - dt0 * v.read(id).x;
    
    int i0,i1,j0,j1;
    if(x<0.5) x = 0.5; if(x>fluidParams.N+0.5) x = fluidParams.N+0.5; i0 = (int)x; i1 = i0+1;
    if(y<0.5) y = 0.5; if(y>fluidParams.N+0.5) y = fluidParams.N+0.5; j0 = (int)y; j1 = j0+1;
    
    float s1,s0,t1,t0;
    s1 = x-i0; s0 = 1-s1; t1 = y-j0; t0 = 1-t1;
    
    d.write(s0 * (t0 * d0.read(ushort2(i0,j0)) + t1 * d0.read(ushort2(i0,j1))) +
            s1 * (t0 * d0.read(ushort2(i1,j0)) + t1 * d0.read(ushort2(i1,j1))), ushort2(i,j));
}

//void project(int N, float* u, float* v, float* p, float* div){
//    int i,j,k;
//    float h;
//    
//    h = 1.0/N;
//    for ( i=1 ; i<=N ; i++ ) {
//        for ( j=1 ; j<=N ; j++ ) {
//            div[IX(i,j,N)] = -0.5*h*(u[IX(i+1,j,N)]-u[IX(i-1,j,N)]+
//            v[IX(i,j+1,N)]-v[IX(i,j-1,N)]);
//            p[IX(i,j,N)] = 0;
//        }
//    }
//    set_bnd ( N, 0, div ); set_bnd ( N, 0, p );

//    for ( k=0 ; k<20 ; k++ ) {
//        for ( i=1 ; i<=N ; i++ ) {
//            for ( j=1 ; j<=N ; j++ ) {
//                p[IX(i,j,N)] = (div[IX(i,j,N)]+p[IX(i-1,j,N)]+p[IX(i+1,j,N)]+p[IX(i,j-1,N)]+p[IX(i,j+1,N)])/4;
//            }
//        }
//        set_bnd ( N, 0, p );
//    }
//    for ( i=1 ; i<=N ; i++ ) {
//        for ( j=1 ; j<=N ; j++ ) {
//            u[IX(i,j,N)] -= 0.5*(p[IX(i+1,j,N)]-p[IX(i-1,j,N)])/h;
//            v[IX(i,j,N)] -= 0.5*(p[IX(i,j+1,N)]-p[IX(i,j-1,N)])/h;
//        }
//    }
//    set_bnd ( N, 1, u ); set_bnd ( N, 2, v );
//}

kernel void divergence(texture2d<float> u [[texture(0)]],
                       texture2d<float> v [[texture(1)]],
                       
                       texture2d<float, access::write> div [[texture(2)]],
                       constant FluidParams& fluidParams [[buffer(1)]],
                       uint2 id [[thread_position_in_grid]]
                       ){
    int i=id.x+1,j=id.y+1;
    float h = 1.0 / fluidParams.N;
    div.write(-0.5 * h * ( u.read(ushort2(i+1,j)) - u.read(ushort2(i-1,j)) + v.read(ushort2(i,j+1)) - v.read(ushort2(i,j-1))), ushort2(i,j));
}


kernel void p_step(texture2d<float> div [[texture(0)]],
                   texture2d<float> p_old [[texture(1)]],
                   texture2d<float, access::write> p [[texture(2)]],
                   uint2 id [[thread_position_in_grid]]
                   ){
    int i=id.x+1, j=id.y+1;
    p.write((div.read(ushort2(i,j)) + p_old.read(ushort2(i-1,j)) + p_old.read(ushort2(i+1,j)) + p_old.read(ushort2(i,j-1)) + p_old.read(ushort2(i,j+1))) / 4,
            ushort2(i,j));
}

kernel void sub_p(texture2d<float, access::read_write> u [[texture(0)]],
                  texture2d<float, access::read_write> v [[texture(1)]],
                  texture2d<float> p [[texture(2)]],
                  constant FluidParams& fluidParams [[buffer(1)]],
                  uint2 id [[thread_position_in_grid]]
                  ){
    int i=id.x+1,j=id.y+1;
    u.write(u.read(ushort2(i,j)) - 0.5 * fluidParams.N * (p.read(ushort2(i+1,j)) - p.read(ushort2(i-1,j))),
            ushort2(i,j));
    v.write(v.read(ushort2(i,j)) - 0.5 * fluidParams.N * (p.read(ushort2(i,j+1)) - p.read(ushort2(i,j-1))),
            ushort2(i,j));
}
