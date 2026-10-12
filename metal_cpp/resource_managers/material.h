//
//  material.h
//  metal_cpp
//
//  Created by Neel on 22/03/26.
//

#pragma once

#include <vector>
#include <cstdint>
#include "resource_managers/resource_types.h"
#include "resource_managers/pipeline_manager.h"


struct TextureBinding{
    uint32_t slot;
    TextureID texture;
};

struct SamplerBinding{
    uint32_t slot;
    SamplerID sampler;
};

struct MaterialUniforms{
    std::vector<uint8_t> data;
};

struct Material
{
    PipelineDesc pipeDesc;
    PipelineID pipeID;
    
    std::vector<TextureBinding> textures;
    std::vector<SamplerBinding> samplers;
    
    MaterialUniforms uniforms;
    uint32_t uniform_offset = -1;
};

class MaterialUniformsBuilder{
    size_t offset = 0;
    std::vector<uint8_t> data;
    
public:
    template<typename T>
    size_t add_pointer(const T* item, size_t size){
        if(data.size() < offset + size)data.resize(offset + size);
        std::memcpy(data.data() + offset, item, size);
        offset += size;
        return offset;
    }
    
    template <typename T>
    size_t add(const T& item){
        if(data.size() < offset + sizeof(T)){
            data.resize(offset + sizeof(T));
//            std::cout << offset + sizeof(T) << std::endl;
//            std::cout << data.size() << std::endl;
        }
        std::memcpy(data.data() + offset, &item, sizeof(T));
        offset += sizeof(T);
        return offset;
    }
    
    MaterialUniforms get_buffer() const {
        MaterialUniforms MU;
        MU.data = data; // cpy data to Uniform
        return MU;
    }
};
