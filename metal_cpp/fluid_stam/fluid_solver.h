//
//  fluid_solver.h
//  metal_cpp
//
//  Created by Neel on 18/06/26.
//

#pragma once
#include "config.h"
#include "resource_managers/resource_types.h"
#include "resource_managers/texture_manager.h"
#include "resource_managers/pipeline_manager.h"

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
    float val;
};

class Fluid
{
public:
    Fluid(int size, int steps, TextureManager* texMan, PipelineManager* pipeMan);
    
    void step(float dt);
    void step_2(float dt, MTL::CommandBuffer* cmd);
    
    void addDensity(int x, int y, int size, float amount, MTL::CommandBuffer* cmd);
    
    void addVelocity(int x, int y, int size, float amountX, float amountY, MTL::CommandBuffer* cmd);
    
    const std::vector<float>& getDensity() const;
    
    int N;
    
    int steps;
    
    // textures
    TextureID density_source_texture = INVALID_TEXTURE;
    TextureID density_prev_texture = INVALID_TEXTURE;
    TextureID density_texture = INVALID_TEXTURE;
    
    TextureID u_texture = INVALID_TEXTURE;
    TextureID v_texture = INVALID_TEXTURE;
    
    TextureID uPrev_texture = INVALID_TEXTURE;
    TextureID vPrev_texture = INVALID_TEXTURE;
    
    TextureID su_texture = INVALID_TEXTURE;
    TextureID sv_texture = INVALID_TEXTURE;
    
    TextureID pingPong_texture = INVALID_TEXTURE;
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
    
    float diffusion = 0.0001f;
    float viscosity = 0.0001f;
    
    std::vector<float> density_source;
    std::vector<float> density;
    std::vector<float> densityPrev;
    
    std::vector<float> u;
    std::vector<float> v;
    
    std::vector<float> uPrev;
    std::vector<float> vPrev;
    
    std::vector<float> su;
    std::vector<float> sv;
    
private:
    TextureManager* texMan = nullptr;
    PipelineManager* pipeMan = nullptr;
    
    void set_bnd_2(int b, TextureID x, MTL::CommandBuffer* cmd);
    void add_source_2(TextureID x, TextureID s, float dt, MTL::CommandBuffer* cmd);
    void diffuse_2(int b, TextureID& x, TextureID x0, float diff, float dt, MTL::CommandBuffer* cmd);
    void advect_2(int b, TextureID d, TextureID d0, TextureID u, TextureID v, float dt, MTL::CommandBuffer* cmd);
    void project_2(TextureID u, TextureID v, TextureID& p, TextureID div, MTL::CommandBuffer* cmd);
    void dens_step_2(TextureID& x, TextureID& x0, TextureID s, TextureID u, TextureID v, float diffusion, float dt, MTL::CommandBuffer* cmd);
    void vel_step_2(TextureID& u, TextureID& v, TextureID& u0, TextureID& v0, TextureID su, TextureID sv, float visc, float dt, MTL::CommandBuffer* cmd);
};
