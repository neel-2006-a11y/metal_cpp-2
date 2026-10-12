//
//  texture_generator.cpp
//  metal_cpp
//
//  Created by Neel on 14/03/26.
//

#include "texture_generator.h"
#include <fstream>
#include <random>


void BayerFactory::generateBayer(int *mat, int n)
{
    if (n == 2){
        int bayer2x2[] = {
            0,2,
            3,1
        };
        for (int i=0; i<4; i++) {
            mat[i]=bayer2x2[i];
        }
        return;
    }

    int half = n / 2;
    
    int mat_half[half * half];
    BayerFactory::generateBayer(mat_half, half);

    for (int y = 0; y < half; y++)
    {
        for (int x = 0; x < half; x++)
        {
            int v = mat_half[y*half + x] * 4;

            mat[y*n + x] = v + 0;
            mat[y*n + x + half] = v + 2;
            mat[(y + half)*n + x] = v + 3;
            mat[(y + half)*n + x + half] = v + 1;
        }
    }
}


void corner_align(double& c_x, double& c_y, double cell_width, int corner){
    switch (corner){
        case 0:
            break;
        case 1:
            c_x += cell_width;
            break;
        case 2:
            c_y += cell_width;
            break;
        case 3:
            c_x += cell_width;
            c_y += cell_width;
            break;
    }
}

void TextureFactory::generateTexture(Image* outTexture, int resolution, int dots, int n, bool grad){
    int bayer[n * n];
    BayerFactory::generateBayer(bayer, n);
    for (int i=0; i<n; i++) {
        for (int j=0; j<n; j++) {
            std::cout << bayer[i*n + j] << " ";
        }
        std::cout << std::endl;
    }
    
    int quarter_mat[4 * n * n]; //
    for (int i=0; i<n; i++) {
        for (int j=0; j<n; j++) {
            int count = 1;
            for (int x=0; x<2; x++) {
                for (int y=0; y<2; y++) {
                    quarter_mat[((2*i-1 + x +2*n)%(2*n))*2*n + (2*j-1 + y +2*n)%(2*n)] = 4*bayer[i*n + j] + count++;
                }
            }
        }
    }
    for (int i=0; i<2*n; i++) {
        for (int j=0; j<2*n; j++) {
            std::cout << quarter_mat[i*2*n + j] << " ";
        }
        std::cout << std::endl;
    }
    
    int corners[2][2] = {
        {0, 1},
        {2, 3}
    };
    
    int activated = (dots) * 4;
    
    double radius_2 = 0.5 / ((M_PI) * (dots));
//    double radius_2 = 0.125 / dots;
    
    for(int x=0; x<resolution; x++){
        for(int y=0; y<resolution; y++){
            outTexture->pixels[x*resolution + y] = 0.0f;
            for(int i = 0; i<2*n; i++){
                for(int j = 0; j<2*n; j++){ // we are checking each center and finding the minimum distance (can be more efficient)
                    int node = quarter_mat[i*2*n + j];
                    if(node>activated)continue;
                    
                    double cell_size = ((double)1/(2*n));
                    double cx = cell_size*j;
                    double cy = cell_size*i;
                    corner_align(cx, cy, cell_size, corners[i%2][j%2]);
                    double pos_x = (float)x/resolution;
                    double pos_y = (float)y/resolution;
                    
                    double dist_2 = (cx-pos_x)*(cx-pos_x) + (cy-pos_y)*(cy-pos_y);
//                    double dist_2 = fmax((cx-pos_x)*(cx-pos_x), (cy-pos_y)*(cy-pos_y));
                    
                    if(grad)
                        outTexture->pixels[x*resolution + y] = fmax((radius_2-dist_2)/radius_2 , outTexture->pixels[x*resolution + y]);
                    else
                        if(dist_2 < radius_2){
                            outTexture->pixels[x*resolution + y] = 1.0f;
                        }
                }
            }
        }
    }
}

void TextureFactory::generate2x2(Image outTextures[4], int resolution, bool grad){
    for (int dots=1; dots<=4; dots++) {
        generateTexture(&outTextures[dots-1], resolution, dots, 2, grad);
    }
}
void TextureFactory::generate4x4(Image outTextures[13], int resolution, bool grad){
    for (int dots=4; dots<=16; dots++) {
        generateTexture(&outTextures[dots-4], resolution, dots, 4, grad);
    }
}

void TextureFactory::generate8x8(Image outTextures[49], int resolution, bool grad){
    for (int dots=16; dots<=64; dots++) {
        generateTexture(&outTextures[dots-16], resolution, dots, 8, grad);
    }
}


float min_distance_from_line(float cx, float cy, float half_length, float angle_x, float x, float y){
    simd::float2 C = simd::float2{cx,cy};
    simd::float2 d = simd::float2{std::cos(angle_x), std::sin(angle_x)};
    simd::float2 X = simd::float2{x,y};
    
    float t = simd_dot(d, X-C);
    t = simd_clamp(t, -half_length, half_length);
    return simd_length(C - X + t*d);
}


void TextureFactory::generateTextureHatch(Image *outTexture, int resolution, int strokes, int n, bool grad, bool fill_angles, float* quarter_angle){
    int bayer[n*n];
    BayerFactory::generateBayer(bayer, n);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    float min = M_PI / 4 - M_PI / 16;
    float max = M_PI / 4 + M_PI / 16;
    std::uniform_real_distribution<float> dis(min, max);
    
    if(fill_angles){
        float angles[n*n];
        for(int i=0; i<n/2; i++){
            for(int j=0; j<n/2; j++){
                float angle = dis(gen);
                angles[i*n + j] = angle;
                angles[(i+n/2)*n + j] = angle;
                angles[i*n + j+n/2] = angle;
                angles[(i+n/2)*n + j+n/2] = angle;
            }
        }
        
        for (int i=0; i<n; i++) {
            for (int j=0; j<n; j++) {
                float random_angle = angles[i*n + j];
                for (int x=0; x<2; x++) {
                    for (int y=0; y<2; y++) {
                        quarter_angle[((2*i-1 + x +2*n)%(2*n))*2*n + (2*j-1 + y +2*n)%(2*n)] = random_angle;
                    }
                }
            }
        }
    }
    
    
    int quarter_mat[4 * n * n];
    for (int i=0; i<n; i++) {
        for (int j=0; j<n; j++) {
            int count = 1;
            for (int x=0; x<2; x++) {
                for (int y=0; y<2; y++) {
                    quarter_mat[((2*i-1 + x +2*n)%(2*n))*2*n + (2*j-1 + y +2*n)%(2*n)] = 4*bayer[i*n + j] + count++;
                }
            }
        }
    }
    
    
    int corners[2][2] = {
        {0, 1},
        {2, 3}
    };
    
    int activated = strokes * 4;
    
    float length = 4.0 / (M_PI * sqrt(strokes));
    float width = length / 20;
    
    for(int x=0; x<resolution; x++)
        for(int y=0; y<resolution; y++)
            outTexture->pixels[y*resolution + x]=0.0f;
    
    for(int i=0; i<2*n; i++){
        for(int j=0; j<2*n; j++){
            int node = quarter_mat[i*2*n + j];
            if(node>activated)continue;
            
//            int corner = corners[i%2][j%2];
            
            double cell_size = ((double)1/(2*n));
            double cx = cell_size*j;
            double cy = cell_size*i;
            corner_align(cx, cy, cell_size, corners[i%2][j%2]);
            
            for(int x=0; x<resolution; x++){
                for(int y=0; y<resolution; y++){
                    
                    
                    float pixel_x = (float)x/resolution, pixel_y = (float)y/resolution;
                    
                    float min_dist = min_distance_from_line(cx, cy, length, quarter_angle[i*2*n + j], pixel_x, pixel_y);
                    
                    if(grad){
                        outTexture->pixels[y*resolution + x] = fmax((width - min_dist)/width, outTexture->pixels[y*resolution + x]);
//                        outTexture->pixels[y*resolution + x] =
//                        fmin(min_dist, outTexture->pixels[y*resolution + x]);
                    }else{
                        if(min_dist<width){
                            outTexture->pixels[y*resolution + x] = 1.0f;
                        }
                    }
                    
                }
            }
        }
    }
    
}

void TextureFactory::generate2x2Hatch(Image *outTexture, int resolution, bool grad){
    int n = 2;
    bool fill_angles = true;
    float quarter_angle[4 * n * n];
    for(int strokes = (n/2)*(n/2); strokes <= 4*(n/2)*(n/2); strokes++){

        TextureFactory::generateTextureHatch(&outTexture[strokes-(n/2)*(n/2)], resolution, strokes, n, grad, fill_angles, quarter_angle);
        fill_angles = false;
    }
}

void TextureFactory::generate4x4Hatch(Image *outTexture, int resolution, bool grad){
    int n = 4;
    bool fill_angles = true;
    float quarter_angle[4 * n * n];
    for(int strokes = (n/2)*(n/2); strokes <= 4*(n/2)*(n/2); strokes++){

        TextureFactory::generateTextureHatch(&outTexture[strokes-(n/2)*(n/2)], resolution, strokes, n, grad, fill_angles, quarter_angle);
        fill_angles = false;
    }
}

void TextureFactory::generate8x8Hatch(Image *outTexture, int resolution, bool grad){
    int n = 8;
    bool fill_angles = true;
    float quarter_angle[4 * n * n];
    for(int strokes = (n/2)*(n/2); strokes <= 4*(n/2)*(n/2); strokes++){

        TextureFactory::generateTextureHatch(&outTexture[strokes-(n/2)*(n/2)], resolution, strokes, n, grad, fill_angles, quarter_angle);
        fill_angles = false;
    }
}

void TextureFactory::generatenxnHatch(Image *outTexture, int resolution, int n, bool grad){
    bool fill_angles = true;
    float quarter_angle[4 * n * n];
    for(int strokes = (n/2)*(n/2); strokes <= 4*(n/2)*(n/2); strokes++){

        TextureFactory::generateTextureHatch(&outTexture[strokes-(n/2)*(n/2)], resolution, strokes, n, grad, fill_angles, quarter_angle);
        fill_angles = false;
    }
}

void savePPM(const Image& img, const std::string& filename){
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        return;
    }
    
    file << "P6\n";
    file << img.resolution << " " << img.resolution << "\n";
    file << (HALFTONE_TEXTURE_16BIT ? 65535 : 255) << "\n";
    
    const int size = img.resolution * img.resolution;
    
    for (int i = 0; i < size; ++i) {
        float v = img.pixels[i];
#if HALFTONE_TEXTURE_16BIT

        uint16_t value = static_cast<uint16_t>(v * 65535.0f);

        unsigned char rgb[6] = {
            (unsigned char)(value >> 8), (unsigned char)(value & 0xFF),
            (unsigned char)(value >> 8), (unsigned char)(value & 0xFF),
            (unsigned char)(value >> 8), (unsigned char)(value & 0xFF)
        };

        file.write(reinterpret_cast<const char*>(rgb), 6);
#else
        uint8_t value = static_cast<uint8_t>(v * 255.0f);

        unsigned char rgb[3] = { value, value, value };
        file.write(reinterpret_cast<const char*>(rgb), 3);
#endif
    }
    std::cout << "saved: " << filename << std::endl;
}
