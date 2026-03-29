//
//  MTL_material_passer.cpp
//  metal_cpp
//
//  Created by Neel on 22/03/26.
//

#include "passers/MTL_material_passer.h"
#include "passers/MTL_Texture_passer.h"

void bindMaterial(Material& mat, MTL::RenderCommandEncoder* encoder, TextureManager& texMgr,  MTL::Device* device){
    
    // textures
    for(auto& t : mat.textures){
        Texture* tex = texMgr.get(t.texture);
        if(!tex) continue;
        
        uploadTextureToGPU(*tex, device);
        
        encoder->setFragmentTexture((MTL::Texture*)tex->gpuTexture, t.slot);
    }
    
    // samplers
    for(auto& s : mat.samplers){
        encoder->setFragmentSamplerState((MTL::SamplerState*)s.sampler, s.slot);
    }
}
