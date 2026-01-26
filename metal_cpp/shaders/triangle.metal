////
////  triangle.metal
////  metal_cpp
////
////  Created by Neel on 14/01/26.
////
//
//#include <metal_stdlib>
//using namespace metal;
//
//// basic test
//constant float4 positions[] = {
//    float4(-0.75,-0.75,0.0,1.0),
//    float4(0.75,-0.75,0.0,1.0),
//    float4(0.0,0.75,0.0,1.0)
//};
//constant float3 colors[] = {
//    float3(1.0,0.0,0.0),
//    float3(0.0,1.0,0.0),
//    float3(0.0,0.0,1.0)
//};
//
//
//struct vertexPayload{
//    float4 position [[position]];
//    float3 color;
//};
//
//vertexPayload vertex vertexMain(uint vertexID [[vertex_id]]){
//    vertexPayload payload;
//    
//    payload.position = positions[vertexID];
//    payload.color = colors[vertexID];
//    
//    return payload;
//}
//
//float4 fragment fragmentMain(vertexPayload frag [[stage_in]]){
//    return float4(frag.color, 1.0);
//}
