//
//  MTL_material_passer.cpp
//  metal_cpp
//
//  Created by Neel on 22/03/26.
//

#include "passers/MTL_material_passer.h"
#include "passers/MTL_Texture_passer.h"

void bindMaterial(MaterialID mat_id, Material& mat, MTL::RenderCommandEncoder* encoder, TextureManager& texMgr,  Renderer2& renderer){
    
    // textures
    for(auto& t : mat.textures){
        Texture* tex = texMgr.get(t.texture);
        if(!tex) continue;
        
        uploadTextureToGPU(*tex, renderer.device);
        
        encoder->setFragmentTexture((MTL::Texture*)tex->gpuTexture, t.slot);
    }
    
    // samplers
    for(auto& s : mat.samplers){
        encoder->setFragmentSamplerState((MTL::SamplerState*)s.sampler, s.slot);
    }
    
    // uniforms
    size_t offset = (mat_id - 1) * renderer.materialStride;
    encoder->setFragmentBuffer(renderer.materialBuffer, offset, 3);
}
