//
//  mtlm.h
//  metal_cpp
//
//  Created by Neel on 15/01/26.
//

#pragma once
#include <simd/simd.h>

namespace mtlm {

simd::float4x4 identity();
simd::float4x4 translation(simd::float3 dPos);
simd::float4x4 z_rotation(float theta);
simd::float4x4 y_rotation(float theta);
simd::float4x4 scale(float factor);
simd::float4x4 scale3D(simd::float3 factor);
simd::float4x4 perspective_projection(float fovy, float aspect, float near, float far);
simd::float4x4 orthographic_projection(float x1, float x2, float y1, float y2, float z1, float z2);
simd::float4x4 look_at(simd::float3 from, simd::float3 target, simd::float3 up);
}


