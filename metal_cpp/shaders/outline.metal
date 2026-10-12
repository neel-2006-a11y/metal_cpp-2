//
//  outline.metal
//  metal_cpp
//
//  Created by Neel on 02/10/26.
//

#include <metal_stdlib>
using namespace metal;
#include "voronoi.metal"

struct OutlineParams{
    int width, height;
    float depth_thres, norm_thres;
    int ink_width;
};

kernel void clear_outline(
                          texture2d<float, access::write> outlineTex [[texture(0)]],
                          uint2 id [[thread_position_in_grid]]
                          ){
    outlineTex.write(float4(1.0,1.0,1.0,1.0), ushort2(id.x, id.y));
}

kernel void fill_outline(
                         texture2d<float, access::read> normalTex [[texture(0)]],
                         texture2d<float, access::read> depthTex [[texture(1)]],
                         texture2d<float, access::write> outlineTex [[texture(2)]],
                         constant OutlineParams& params [[buffer(0)]],
                         uint2 id [[thread_position_in_grid]]
                         )
{
//    float depth = depthTex.read(ushort2(id.x, id.y)).r;
    float depth = normalTex.read(ushort2(id.x, id.y)).w;
    float3 normal = normalTex.read(ushort2(id.x, id.y)).rgb;

    
    float ink = 1.0;
    
    if(depth < 1.0 - 0.001){ // only check if it is not a background pixel
        int modified_width = params.ink_width * exp(5*(-depth));
        
        for(int i=-1; i<=1; i++){
            for(int j=-1; j<=1; j++){
                
                uint2 neighbour = uint2(id.x+i*modified_width, id.y+j*modified_width);
                neighbour.x = clamp((int)neighbour.x, 0, params.width-1);
                neighbour.y = clamp((int)neighbour.y, 0, params.height-1);
                
                //            float depth_n = depthTex.read(neighbour).r;
                float depth_n = normalTex.read(neighbour).w;
                float3 normal_n = normalTex.read(neighbour).rgb;
                
                if(abs(depth-depth_n)>params.depth_thres || length(normal - normal_n)>params.norm_thres) ink = 0.0;
                
            }
        }
    }
    
//    float2 uv = float2((float)id.x / params.width, (float)id.y / params.height);
//    float2 worleyUV = uv * 10;
//    float2 noise = 2 * worley2D(worleyUV) - 1;
    
//    float2 noisyUV = uv + noise*0.007;
//    ushort2 noisyPixel = ushort2(noisyUV.x * params.width, noisyUV.y * params.height);
//    outlineTex.write(float4(ink,ink,ink,1.0), noisyPixel);
    outlineTex.write(float4(ink,ink,ink,1.0), ushort2(id.x,id.y));
//    outlineTex.write(float4(depth,depth,depth,1.0), ushort2(id.x,id.y));
}
