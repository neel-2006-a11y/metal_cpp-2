//
//  fluid_pass.cpp
//  metal_cpp
//
//  Created by Neel on 03/07/26.
//

#include "render_passes/fluid_pass.h"
#include "passers/MTL_Texture_passer.h"

#define SWAP(x0, x) {auto tmp=x0; x0=x; x=tmp;}

void FluidPass::init(RenderDevice renderDevice){
    make_textures(renderDevice.textureManager);
    make_pipelines(renderDevice.pipelineManager);
}

void FluidPass::execute(RenderContext renderContext, RenderGraph* renderGraph){
    
    upload_textures(renderContext);
    
    
    // interaction commands
    FluidCommandBuffer cmds = renderGraph->frameDataRegistry->get<FluidCommandBuffer>();
    
    while(!cmds.densityIneractQ.empty()){
        auto dI = cmds.densityIneractQ.front();
        addDensity(dI.x, dI.y, dI.size, dI.amount, renderContext);
        cmds.densityIneractQ.pop();
    }
    
    while(!cmds.velIneractQ.empty()){
        auto vI = cmds.velIneractQ.front();
        addVelocity(vI.x, vI.y, vI.size, vI.amountX, vI.amountY, renderContext);
        cmds.velIneractQ.pop();
    }
    
    
    step_2(0.016, renderContext);

    publish(renderGraph);
}

void FluidPass::resize(RenderContext renderContext){}


// interaction
void FluidPass::addDensity(int x, int y, int size, simd::float4 amount, RenderContext renderContext){
    auto add_val_PS = renderContext.pipelineManager->getCompute(add_val_pipe);
    
    Texture* dens_s_tex = renderContext.textureManager->get(density_source_texture);
    
    InteractParams ip;
    ip.N = N;
    ip.x = x;
    ip.y = y;
    ip.size = size;
    ip.val = amount;
    
    auto enc = renderContext.cmd->computeCommandEncoder();
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

void FluidPass::addVelocity(int x, int y, int size, float amountX, float amountY, RenderContext renderContext){
    auto add_val_PS = renderContext.pipelineManager->getCompute(add_val_pipe);
    
    Texture* s_u_tex = renderContext.textureManager->get(su_texture);
    Texture* s_v_tex = renderContext.textureManager->get(sv_texture);
    
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
    
    auto enc = renderContext.cmd->computeCommandEncoder();
    enc->setComputePipelineState(add_val_PS);
    enc->setTexture((MTL::Texture*)s_u_tex->gpuTexture, 0);
    enc->setBytes(&ip, sizeof(InteractParams), 2);
    
    enc->dispatchThreads(gridSize, tgSize);
    
    ip.val = amountY;
    
    enc->setTexture((MTL::Texture*)s_v_tex->gpuTexture, 0);
    enc->setBytes(&ip, sizeof(InteractParams), 2);
    enc->dispatchThreads(gridSize, tgSize);
    enc->endEncoding();
}

void FluidPass::publish(RenderGraph* renderGraph){
    renderGraph->resourceRegistry->publishTexture(ResourceKey::DensityTexture, density_texture);
}

void FluidPass::upload_textures(RenderContext renderContext){
    //--------------
    // upload textures to gpu
    //--------------
    uploadTextureToGPU(*renderContext.textureManager->get(density_source_texture), renderContext.device);
    uploadEmptyTextureToGPU(*renderContext.textureManager->get(density_prev_texture), renderContext.device);
    uploadEmptyTextureToGPU(*renderContext.textureManager->get(density_texture), renderContext.device);
    
    uploadEmptyTextureToGPU(*renderContext.textureManager->get(u_texture), renderContext.device);
    uploadEmptyTextureToGPU(*renderContext.textureManager->get(v_texture), renderContext.device);
    
    uploadEmptyTextureToGPU(*renderContext.textureManager->get(uPrev_texture), renderContext.device);
    uploadEmptyTextureToGPU(*renderContext.textureManager->get(vPrev_texture), renderContext.device);
    
    uploadTextureToGPU(*renderContext.textureManager->get(su_texture), renderContext.device);
    uploadEmptyTextureToGPU(*renderContext.textureManager->get(sv_texture), renderContext.device);
    
    uploadEmptyTextureToGPU(*renderContext.textureManager->get(pingPong_vel_texture), renderContext.device);
    uploadEmptyTextureToGPU(*renderContext.textureManager->get(pingPong_dens_texture), renderContext.device);
    uploadEmptyTextureToGPU(*renderContext.textureManager->get(zero_texture), renderContext.device);
}

void FluidPass::make_textures(TextureManager* texMan){
    TextureDesc vel_desc;
    vel_desc.width = N+2;
    vel_desc.height = N+2;
    vel_desc.format = TextureFormat::R32Float;
    vel_desc.storageMode = StorageMode::Shared;
    vel_desc.usage = TextureUsage::Sampled | TextureUsage::ShaderWrite;
    
    TextureDesc dens_desc;
    dens_desc.width = N+2;
    dens_desc.height = N+2;
    dens_desc.format = TextureFormat::RGBA16Float;
    dens_desc.storageMode = StorageMode::Shared;
    dens_desc.usage = TextureUsage::Sampled | TextureUsage::ShaderWrite;
    
    
    density_source_texture = texMan->createEmpty(dens_desc);
    density_prev_texture = texMan->createEmpty(dens_desc);
    density_texture = texMan->createEmpty(dens_desc);
    
    u_texture = texMan->createEmpty(vel_desc);
    v_texture = texMan->createEmpty(vel_desc);
    
    uPrev_texture = texMan->createEmpty(vel_desc);
    vPrev_texture = texMan->createEmpty(vel_desc);
    
    su_texture = texMan->createEmpty(vel_desc);
    sv_texture = texMan->createEmpty(vel_desc);
    
    pingPong_vel_texture = texMan->createEmpty(vel_desc);
    pingPong_dens_texture = texMan->createEmpty(dens_desc);
    
    zero_texture = texMan->createZeroTexture(vel_desc);
}

void FluidPass::make_pipelines(PipelineManager *pipeMan){
    //-------------- Compute Pipelines
    ComputePipelineDesc add_val_desc;
    add_val_desc.kernelFunc = "add_val";
    add_val_pipe = pipeMan->createComputePipeline(add_val_desc);
    
    ComputePipelineDesc set_bnd_1_desc;
    set_bnd_1_desc.kernelFunc = "set_bnd_1";
    set_bnd_1_pipe = pipeMan->createComputePipeline(set_bnd_1_desc);
    
    ComputePipelineDesc set_bnd_2_desc;
    set_bnd_2_desc.kernelFunc = "set_bnd_2";
    set_bnd_2_pipe = pipeMan->createComputePipeline(set_bnd_2_desc);
    
    ComputePipelineDesc add_source_desc;
    add_source_desc.kernelFunc = "add_source";
    add_source_pipe = pipeMan->createComputePipeline(add_source_desc);
    
    ComputePipelineDesc diffuse_step_desc;
    diffuse_step_desc.kernelFunc = "diffuse_step";
    diffuse_step_pipe = pipeMan->createComputePipeline(diffuse_step_desc);
    
    ComputePipelineDesc advect_desc;
    advect_desc.kernelFunc = "advect";
    advect_pipe = pipeMan->createComputePipeline(advect_desc);
    
    ComputePipelineDesc divergence_desc;
    divergence_desc.kernelFunc = "divergence";
    divergence_pipe = pipeMan->createComputePipeline(divergence_desc);
    
    ComputePipelineDesc p_step_desc;
    p_step_desc.kernelFunc = "p_step";
    p_step_pipe = pipeMan->createComputePipeline(p_step_desc);
    
    ComputePipelineDesc sub_p_desc;
    sub_p_desc.kernelFunc = "sub_p";
    sub_p_pipe = pipeMan->createComputePipeline(sub_p_desc);
}




//================================================================================================

void FluidPass::set_bnd_2(int b, TextureID x, RenderContext renderContext){
    auto set_bnd_1_PS = renderContext.pipelineManager->getCompute(set_bnd_1_pipe);
    auto set_bnd_2_PS = renderContext.pipelineManager->getCompute(set_bnd_2_pipe);
    
    Texture* x_tex = renderContext.textureManager->get(x);
    
    FluidParams fluidParams;
    fluidParams.N = N;
    fluidParams.b = b;
    
    auto enc1 = renderContext.cmd->computeCommandEncoder();
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

void FluidPass::add_source_2(TextureID x, TextureID s, float dt, RenderContext renderContext){
    auto add_source_PS = renderContext.pipelineManager->getCompute(add_source_pipe);
    
    Texture* x_tex = renderContext.textureManager->get(x);
    Texture* s_tex = renderContext.textureManager->get(s);
    
    FluidParams fp;
    fp.dt = dt;
    
    auto enc = renderContext.cmd->computeCommandEncoder();
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

void FluidPass::diffuse_2(int b, TextureID& x, TextureID x0, TextureID& PP_tex, TextureID zero_tex, float diff, float dt, RenderContext renderContext){
    auto diffuse_step_PS = renderContext.pipelineManager->getCompute(diffuse_step_pipe);
    
    FluidParams fp;
    fp.N = N;
    fp.diff = diff;
    fp.dt = dt;
    
    MTL::Size gridSize = MTL::Size(N, N, 1);
    NS::UInteger w = diffuse_step_PS->threadExecutionWidth();
    NS::UInteger h = diffuse_step_PS->maxTotalThreadsPerThreadgroup() / w;
    MTL::Size tgSize = MTL::Size(w,h,1);
    
    for (int k=0; k<steps; k++) {
        
        SWAP(x,PP_tex);

        Texture* x_tex = renderContext.textureManager->get(x);
        Texture* x0_tex = renderContext.textureManager->get(x0);
        Texture* x_old_tex = renderContext.textureManager->get(k==0 ? zero_tex : PP_tex);
        
        auto enc = renderContext.cmd->computeCommandEncoder();
        enc->setComputePipelineState(diffuse_step_PS);
        enc->setTexture((MTL::Texture*)x_tex->gpuTexture, 0);
        enc->setTexture((MTL::Texture*)x_old_tex->gpuTexture, 1);
        enc->setTexture((MTL::Texture*)x0_tex->gpuTexture, 2);
        enc->setBytes(&fp, sizeof(FluidParams), 1);
        
        enc->dispatchThreads(gridSize, tgSize);
        enc->endEncoding();
        
        set_bnd_2(b, x, renderContext);
    }
}

void FluidPass::advect_2(int b, TextureID d, TextureID d0, TextureID u, TextureID v, float dt, RenderContext renderContext){
    auto advect_PS = renderContext.pipelineManager->getCompute(advect_pipe);
    
    Texture* d_tex = renderContext.textureManager->get(d);
    Texture* d0_tex = renderContext.textureManager->get(d0);
    Texture* u_tex = renderContext.textureManager->get(u);
    Texture* v_tex = renderContext.textureManager->get(v);
    
    FluidParams fp;
    fp.N = N;
    fp.dt = dt;
    
    auto enc = renderContext.cmd->computeCommandEncoder();
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
    
    set_bnd_2(b, d, renderContext);
}

void FluidPass::project_2(TextureID u, TextureID v, TextureID& p, TextureID div, RenderContext renderContext){
    auto div_PS = renderContext.pipelineManager->getCompute(divergence_pipe);
    auto p_step_PS = renderContext.pipelineManager->getCompute(p_step_pipe);
    auto sub_p_PS = renderContext.pipelineManager->getCompute(sub_p_pipe);
    
    // first pass (div calculation)
    Texture* u_tex = renderContext.textureManager->get(u);
    Texture* v_tex = renderContext.textureManager->get(v);
    Texture* div_tex = renderContext.textureManager->get(div);
    
    FluidParams fp;
    fp.N = N;
    
    auto div_enc = renderContext.cmd->computeCommandEncoder();
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
    
    set_bnd_2(0, div, renderContext); set_bnd_2(0, p, renderContext);
    
    // second pass (p calculation)
    for (int k=0; k<steps; k++) {
        
        SWAP(p,pingPong_vel_texture);

        Texture* p_tex = renderContext.textureManager->get(p);
        Texture* p_old_tex = renderContext.textureManager->get(k==0 ? zero_texture : pingPong_vel_texture);
        
        auto enc = renderContext.cmd->computeCommandEncoder();
        enc->setComputePipelineState(p_step_PS);
        enc->setTexture((MTL::Texture*)div_tex->gpuTexture, 0);
        enc->setTexture((MTL::Texture*)p_old_tex->gpuTexture, 1);
        enc->setTexture((MTL::Texture*)p_tex->gpuTexture, 2);
        
        enc->dispatchThreads(gridSize, tgSize);
        enc->endEncoding();
        
        set_bnd_2(0, p, renderContext);
    }
    
    // third pass (subtract p from u and v)
    Texture* p_tex = renderContext.textureManager->get(p);
    
    auto sub_p_enc = renderContext.cmd->computeCommandEncoder();
    sub_p_enc->setComputePipelineState(sub_p_PS);
    sub_p_enc->setTexture((MTL::Texture*)u_tex->gpuTexture, 0);
    sub_p_enc->setTexture((MTL::Texture*)v_tex->gpuTexture, 1);
    sub_p_enc->setTexture((MTL::Texture*)p_tex->gpuTexture, 2);
    sub_p_enc->setBytes(&fp, sizeof(FluidParams), 1);
    
    sub_p_enc->dispatchThreads(gridSize, tgSize);
    sub_p_enc->endEncoding();
    set_bnd_2(1, u, renderContext);
    set_bnd_2(2, v, renderContext);
}


void FluidPass::dens_step_2(TextureID& x, TextureID& x0, TextureID s, TextureID u, TextureID v, float diffusion, float dt, RenderContext renderContext){
    add_source_2(x, s, dt, renderContext);
    SWAP(x,x0);
    diffuse_2(0, x, x0, pingPong_dens_texture, density_source_texture, diffusion, dt, renderContext);
    SWAP(x,x0);
    advect_2(0, x, x0, u, v, dt, renderContext);
    
}

void FluidPass::vel_step_2(TextureID& u, TextureID& v, TextureID& u0, TextureID& v0, TextureID su, TextureID sv, float visc, float dt, RenderContext renderContext){
    add_source_2(u, su, dt, renderContext); add_source_2(v, sv, dt, renderContext);
    SWAP(u,u0); diffuse_2(1, u, u0, pingPong_vel_texture, su_texture, visc, dt, renderContext);
    SWAP(v,v0); diffuse_2(1, v, v0, pingPong_vel_texture, sv_texture, visc, dt, renderContext);
    
    project_2(u, v, u0, v0, renderContext); // u0, v0 reused as p, div
    
    SWAP(u0,u); SWAP(v,v0);
    advect_2(1, u, u0, u0, v0, dt, renderContext); advect_2(2, v, v0, u0, v0, dt, renderContext);
    
    project_2(u, v, u0, v0, renderContext);
}

void FluidPass::step_2(float dt, RenderContext renderContext){
    vel_step_2(u_texture, v_texture, uPrev_texture,vPrev_texture,su_texture,sv_texture, viscosity, dt, renderContext);
    dens_step_2(density_texture, density_prev_texture, density_source_texture, u_texture, v_texture, diffusion, dt, renderContext);
}
