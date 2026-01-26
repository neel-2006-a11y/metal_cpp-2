//
//  boids_compute.metal
//  metal_cpp
//
//  Created by Neel on 17/01/26.
//

#include <metal_stdlib>
using namespace metal;

struct Boid
{
    float2 position;
    float2 velocity;
    float3 color;
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

constant float perceptionRadius = 0.3f;
constant float seperationRadius = 0.02f;

constant float sepWeight = 0.6f;
constant float aliWeight = 1.3f;
constant float cohWeight = 1.3f;

constant float maxSpeed = 0.3f;

kernel void updateBoids(
    device const Boid* boidsIn [[buffer(0)]],
    device Boid* boidsOut [[buffer(1)]],
    constant SimParams& params [[buffer(2)]],
    uint id [[thread_position_in_grid]]
){
    Boid self = boidsIn[id];
    
    float2 seperation = float2(0);
    float2 alignment = float2(0);
    float2 cohesion = float2(0);
    
    int count = 0;
    for(uint i = 0; i < params.kCount; i++){
        if(i == id)continue;
        
        Boid other = boidsIn[i];
        float2 diff = other.position - self.position;
        float dist = length(diff);
        
        if(dist < perceptionRadius && dist > 0.000001){
            alignment += other.velocity;
            cohesion += other.position;
            count++;
            
            if(dist < seperationRadius){
                seperation -= diff/dist;
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

#define MAX_BOIDS_PER_CELL 128
struct Cell{
    atomic_uint count;
    uint32_t indices[MAX_BOIDS_PER_CELL];
};

kernel void updateBoids_grid(
    device Boid* boidsIn [[buffer(0)]],
    device Boid* boidsOut [[buffer(1)]],
    constant SimParams& params [[buffer(2)]],
    device Cell* grid [[buffer(3)]],
    constant float& cellSize [[buffer(4)]],
    constant uint32_t& gridDim [[buffer(5)]],
    uint id [[thread_position_in_grid]]
){
    Boid self = boidsIn[id];
    
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
            device Cell* cellData = &grid[cellIndex];
            uint n = atomic_load_explicit(&(cellData->count), memory_order_relaxed);
            
            for(uint j = 0; j < n; j++){
                uint otherID = cellData->indices[j];
                if(otherID == id) continue;
                
                
                Boid other = boidsIn[otherID];
                if(id==1){
                    boidsOut[otherID].color = float3(0.0,0.0,1.0); // blue // in the grid with boid 1
                    boidsIn[otherID].color = float3(0.0,0.0,1.0);
                }
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
    
    count--;
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
    /*+ flow*/;
    
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

kernel void clearGrid(
      device Cell* grid [[buffer(0)]],
      constant uint32_t& gridCellCount [[buffer(1)]],
      uint id [[ thread_position_in_grid]]
){
    if(id >= gridCellCount) return;
    atomic_store_explicit(&grid[id].count, 0, memory_order_relaxed);
}

kernel void insertBoids(
                        device Boid* boids [[buffer(0)]],
                        device Cell* grid [[buffer(1)]],
                        constant float& cellSize [[buffer(2)]],
                        constant uint32_t& gridDim [[buffer(3)]],
                        uint id [[thread_position_in_grid]]
                        ){
    boids[id].color = float3(1.0,0.0,0.0); //red // not assigned to grid
    uint2 c = uint2(
                    clamp((boids[id].position.x+1)/cellSize, 0.0, float(gridDim-1)),
                    clamp((boids[id].position.y+1)/cellSize, 0.0, float(gridDim-1))
                    );
    
    uint index = c.y * gridDim + c.x;
    
    uint slot = atomic_fetch_add_explicit(&grid[index].count, 1, memory_order_relaxed);
    if(slot < MAX_BOIDS_PER_CELL){
        boids[id].color = float3(0.0,1.0,0.0); //green // assigned to a grid
        grid[index].indices[slot] = id;
    }
}
