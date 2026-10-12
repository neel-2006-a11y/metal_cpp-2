//
//  voronoi.metal
//  metal_cpp
//
//  Created by Neel on 30/09/26.
//

#include <metal_stdlib>
using namespace metal;
#ifndef VORONOI_METAL
#define VORONOI_METAL

static inline float2 hash22(int2 p)
{ // return feature point of grid
    uint2 x = uint2(p);

    x = x * uint2(1664525u, 1013904223u);

    uint n = x.x ^ (x.y * 374761393u);

    n = (n ^ (n >> 16)) * 2246822519u;
    n = (n ^ (n >> 13)) * 3266489917u;
    n = n ^ (n >> 16);

    float a = float(n) / 4294967295.0;

    n = n * 747796405u + 2891336453u;
    n = (n ^ (n >> 16)) * 2246822519u;
    n = n ^ (n >> 13);

    float b = float(n) / 4294967295.0;

    return float2(a, b);
}

static inline float worley_cell_val(float2 p){
    int2 cell = int2(floor(p));
    int2 min_cell = cell;
    float minDist = 100000.0;
    for (int y = -1; y <= 1; ++y)
    {
        for (int x = -1; x <= 1; ++x)
        {
            int2 neighbor = cell + int2(x, y);

            // Random point inside this cell.
            float2 feature = hash22(neighbor);

            // Convert feature point into the same coordinate
            // system as p.
            float2 featurePos =
                float2(neighbor) + feature;

            float2 delta = p - featurePos;

            float dist = dot(delta, delta);
            
            if(dist < minDist){
                minDist = dist;
                min_cell = neighbor;
            }
        }
    }
    
    float2 cell_val = hash22(min_cell);

    return cell_val.x;
}

static inline float worley2D(float2 p)
{
    int2 cell = int2 (floor(p));
    float minDist = 100000.0;
    for (int y = -1; y <= 1; ++y)
        {
            for (int x = -1; x <= 1; ++x)
            {
                int2 neighbor = cell + int2(x, y);

                // Random point inside this cell.
                float2 feature = hash22(neighbor);

                // Convert feature point into the same coordinate
                // system as p.
                float2 featurePos =
                    float2(neighbor) + feature;

                float2 delta = p - featurePos;

                float dist = dot(delta, delta);

                minDist = min(minDist, dist);
            }
        }

        return sqrt(minDist);
}


#endif
