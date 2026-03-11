//
//  dithering.h
//  metal_cpp
//
//  Created by Neel on 04/03/26.
//

#pragma once

#include "config.h"

simd::float4x4 Bayer4x4();
void Bayer4x4Transitions(float (&Transitions)[4][8][8]);
