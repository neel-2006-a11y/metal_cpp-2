//
//  fluid_pass.h
//  metal_cpp
//
//  Created by Neel on 03/07/26.
//

#pragma once
#include "queue"
#include "render_passes/render_pass.h"
#include "fluid_stam/fluid_interaction.h"

struct FluidParams{
    uint N = 0;
    float diff = 0;
    float dt = 0;
    int b = 0;
};

struct InteractParams {
    int N;
    int x, y;
    int size; // square dim 2*size + 1
    simd::float4 val;
};

class FluidPass : public RenderPass{
public:
    // owned / output
    TextureID density_source_texture = INVALID_TEXTURE;
    TextureID density_prev_texture = INVALID_TEXTURE;
    TextureID density_texture = INVALID_TEXTURE;
    
    TextureID u_texture = INVALID_TEXTURE;
    TextureID v_texture = INVALID_TEXTURE;
    
    TextureID uPrev_texture = INVALID_TEXTURE;
    TextureID vPrev_texture = INVALID_TEXTURE;
    
    TextureID su_texture = INVALID_TEXTURE;
    TextureID sv_texture = INVALID_TEXTURE;
    
    TextureID pingPong_vel_texture = INVALID_TEXTURE;
    TextureID pingPong_dens_texture = INVALID_TEXTURE;
    TextureID zero_texture = INVALID_TEXTURE;
    
    // pipelines
    ComputePipelineID add_val_pipe = INVALID_COMPUTE_PIPE;
    ComputePipelineID set_bnd_1_pipe = INVALID_COMPUTE_PIPE;
    ComputePipelineID set_bnd_2_pipe = INVALID_COMPUTE_PIPE;
    ComputePipelineID add_source_pipe = INVALID_COMPUTE_PIPE;
    ComputePipelineID diffuse_step_pipe = INVALID_COMPUTE_PIPE;
    ComputePipelineID advect_pipe = INVALID_COMPUTE_PIPE;
    ComputePipelineID divergence_pipe = INVALID_COMPUTE_PIPE;
    ComputePipelineID p_step_pipe = INVALID_COMPUTE_PIPE;
    ComputePipelineID sub_p_pipe = INVALID_COMPUTE_PIPE;
    
    void init(RenderDevice renderDevice) override;
    void execute(RenderContext renderContext, RenderGraph* renderGraph) override;
    void resize(RenderContext renderContext) override;
    
private:
    int N = 256;
    int steps = 50;
    // 256 * 256 * 200 ->15ms
    // 1024 * 1024 * 12 -> 15ms
    float viscosity = 0.0001;
    float diffusion = 0.000;
    
    void publish(RenderGraph* renderGraph);
    
    void upload_textures(RenderContext renderContext);
    
    void make_textures(TextureManager* texMan);
    void make_pipelines(PipelineManager* pipeMan);
    
    // interaction
    void addDensity(int x, int y, int size, simd::float4 amount, RenderContext renderContext);
    
    void addVelocity(int x, int y, int size, float amountX, float amountY, RenderContext renderContext);
    
    // update
    void set_bnd_2(int b, TextureID x, RenderContext renderContext, MTL::ComputeCommandEncoder* enc);
    
    void add_source_2(TextureID x, TextureID s, float dt, RenderContext renderContext);
    
    void diffuse_2(int b, TextureID& x, TextureID x0, TextureID& PP_tex, TextureID zero_tex, float diff, float dt, RenderContext renderContext);
    
    void advect_2(int b, TextureID d, TextureID d0, TextureID u, TextureID v, float dt, RenderContext renderContext);
    
    void project_2(TextureID u, TextureID v, TextureID& p, TextureID div, RenderContext renderContext);
    
    void dens_step_2(TextureID& x, TextureID& x0, TextureID s, TextureID u, TextureID v, float diffusion, float dt, RenderContext renderContext);
    
    void vel_step_2(TextureID& u, TextureID& v, TextureID& u0, TextureID& v0, TextureID su, TextureID sv, float visc, float dt, RenderContext renderContext);
    
    void step_2(float dt, RenderContext renderContext);
};
