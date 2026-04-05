//
//  texture_manager.cpp
//  metal_cpp
//
//  Created by Neel on 18/03/26.
//

#include "resource_managers/texture_manager.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

TextureID TextureManager::createTexture(TextureDesc& desc, void* srcData, bool no_cpu = false)
{
    Texture tex;
    tex.desc = desc;

    size_t bpp = bytesPerPixel(desc.format);
    size_t totalSize = desc.width * desc.height * desc.layers * bpp;

    if(!no_cpu)
        tex.raw_data.resize(totalSize);

    if (srcData) {
        memcpy(tex.raw_data.data(), srcData, totalSize);
    }

    TextureID id = nextID++;
    textures[id] = std::move(tex);

    return id;
}


TextureID TextureManager::createEmpty(TextureDesc& desc)
{
    return createTexture(desc, nullptr);
}

TextureID TextureManager::createEmptyNoCPU(TextureDesc& desc)
{
    return createTexture(desc, nullptr, true);
}


Texture* TextureManager::get(TextureID id)
{
    auto it = textures.find(id);
    if (it == textures.end()) return nullptr;
    return &it->second;
}

void TextureManager::destroy(TextureID id)
{
    // also need to free the gpu resources
    textures.erase(id);
}

TextureID TextureManager::loadPPM(std::string &filename, TextureDesc &desc){
    std::ifstream file(filename, std::ios::binary);
    if(!file){
        std::cerr << "Failed to open" << filename << std::endl;
        return INVALID_TEXTURE;
    }
    
    std::string header;
    file >> header;
    if(header != "P6"){
        std::cerr << "Invalid PPM format in" << filename << std::endl;
        return INVALID_TEXTURE;
    }
    
    int width, height, maxval;
    file >> width >> height >> maxval;
    file.get(); // newline
    
    desc.width = width;
    desc.height = height;
    desc.layers = 1;
    
    if(maxval <= 255){ // 8-bit format
        std::vector<uint8_t> raw(width * height * 3);
        file.read(reinterpret_cast<char*>(raw.data()), raw.size());

        if (desc.format == TextureFormat::R8Unorm)
        {
            std::vector<uint8_t> gray(width * height);

            for (int i = 0; i < width * height; i++) {
                gray[i] = raw[i * 3]; // take red
            }

            return createTexture(desc, gray.data());
        }
        else if (desc.format == TextureFormat::R16Unorm)
        {
            std::vector<uint16_t> gray(width * height);

            for (int i = 0; i < width * height; i++) {
                // expand 8-bit -> 16-bit
                gray[i] = (uint16_t)raw[i * 3] * 257;
            }

            return createTexture(desc, gray.data());
        }
    
    }else{ // 16-bit format
        std::vector<uint16_t> raw(width * height * 3);
        file.read(reinterpret_cast<char*>(raw.data()), raw.size() * sizeof(uint16_t));

        if (desc.format == TextureFormat::R16Unorm)
        {
            std::vector<uint16_t> gray(width * height);

            for (int i = 0; i < width * height; i++) {
                gray[i] = raw[i * 3]; // take red
            }

            return createTexture(desc, gray.data());
        }
        else if (desc.format == TextureFormat::R8Unorm)
        {
            std::vector<uint8_t> gray(width * height);

            for (int i = 0; i < width * height; i++) {
                // downscale 16-bit -> 8-bit
                gray[i] = (uint16_t)raw[i * 3] >> 8;
            }

            return createTexture(desc, gray.data());
        }
    }
    
    std::cerr << "Unsupported format conversion\n";
    return INVALID_TEXTURE;
}


TextureID TextureManager::loadPPMArray(std::vector<std::string> &filenames, TextureDesc &desc){
    if(filenames.empty()){
        std::cerr << "No files provided\n";
        return INVALID_TEXTURE;
    }
    
    int width = 0, height = 0;
    int layers = (int)filenames.size();
    
    std::vector<uint8_t> data8;
    std::vector<uint16_t> data16;
    
    bool use16 = (desc.format == TextureFormat::R16Unorm);
    
    for(int layer = 0; layer < layers; layer++){
        std::ifstream file(filenames[layer], std::ios::binary);
        if(!file){
            std::cerr << "Failed to open" << filenames[layer] << std::endl;
            return INVALID_TEXTURE;
        }
        
        std::string header;
        file >> header;
        if(header != "P6"){
            std::cerr << "Invalid PPM format in" << filenames[layer] << std::endl;
            return INVALID_TEXTURE;
        }
        
        int w,h, maxval;
        file >> w >> h >> maxval;
        file.get();
        
        size_t totalPixels = w * h * layers;
        if(layer == 0){
            width = w;
            height = h;
            desc.width = width;
            desc.height = height;
            desc.layers = layers;
            
            if(use16)
                data16.resize(totalPixels);
            else
                data8.resize(totalPixels);
        }else{
            if (w!=width || h!=height){
                std::cerr << "Mismatched texture sizes in array\n";
                return INVALID_TEXTURE;
            }
        }
        
        // read data
        if(maxval <= 255){
            for(int i = 0; i<width * height; i++){
                int idx = width * height * layer + i;
                unsigned char rgb[3];
                file.read(reinterpret_cast<char*>(rgb), 3);
                
                if(use16){
                    data16[idx] = rgb[0] * 256;
                }else{
                    data8[idx] = rgb[0];
                }
            }
        }else{
            for (int i = 0; i < width * height; i++)
            {
                int idx = width * height * layer + i;
                unsigned char rgb[6];
                file.read(reinterpret_cast<char*>(rgb), 6);

                uint16_t r = (rgb[0] << 8) | rgb[1]; // big-endian
                if(use16){
                    data16[idx] = r;
                }else{
                    data8[idx] = rgb[0];
                }
            }
        }
    }
    
    if(use16)
        return createTexture(desc, data16.data());
    else
        return createTexture(desc, data8.data());
}

TextureID TextureManager::loadFromFile(const std::string& path){
    // ----------
    // Check Cache
    // ----------
    if(cache.find(path) != cache.end()){
        return cache[path];
    }
    
    // ----------
    // load from disk
    // ----------
    int width, height, channels;
    
    stbi_set_flip_vertically_on_load(false);
    
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if(!data){
        std::cout << "Failed to load Texture: " << path << std::endl;
        return INVALID_TEXTURE;
    }
    
    // ----------
    // decide format
    // ----------
    TextureFormat format;
    
    if(channels == 1){
        format = TextureFormat::R8Unorm;
    }else if(channels == 3){
        format = TextureFormat::RGBA8Unorm;
    }else{
        printf("Unsupported channel count: %d\n", channels);
        return INVALID_TEXTURE;
    }
    
    // ----------
    // convert to RGBA if needed
    // ----------
    unsigned char* finalData = data;
    
    if(channels == 3){
        //expand RGB -> RGBA
        size_t size = width * height * 4;
        unsigned char* rgba = new unsigned char[size];
        
        for(int i=0; i<width * height; i++){
            rgba[i * 4 + 0] = data[i * 3 + 2];
            rgba[i * 4 + 1] = data[i * 3 + 1];
            rgba[i * 4 + 2] = data[i * 3 + 0];
            rgba[i * 4 + 3] = 255;
        }
        finalData = rgba;
        stbi_image_free(data);
    }
    
    // ----------
    // Create Texture Desc
    // ----------
    TextureDesc desc;
    desc.width = width;
    desc.height = height;
    desc.format = format;
    desc.usage = TextureUsage::Sampled;
    desc.storageMode = StorageMode::Shared;
    
    TextureID id = createTexture(desc, finalData);
    
    // ----------
    // cleanup
    // ----------
    if(channels == 3){
        delete[] finalData;
    }else{
        stbi_image_free(finalData);
    }
    
    // ----------
    // Cache
    // ----------
    cache[path] = id;
    return id;
}
