//
//  texture_generator.h
//  metal_cpp
//
//  Created by Neel on 14/03/26.
//

#pragma once

#include <cstdint>
#include <vector>
#include "config.h"

struct Image{
    int resolution;
    std::vector<float> pixels;
};

namespace BayerFactory{
void generateBayer(int mat[], int n);
}

namespace TextureFactory{
    void generate2x2(Image outTextures[4], int resolution, bool grad);
    void generate4x4(Image outTextures[13], int resolution, bool grad);
    void generate8x8(Image outTextures[49], int resolution, bool grad);
    void generateTexture(Image* outTexture, int resolution, int dots, int n, bool grad);

    void generate2x2Hatch(Image outTexture[4], int resolution, bool grad);
    void generate4x4Hatch(Image outTextures[13], int resolution, bool grad);
    void generate8x8Hatch(Image outTextures[49], int resolution, bool grad);
    void generatenxnHatch(Image outTextures[], int resolution, int n, bool grad);
    void generateTextureHatch(Image* outTexture, int resolution, int strokes, int n, bool grad, bool fill_angle, float* quarter_angles);
}

void savePPM(const Image& img, const std::string& filename);
MTL::Texture* createTextureArray(MTL::Device* device, std::vector<Image>& images);
