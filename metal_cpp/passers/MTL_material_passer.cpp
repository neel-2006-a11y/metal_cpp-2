//
//  MTL_material_passer.cpp
//  metal_cpp
//
//  Created by Neel on 22/03/26.
//

#include "passers/MTL_material_passer.h"
#include "passers/MTL_Texture_passer.h"
#include "view/renderContext.h"

void bindMaterial(Material& mat, MTL::RenderCommandEncoder* encoder, RenderContext renderContext){
    
    // textures
    for(auto& t : mat.textures){
        Texture* tex = renderContext.textureManager->get(t.texture);
        if(!tex) continue;
        
        uploadTextureToGPU(*tex, renderContext.device);
        
        encoder->setFragmentTexture(tex->gpuTexture, t.slot);
    }
    
    // samplers
    for(auto& s : mat.samplers){
        encoder->setFragmentSamplerState(renderContext.samplerManager->getState(s.sampler)->state, s.slot);
    }
    
    // uniforms
    size_t offset = mat.uniform_offset;
    if(mat.uniforms.data.size()>0)
        encoder->setFragmentBuffer(renderContext.materialBufferData.materialBuffer, offset, 3);
}
