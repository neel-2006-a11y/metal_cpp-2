//
//  boids_compute-2.metal
//  metal_cpp
//
//  Created by Neel on 24/01/26.
//

#include <metal_stdlib>
using namespace metal;

struct Boid
{
    float2 position;
    float2 velocity;
    float3 color;
};

struct gridParams{
    float cellSize;
    uint32_t gridDim;
    uint32_t numCells;
    uint32_t numBoids;
};

struct SimParams {
    float2 mousePos;
    float mouseForce;
    
    float2 flowDirection;
    float flowStrength;
    
    uint32_t frame;
    uint32_t kCount;
    float dt;
};

constant float perceptionRadius = 0.2f;
constant float seperationRadius = 0.005f;

constant float sepWeight = 0.6f;
constant float aliWeight = 1.3f;
constant float cohWeight = 1.3f;

constant float maxSpeed = 0.6f;

uint computeCellID(float2 pos, float cellSize, uint32_t gridDim){
    uint2 c = uint2(
                    clamp((pos.x+1)/cellSize, 0.0, float(gridDim-1)),
                    clamp((pos.y+1)/cellSize, 0.0, float(gridDim-1))
                    );
    return c.x + c.y * gridDim;
}


kernel void clearCellCounts(
    device atomic_uint* cellCounts [[ buffer(0) ]],
    constant gridParams& gridParam [[ buffer(1)]],
    uint id [[thread_position_in_grid]]
){
        if(id < gridParam.numCells)
            atomic_store_explicit(&cellCounts[id], 0 , memory_order_relaxed);
}


kernel void countBoids(
   device Boid* boids [[ buffer(0) ]],
   device atomic_uint* cellCounts [[ buffer(1) ]],
   constant gridParams& gridParam [[ buffer(2)]],
   uint id [[thread_position_in_grid]]
)
{
    if(id >= gridParam.numBoids) return;
    boids[id].color = float3(1.0,0.0,0.0); // red
    
    uint cell = computeCellID(boids[id].position, gridParam.cellSize, gridParam.gridDim);
    atomic_fetch_add_explicit(&cellCounts[cell], 1, memory_order_relaxed);
}

// cpu side prefix-sum to find cellStart (cellOffsets), cellFinal
// <----- GOES HERE

kernel void scatterBoids(
     device const Boid* boids [[ buffer(0) ]],
     device atomic_uint* cellCounts [[ buffer(1) ]],
     device const uint* cellOffsets [[ buffer(2) ]],
     device uint* cellIndices [[ buffer(3) ]],
     constant gridParams& gridParam [[ buffer(4) ]],
     uint id [[thread_position_in_grid]]
){
    if(id >= gridParam.numBoids) return;
    
    uint cell = computeCellID(boids[id].position, gridParam.cellSize, gridParam.gridDim);
    uint index = atomic_fetch_sub_explicit(&cellCounts[cell], 1, memory_order_relaxed);
    if(index>0){
        cellIndices[cellOffsets[cell] + index-1] = id;
    }
}


kernel void updateBoids_grid_2(
    device Boid* boidsIn [[buffer(0)]],
    device Boid* boidsOut [[buffer(1)]],
    constant SimParams& params [[buffer(2)]],
    device uint* cellIndices [[ buffer(3) ]],
    device uint* cellStart [[ buffer(4) ]],
    device uint* cellFinal [[ buffer(5) ]],
    constant gridParams& gridParam [[ buffer(6) ]],
    uint id [[thread_position_in_grid]]
){
    Boid self = boidsIn[id];
    uint gridDim = gridParam.gridDim;
    float cellSize = gridParam.cellSize;
    
    uint2 c = uint2(
                    clamp((boidsIn[id].position.x+1)/cellSize, 0.0, float(gridDim-1)),
                    clamp((boidsIn[id].position.y+1)/cellSize, 0.0, float(gridDim-1))
                    );
    
    float2 seperation = float2(0);
    float2 alignment = float2(0);
    float2 cohesion = float2(0);
    uint count = 0;
    
    
    for(int oy = -1; oy <= 1; oy++){
        for(int ox = -1; ox <=1; ox++){
            int2 nc = int2(c) + int2(ox,oy);
            
            // when self is at the edge, and other is on the other side, it goes back from edge instead of going through
            
            nc.x = (nc.x+gridDim)%gridDim;
            nc.y = (nc.y+gridDim)%gridDim;
//            if(nc.x > int(gridDim) || nc.x < 0  || nc.y > int(gridDim) || nc.y < 0)continue;
            uint cellIndex = nc.y * gridDim + nc.x;
            
            for(uint j = cellStart[cellIndex]; j < cellFinal[cellIndex]; j++){
                uint otherID = cellIndices[j];
                if(otherID == id){
                    continue;
                }
                if(id==1){
                    boidsOut[otherID].color = float3(0.0,1.0,0.0); // blue
                    boidsIn[otherID].color = float3(0.0,1.0,0.0);
                }
                
                Boid other = boidsIn[otherID];
                float2 diff = other.position - self.position;
                float dist = length(diff);
                
                if(dist < perceptionRadius && dist > 0.000001){
                    count++;
                    alignment += other.velocity;
                    cohesion += other.position;
                    
                    if(dist < seperationRadius){
                        seperation -= diff/dist;
                    }
                }

            }
        }
    }
    if(count > 0){
        alignment /= count;
        cohesion = (cohesion/count) - self.position;
    }
    
    // mouse interaction;
    float2 mousePos = params.mousePos;
    float2 mouseVec = mousePos - self.position;
    
    float2 mouseForce = 0;
    if(mousePos.x < 1.0f && mousePos.x > -1.0f && mousePos.y < 1.0f && mousePos.y >-1.0f){
        float mouseDist = length(mouseVec);
        if(mouseDist < 0.5){
            mouseForce = normalize(mouseVec) * params.mouseForce / (mouseDist+0.01);
        }
    }
    
    // global flow
    float2 flow = normalize(params.flowDirection) * params.flowStrength;
    
    float2 accel =
    sepWeight * seperation
    + aliWeight * alignment
    + cohWeight * cohesion
    + mouseForce
    + flow;
    
    self.velocity += accel * params.dt;
    
    // speed limit
    float speed = length(self.velocity);
    if(speed > maxSpeed)
        self.velocity = normalize(self.velocity) * maxSpeed;
    
    self.position += self.velocity * params.dt;
    
    // screen wrap
    if (self.position.x > 1) self.position.x = -1;
    if (self.position.x < -1) self.position.x = 1;
    if (self.position.y > 1) self.position.y = -1;
    if (self.position.y < -1) self.position.y = 1;

    boidsOut[id] = self;
}
