//
//  hatching.metal
//  metal_cpp
//
//  Created by Neel on 25/09/26.
//

#include <metal_stdlib>
#include "common.metal"
#include "voronoi.metal"
using namespace metal;

#define MAX_DIRS 7

struct MUHatch{
    float scale;
    int num_dir;
    float dir_angles[MAX_DIRS];
    float main_dir;
    
    int hasDiffuse;
    int texture_dim;
};


VertexOutput3D_2 vertex vertexMain3D_hatch(
                                           VertexInput3D_2 in [[stage_in]],
                                           constant FrameUniforms_2& frame[[buffer(FRAME_BUFFER_BIND)]],
                                           constant ObjectUniforms_2& object[[buffer(OBJ_BUFFER_BIND)]],
                                           constant ViewUniforms& view[[buffer(VIEW_BUFFER_BIND)]]
                                   ){
    VertexOutput3D_2 out;
    float4 worldPos = object.model * float4(in.position,1);
    out.position = view.viewProj * worldPos;
    out.color = in.color;
    out.normal = (transpose(object.invModel) * float4(in.normal,0)).xyz;
    out.worldPos = worldPos.xyz;
    out.uv = in.uv;
    out.viewDepth = (view.view * worldPos).z / view.far;
    return out;
}

FragmentOutput fragment fragmentMain3D_hatch(
                                             VertexOutput3D_2 in [[stage_in]],
                                             texture2d_array<float> shadowMap [[texture(SHADOW_MAP_BIND)]],
                                             sampler shadowSampler [[sampler(0)]],
                                             texture2d_array<half> hatchTex [[texture(1)]],
                                             sampler hatchSampler [[sampler(1)]],
                                             texture2d<float> diffuse [[texture(DIFFUSE_BIND)]],
                                             constant FrameUniforms_2& frame [[buffer(FRAME_BUFFER_BIND)]],
                                             constant ObjectUniforms_2& object [[buffer(OBJ_BUFFER_BIND)]],
                                             constant ViewUniforms& view[[buffer(VIEW_BUFFER_BIND)]],
                                             constant MUHatch& material [[buffer(MAT_BUFFER_BIND)]]
                                        ){
    if(view.has_oblique_plane){
        if(dot(float4(in.worldPos,1.0), view.plane) < 0){
            discard_fragment();
        }
    }
    
    float3 N = normalize(in.normal);
    float3 V = normalize(view.cameraPos - in.worldPos);

    float3 L = normalize(-frame.direction);
    float3 H = normalize(L + V);

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), 64.0);
    spec = 0;
    
    int selected_cascade = select_cascade(in.viewDepth * view.far, view.cascadeSplits, frame.cascades);
    
    float shadow = shadowCalculationCSM(in.worldPos, shadowMap, shadowSampler, view.sunVPs[selected_cascade], selected_cascade);
    
    float3 diffuse_color = float3(1.0);
    if(material.hasDiffuse == 1)
        diffuse_color = diffuse.sample(hatchSampler, in.uv).xyz;
    
    float3 light =
        frame.color * frame.intensity * diff +
        frame.color * spec * 0.2;
    
    light *= (1.0-shadow);
    
    light += 0.05*float3(1.0); // ambient light
    
    float luminance = dot(light, float3(0.299, 0.587, 0.114));
    float luminance_max = dot(float3(1.0,1.0,1.0), float3(0.299, 0.587, 0.114));
    luminance /= luminance_max;
//    float sqrt_lum = sqrt(luminance);
    
    
    float2 duvdx = dfdx(in.uv);
    float2 duvdy = dfdy(in.uv);
    
    float pixelCoverage = max(length(duvdx), length(duvdy));
    
    float MaxAdapt = exp2(10.0f);
    float MinAdapt = exp2(-3.0f);
    
    float UVscale = frame.BayerScale * material.scale / pixelCoverage; // pixelCoverage->high --> uv scaled low, hence less frequent pattern and less aliasing
    
    UVscale = clamp(UVscale, MinAdapt, MaxAdapt);
    float log_scale = log2(UVscale);
    float lod = floor(log_scale);
    float frac = fract(log_scale);
    int transition = material.texture_dim*frac;
//    return float(transition) / material.texture_dim;
//    int transition = material.texture_dim-1;
    
    float adaptiveScale = exp2(lod);
//    float adaptiveScale = 30;
    float2 sampleUV = fract(in.uv * adaptiveScale);
    float c_main = cos(material.main_dir), s_main = sin(material.main_dir);
    float2 mainUV = float2(sampleUV.x * c_main - sampleUV.y * s_main, sampleUV.x * s_main + sampleUV.y * c_main);
    
    
    float2 voronoiUV = in.uv * 400;
//    float2 voronoiUV = in.uv * adaptiveScale*10;
    float voronoi = worley_cell_val(voronoiUV);
    luminance += voronoi*0.2; // some light areas in between
    
    
    float dithered = 1.0;
    float threshold = 0.2 * (/*luminance*/+0.1); // threshold in the gradient texture
    int num_layers = min((1-luminance+0.2) * material.num_dir, (float)material.num_dir);
    
    for(int i=0; i<num_layers; i++){
        float c = cos(material.dir_angles[i]),s = sin(material.dir_angles[i]);
        float2 uv = float2(mainUV.x * c - mainUV.y * s, mainUV.x * s + mainUV.y * c);
        float value = hatchTex.sample(hatchSampler, uv, transition).r; 
        if(value > threshold){dithered = 0.0;break;}
    }
//    if(luminance<0.06)dithered = 0.0;
    
    
//    return float4(voronoi, 0, 0, 1);
    
    float npr_shadow = (int(luminance * 4)) / float(4);
    
    
    float3 out_color;
    
    out_color = dithered * diffuse_color * npr_shadow;
    
    FragmentOutput out;
//    out.color = float4(1.0,0.0,0.0,1.0);
    out.color = float4(out_color,1.0);
//    out.color = float4(voronoi, voronoi, voronoi,0);
//    out.color = float4(in.viewDepth,in.viewDepth,in.viewDepth,1.0);
    out.normal = float4(N,in.viewDepth);
    
//    discard_fragment();
    return out;
}
