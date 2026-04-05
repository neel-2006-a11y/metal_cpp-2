//
//  composite.metal
//  metal_cpp
//
//  Created by Neel on 05/04/26.
//

#include <metal_stdlib>
using namespace metal;

struct VSOut {
    float4 position [[position]];
    float2 uv;
};

vertex VSOut compositeVS(uint vid [[vertex_id]]){
    float2 pos[3] = {
        float2(-1.0, -1.0),
        float2( 3.0, -1.0),
        float2(-1.0,  3.0)
    };

    float2 uv[3] = {
        float2(0.0, 0.0),
        float2(2.0, 0.0),
        float2(0.0, 2.0)
    };

    VSOut out;
    out.position = float4(pos[vid], 0.0, 1.0);
    out.uv = uv[vid];
    return out;
}

fragment float4 compositeFS(VSOut in [[stage_in]],
                            texture2d<float> sceneTex [[texture(0)]],
                            texture2d<float> volumeTex [[texture(1)]],
                            sampler samp [[sampler(0)]]){
    float2 uv = in.uv;
    float2 uv_flipped = float2(uv.x, 1-uv.y);
    float4 sceneCol = sceneTex.sample(samp, uv_flipped);
    float4 volumeCol = volumeTex.sample(samp, uv_flipped);
    return sceneCol * 0.5 + volumeCol * 0.5;
}
