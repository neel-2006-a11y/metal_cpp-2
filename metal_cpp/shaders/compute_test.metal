//
//  compute_test.metal
//  metal_cpp
//
//  Created by Neel on 17/01/26.
//

#include <metal_stdlib>
using namespace metal;

struct InstanceData
{
    float offset;
};

kernel void write_indices(
    device InstanceData* instances [[buffer(0)]],
    uint id [[thread_position_in_grid]]
){
    instances[id].offset = float(id) * 0.05;
}
