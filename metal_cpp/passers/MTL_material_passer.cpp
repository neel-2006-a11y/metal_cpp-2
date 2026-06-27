//
//  MTL_material_passer.cpp
//  metal_cpp
//
//  Created by Neel on 22/03/26.
//

#include "passers/MTL_material_passer.h"
#include "passers/MTL_Texture_passer.h"
#include "view/renderContext.h"

void bindMaterial(MaterialID mat_id, Material& mat, MTL::RenderCommandEncoder* encoder, RenderContext renderContext){
    
    // textures
    for(auto& t : mat.textures){
        Texture* tex = renderContext.textureManager->get(t.texture);
        if(!tex) continue;
        
        uploadTextureToGPU(*tex, renderContext.renderer->device);
        
        encoder->setFragmentTexture((MTL::Texture*)tex->gpuTexture, t.slot);
    }
    
    // samplers
    for(auto& s : mat.samplers){
        encoder->setFragmentSamplerState((MTL::SamplerState*)s.sampler, s.slot);
    }
    
    // uniforms
    size_t offset = (mat_id - 1) * renderContext.renderer->materialStride;
    encoder->setFragmentBuffer(renderContext.renderer->materialBuffer, offset, 3);
}
