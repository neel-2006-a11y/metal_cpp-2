//
//  fluid_solver.cpp
//  metal_cpp
//
//  Created by Neel on 18/06/26.
//

#include "fluid_stam/fluid_solver.h"
#include "passers/MTL_Texture_passer.h"

#define SWAP(x0, x) {auto tmp=x0; x0=x; x=tmp;}

inline int IX(int x, int y, int N){
    return x+(N+2)*y;
}


Fluid::Fluid(int N, int steps, TextureManager* texMan, PipelineManager* pipeMan){
    this->texMan = texMan;
    this->pipeMan = pipeMan;
    this->N = N;
    this->steps = steps;
    density_source.resize((N+2) * (N+2));
    density.resize((N+2) * (N+2));
    densityPrev.resize((N+2) * (N+2));
    
    u.resize((N+2) * (N+2));
    v.resize((N+2) * (N+2));
    
    uPrev.resize((N+2) * (N+2));
    vPrev.resize((N+2) * (N+2));
    
    su.resize((N+2) * (N+2));
    sv.resize((N+2) * (N+2));
    
    TextureDesc dens_desc;
    dens_desc.width = N+2;
    dens_desc.height = N+2;
    dens_desc.format = TextureFormat::R32Float;
    dens_desc.storageMode = StorageMode::Shared;
    dens_desc.usage = TextureUsage::Sampled | TextureUsage::ShaderWrite;
    
    int sz = N/20;
    for(int i=-sz; i<=sz; i++){
        for(int j=-sz; j<=sz; j++){
            density_source[IX(N/2 + i, N/2 + j, N)] = N/20;
        }
    }
    
    int szv = N/20;
    for(int i=-szv; i<=szv; i++){
        for(int j=-szv; j<=szv; j++){
            su[IX(N/2 + i, N/2 + j, N)] = N/20;
        }
    }
    
    
    density_source_texture = texMan->createTexture(dens_desc, density_source.data(), false);
//    density_source_texture = texMan->createEmpty(dens_desc);
    density_prev_texture = texMan->createEmpty(dens_desc);
    density_texture = texMan->createEmpty(dens_desc);
    
    u_texture = texMan->createEmpty(dens_desc);
    v_texture = texMan->createEmpty(dens_desc);
    
    uPrev_texture = texMan->createEmpty(dens_desc);
    vPrev_texture = texMan->createEmpty(dens_desc);
    
    su_texture = texMan->createTexture(dens_desc, su.data(), false);
//    su_texture = texMan->createEmpty(dens_desc);
    sv_texture = texMan->createEmpty(dens_desc);
    
    pingPong_texture = texMan->createEmpty(dens_desc);
    
    zero_texture = texMan->createZeroTexture(dens_desc);
    
    // adding dens
//    density_source[IX(N/2, N/2, N)] = 5;
//    sv[IX(N/2,N/2,N)] = 30;
}

const std::vector<float>& Fluid::getDensity() const{
    return density;
}

void Fluid::addDensity(int x, int y, int size, float amount, MTL::CommandBuffer* cmd){
    auto add_val_PS = pipeMan->getCompute(add_val_pipe);
    
    Texture* dens_s_tex = texMan->get(density_source_texture);
    
    InteractParams ip;
    ip.N = N;
    ip.x = x;
    ip.y = y;
    ip.size = size;
    ip.val = amount;
    
    auto enc = cmd->computeCommandEncoder();
    enc->setComputePipelineState(add_val_PS);
    enc->setTexture((MTL::Texture*)dens_s_tex->gpuTexture, 0);
    enc->setBytes(&ip, sizeof(InteractParams), 2);
    
    MTL::Size gridSize = MTL::Size(2*size+1, 2*size+1, 1);
    NS::UInteger w = add_val_PS->threadExecutionWidth();
    NS::UInteger h = add_val_PS->maxTotalThreadsPerThreadgroup() / w;
    MTL::Size tgSize = MTL::Size(w,h,1);
    enc->dispatchThreads(gridSize, tgSize);
    enc->endEncoding();
}

void Fluid::addVelocity(int x, int y, int size, float amountX, float amountY, MTL::CommandBuffer* cmd){
    auto add_val_PS = pipeMan->getCompute(add_val_pipe);
    
    Texture* s_u_tex = texMan->get(su_texture);
    Texture* s_v_tex = texMan->get(sv_texture);
    
    InteractParams ip;
    ip.N = N;
    ip.x = x;
    ip.y = y;
    ip.size = size;
    ip.val = amountX;
    
    MTL::Size gridSize = MTL::Size(2*size+1, 2*size+1, 1);
    NS::UInteger w = add_val_PS->threadExecutionWidth();
    NS::UInteger h = add_val_PS->maxTotalThreadsPerThreadgroup() / w;
    MTL::Size tgSize = MTL::Size(w,h,1);
    
    auto enc = cmd->computeCommandEncoder();
    enc->setComputePipelineState(add_val_PS);
    enc->setTexture((MTL::Texture*)s_u_tex->gpuTexture, 0);
    enc->setBytes(&ip, sizeof(InteractParams), 2);
    
    enc->dispatchThreads(gridSize, tgSize);
    
    ip.val = amountY;
    
    enc->setTexture((MTL::Texture*)s_u_tex->gpuTexture, 0);
    enc->dispatchThreads(gridSize, tgSize);
    enc->endEncoding();
}

void set_bnd(int N, int b, float* x){
    int i;
    for(i=1; i<=N; i++){
        x[IX(0,i,N)] =   b == 1 ? -x[IX(1,i,N)] : x[IX(1,i,N)];
        x[IX(N+1,i,N)] = b == 1 ? -x[IX(N,i,N)] : x[IX(N,i,N)];
        x[IX(i,0,N)] =   b == 2 ? -x[IX(i,1,N)] : x[IX(i,1,N)];
        x[IX(i,N+1,N)] = b == 2 ? -x[IX(i,N,N)] : x[IX(i,N,N)];
    }
    x[IX(0,0,N)] = 0.5 * (x[IX(1,0,N)] +x[IX(0,1,N)]);
    x[IX(0,N+1,N)] = 0.5 * (x[IX(1,N+1,N)] +x[IX(0,N,N)]);
    x[IX(N+1,0,N)] = 0.5 * (x[IX(N,0,N)] +x[IX(N+1,1,N)]);
    x[IX(N+1,N+1,N)] = 0.5 * (x[IX(N,N+1,N)] +x[IX(N+1,N,N)]);
}

void add_source(int N, float* x, float* s, float dt){
    int i, size = (N+2) * (N+2);
    for(i=0; i<size; i++) x[i] += dt*s[i];
}

void diffuse(int N, int b, float* x, float* x0, float diff, float dt){
    int i,j,k;
    float a = dt * diff * N * N;
    
    for(k=0; k<20; k++){
        for(i=1; i<=N; i++){
            for(j=1; j<=N; j++){
                x[IX(i,j,N)] = (x0[IX(i,j,N)] + a*(x[IX(i-1,j,N)]+x[IX(i+1,j,N)]+x[IX(i,j-1,N)]+x[IX(i,j+1,N)]))/(1+4*a);
            }
        }
        set_bnd(N,b,x);
    }
}

void advect(int N, int b, float* d, float* d0, float* u, float* v, float dt){
    int i,j,i0,j0,i1,j1;
    float x,y,s0,t0,s1,t1,dt0;
    
    dt0 = dt*N;
    for(i=1; i<=N; i++){
        for(j=1; j<=N; j++){
            x = i- dt0 * u[IX(i,j,N)];
            y = j- dt0 * v[IX(i,j,N)];
            
            if(x<0.5) x = 0.5; if(x>N+0.5) x = N+0.5; i0 = (int)x; i1 = i0+1;
            if(y<0.5) y = 0.5; if(y>N+0.5) y = N+0.5; j0 = (int)y; j1 = j0+1;
            
            s1 = x-i0; s0 = 1-s1; t1 = y-j0; t0 = 1-t1;
            
            d[IX(i,j,N)] = s0 * (t0 * d0[IX(i0,j0,N)] + t1 * d0[IX(i0,j1,N)]) +
                           s1 * (t0 * d0[IX(i1,j0,N)] + t1 * d0[IX(i1,j1,N)]);
        }
    }
    set_bnd(N,b,d);
}

void project(int N, float* u, float* v, float* p, float* div){
    int i,j,k;
    float h;
    
    h = 1.0/N;
    for ( i=1 ; i<=N ; i++ ) {
        for ( j=1 ; j<=N ; j++ ) {
            div[IX(i,j,N)] = -0.5*h*(u[IX(i+1,j,N)]-u[IX(i-1,j,N)]+
            v[IX(i,j+1,N)]-v[IX(i,j-1,N)]);
            p[IX(i,j,N)] = 0;
        }
    }
    set_bnd ( N, 0, div ); set_bnd ( N, 0, p );
    for ( k=0 ; k<20 ; k++ ) {
        for ( i=1 ; i<=N ; i++ ) {
            for ( j=1 ; j<=N ; j++ ) {
                p[IX(i,j,N)] = (div[IX(i,j,N)]+p[IX(i-1,j,N)]+p[IX(i+1,j,N)]+p[IX(i,j-1,N)]+p[IX(i,j+1,N)])/4;
            }
        }
        set_bnd ( N, 0, p );
    }
    for ( i=1 ; i<=N ; i++ ) {
        for ( j=1 ; j<=N ; j++ ) {
            u[IX(i,j,N)] -= 0.5*(p[IX(i+1,j,N)]-p[IX(i-1,j,N)])/h;
            v[IX(i,j,N)] -= 0.5*(p[IX(i,j+1,N)]-p[IX(i,j-1,N)])/h;
        }
    }
    set_bnd ( N, 1, u ); set_bnd ( N, 2, v );
}

void dens_step(int N, float* x, float* x0, float* s, float* u, float* v, float diffusion, float dt){
    add_source(N, x, s, dt);
    SWAP(x,x0);
    diffuse(N, 0, x, x0, diffusion, dt);
    SWAP(x,x0);
    advect(N, 0, x, x0, u, v, dt);
}

void vel_step(int N, float* u, float* v, float* u0, float* v0, float* su, float* sv, float visc, float dt){
    add_source(N, u, su, dt); add_source(N, v, sv, dt);
    SWAP(u,u0); diffuse(N, 1, u, u0, visc, dt);
    SWAP(v,v0); diffuse(N, 2, v, v0, visc, dt);
    
    project(N, u, v, u0, v0); // u0, v0 reused as p, div
    
    SWAP(u0,u); SWAP(v,v0);
    advect(N, 1, u, u0, u0, v0, dt); advect(N, 2, v, v0, u0, v0, dt);
    
    project(N, u, v, u0, v0);
}

void Fluid::step(float dt){
    vel_step(N, u.data(), v.data(), uPrev.data(), vPrev.data(), su.data(), sv.data(), viscosity, dt);
    dens_step(N, density.data(), densityPrev.data(), density_source.data(), u.data(), v.data(), diffusion, dt);
}

//================================================================================================

void Fluid::set_bnd_2(int b, TextureID x, MTL::CommandBuffer* cmd){
    auto set_bnd_1_PS = pipeMan->getCompute(set_bnd_1_pipe);
    auto set_bnd_2_PS = pipeMan->getCompute(set_bnd_2_pipe);
    
    Texture* x_tex = texMan->get(x);
    
    FluidParams fluidParams;
    fluidParams.N = N;
    fluidParams.b = b;
    
    auto enc1 = cmd->computeCommandEncoder();
    // first pass (borders)
    enc1->setComputePipelineState(set_bnd_1_PS);
    enc1->setTexture((MTL::Texture*)x_tex->gpuTexture, 0);
    enc1->setBytes(&fluidParams, sizeof(FluidParams), 1);
    
    MTL::Size gridSize = MTL::Size(N, 1, 1);
    MTL::Size tgSize = MTL::Size(set_bnd_1_PS->maxTotalThreadsPerThreadgroup(),1,1);
    enc1->dispatchThreads(gridSize, tgSize);
    
    // second pass (corners)
    enc1->setComputePipelineState(set_bnd_2_PS);
    enc1->setTexture((MTL::Texture*)x_tex->gpuTexture, 0);
    enc1->setBytes(&fluidParams, sizeof(FluidParams), 1);
    
    MTL::Size gridSize2 = MTL::Size(1, 1, 1);
    MTL::Size tgSize2 = MTL::Size(1,1,1);
    enc1->dispatchThreads(gridSize, tgSize);
    enc1->endEncoding();
}

void Fluid::add_source_2(TextureID x, TextureID s, float dt, MTL::CommandBuffer* cmd){
    auto add_source_PS = pipeMan->getCompute(add_source_pipe);
    
    Texture* x_tex = texMan->get(x);
    Texture* s_tex = texMan->get(s);
    
    FluidParams fp;
    fp.dt = dt;
    
    auto enc = cmd->computeCommandEncoder();
    enc->setComputePipelineState(add_source_PS);
    enc->setTexture((MTL::Texture*)x_tex->gpuTexture, 0);
    enc->setTexture((MTL::Texture*)s_tex->gpuTexture, 1);
    enc->setBytes(&fp, sizeof(FluidParams), 1);
    
    MTL::Size gridSize = MTL::Size(N, N, 1);
    NS::UInteger w = add_source_PS->threadExecutionWidth();
    NS::UInteger h = add_source_PS->maxTotalThreadsPerThreadgroup() / w;
    MTL::Size tgSize = MTL::Size(w,h,1);
    enc->dispatchThreads(gridSize, tgSize);
    enc->endEncoding();
}

void Fluid::diffuse_2(int b, TextureID& x, TextureID x0, float diff, float dt, MTL::CommandBuffer* cmd){
    auto diffuse_step_PS = pipeMan->getCompute(diffuse_step_pipe);
    
    FluidParams fp;
    fp.N = N;
    fp.diff = diff;
    fp.dt = dt;
    
    MTL::Size gridSize = MTL::Size(N, N, 1);
    NS::UInteger w = diffuse_step_PS->threadExecutionWidth();
    NS::UInteger h = diffuse_step_PS->maxTotalThreadsPerThreadgroup() / w;
    MTL::Size tgSize = MTL::Size(w,h,1);
    
    for (int k=0; k<steps; k++) {
        
        SWAP(x,pingPong_texture);

        Texture* x_tex = texMan->get(x);
        Texture* x0_tex = texMan->get(x0);
        Texture* x_old_tex = texMan->get(k==0 ? zero_texture : pingPong_texture);
        
        auto enc = cmd->computeCommandEncoder();
        enc->setComputePipelineState(diffuse_step_PS);
        enc->setTexture((MTL::Texture*)x_tex->gpuTexture, 0);
        enc->setTexture((MTL::Texture*)x_old_tex->gpuTexture, 1);
        enc->setTexture((MTL::Texture*)x0_tex->gpuTexture, 2);
        enc->setBytes(&fp, sizeof(FluidParams), 1);
        
        enc->dispatchThreads(gridSize, tgSize);
        enc->endEncoding();
        
        set_bnd_2(b, x, cmd);
    }
}

void Fluid::advect_2(int b, TextureID d, TextureID d0, TextureID u, TextureID v, float dt, MTL::CommandBuffer* cmd){
    auto advect_PS = pipeMan->getCompute(advect_pipe);
    
    Texture* d_tex = texMan->get(d);
    Texture* d0_tex = texMan->get(d0);
    Texture* u_tex = texMan->get(u);
    Texture* v_tex = texMan->get(v);
    
    FluidParams fp;
    fp.N = N;
    fp.dt = dt;
    
    auto enc = cmd->computeCommandEncoder();
    enc->setComputePipelineState(advect_PS);
    enc->setTexture((MTL::Texture*)d_tex->gpuTexture, 0);
    enc->setTexture((MTL::Texture*)d0_tex->gpuTexture, 1);
    enc->setTexture((MTL::Texture*)u_tex->gpuTexture, 2);
    enc->setTexture((MTL::Texture*)v_tex->gpuTexture, 3);
    
    enc->setBytes(&fp, sizeof(FluidParams), 1);
    
    
    MTL::Size gridSize = MTL::Size(N, N, 1);
    NS::UInteger w = advect_PS->threadExecutionWidth();
    NS::UInteger h = advect_PS->maxTotalThreadsPerThreadgroup() / w;
    MTL::Size tgSize = MTL::Size(w,h,1);
    
    enc->dispatchThreads(gridSize, tgSize);
    enc->endEncoding();
    
    set_bnd_2(b, d, cmd);
}

void Fluid::project_2(TextureID u, TextureID v, TextureID& p, TextureID div, MTL::CommandBuffer* cmd){
    auto div_PS = pipeMan->getCompute(divergence_pipe);
    auto p_step_PS = pipeMan->getCompute(p_step_pipe);
    auto sub_p_PS = pipeMan->getCompute(sub_p_pipe);
    
    // first pass (div calculation)
    Texture* u_tex = texMan->get(u);
    Texture* v_tex = texMan->get(v);
    Texture* div_tex = texMan->get(div);
    
    FluidParams fp;
    fp.N = N;
    
    auto div_enc = cmd->computeCommandEncoder();
    div_enc->setComputePipelineState(div_PS);
    
    div_enc->setTexture((MTL::Texture*)u_tex->gpuTexture, 0);
    div_enc->setTexture((MTL::Texture*)v_tex->gpuTexture, 1);
    div_enc->setTexture((MTL::Texture*)div_tex->gpuTexture, 2);
    div_enc->setBytes(&fp, sizeof(FluidParams), 1);
    
    MTL::Size gridSize = MTL::Size(N, N, 1);
    NS::UInteger w = div_PS->threadExecutionWidth();
    NS::UInteger h = div_PS->maxTotalThreadsPerThreadgroup() / w;
    MTL::Size tgSize = MTL::Size(w,h,1);
    
    div_enc->dispatchThreads(gridSize, tgSize);
    div_enc->endEncoding();
    
    set_bnd_2(0, div, cmd); set_bnd_2(0, p, cmd);
    
    // second pass (p calculation)
    for (int k=0; k<steps; k++) {
        
        SWAP(p,pingPong_texture);

        Texture* p_tex = texMan->get(p);
        Texture* p_old_tex = texMan->get(k==0 ? zero_texture : pingPong_texture);
        
        auto enc = cmd->computeCommandEncoder();
        enc->setComputePipelineState(p_step_PS);
        enc->setTexture((MTL::Texture*)div_tex->gpuTexture, 0);
        enc->setTexture((MTL::Texture*)p_old_tex->gpuTexture, 1);
        enc->setTexture((MTL::Texture*)p_tex->gpuTexture, 2);
        
        enc->dispatchThreads(gridSize, tgSize);
        enc->endEncoding();
        
        set_bnd_2(0, p, cmd);
    }
    
    // third pass (subtract p from u and v)
    Texture* p_tex = texMan->get(p);
    
    auto sub_p_enc = cmd->computeCommandEncoder();
    sub_p_enc->setComputePipelineState(sub_p_PS);
    sub_p_enc->setTexture((MTL::Texture*)u_tex->gpuTexture, 0);
    sub_p_enc->setTexture((MTL::Texture*)v_tex->gpuTexture, 1);
    sub_p_enc->setTexture((MTL::Texture*)p_tex->gpuTexture, 2);
    sub_p_enc->setBytes(&fp, sizeof(FluidParams), 1);
    
    sub_p_enc->dispatchThreads(gridSize, tgSize);
    sub_p_enc->endEncoding();
    set_bnd_2(1, u, cmd);
    set_bnd_2(2, v, cmd);
}


void Fluid::dens_step_2(TextureID& x, TextureID& x0, TextureID s, TextureID u, TextureID v, float diffusion, float dt, MTL::CommandBuffer* cmd){
    add_source_2(x, s, dt, cmd);
    SWAP(x,x0);
    diffuse_2(0, x, x0, diffusion, dt, cmd);
    SWAP(x,x0);
    advect_2(0, x, x0, u, v, dt, cmd);
    
}

void Fluid::vel_step_2(TextureID& u, TextureID& v, TextureID& u0, TextureID& v0, TextureID su, TextureID sv, float visc, float dt, MTL::CommandBuffer* cmd){
    add_source_2(u, su, dt, cmd); add_source_2(v, sv, dt, cmd);
    SWAP(u,u0); diffuse_2(1, u, u0, visc, dt, cmd);
    SWAP(v,v0); diffuse_2(2, v, v0, visc, dt, cmd);
    
    project_2(u, v, u0, v0, cmd); // u0, v0 reused as p, div
    
    SWAP(u0,u); SWAP(v,v0);
    advect_2(1, u, u0, u0, v0, dt, cmd); advect_2(2, v, v0, u0, v0, dt, cmd);
    
    project_2(u, v, u0, v0, cmd);
}

void Fluid::step_2(float dt, MTL::CommandBuffer* cmd){
    vel_step_2(u_texture, v_texture, uPrev_texture,vPrev_texture,su_texture,sv_texture, viscosity, dt, cmd);
    dens_step_2(density_texture, density_prev_texture, density_source_texture, u_texture, v_texture, diffusion, dt, cmd);
}
