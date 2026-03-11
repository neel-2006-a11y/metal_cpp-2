//
//  mesh_factory.cpp
//  metal_cpp
//
//  Created by Neel on 14/01/26.
//

#include "view/mesh_factory.h"

Mesh MeshFactory::buildTriangle(MTL::Device* device) {
    
    Mesh mesh;
    //Declare the data to send
    Vertex vertices[3] = {
        {{-0.75, -0.75}, {1.0, 0.0, 0.0}},
        {{ 0.75, 0.0}, {0.0, 1.0, 0.0}},
        {{  -0.75,  0.75}, {0.0, 0.0, 1.0}}
    };
    
    ushort indices[3] = {0,1,2};
    mesh.indexCount = 3;
    
    //vertex buffer
    mesh.vertexBuffer = device->newBuffer(3 * sizeof(Vertex), MTL::ResourceStorageModeShared);
    memcpy(mesh.vertexBuffer->contents(), vertices, 3 * sizeof(Vertex));
    
    //index buffer
    mesh.indexBuffer = device->newBuffer(3 * sizeof(ushort), MTL::ResourceStorageModeShared);
    memcpy(mesh.indexBuffer->contents(), indices, 3 * sizeof(ushort));
    
    // vertex descriptor
    MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();
    auto attributes = vertexDescriptor->attributes();
    //position: vec2
    auto positionDescriptor = attributes->object(0);
    positionDescriptor->setFormat(MTL::VertexFormat::VertexFormatFloat2);
    positionDescriptor->setBufferIndex(0);
    positionDescriptor->setOffset(0);
    // color: vec3
    auto colorDescriptor = attributes->object(1);
        colorDescriptor->setFormat(MTL::VertexFormat::VertexFormatFloat3);
        colorDescriptor->setBufferIndex(0);
        colorDescriptor->setOffset(offsetof(Vertex, color));
    
    auto layoutDescriptor = vertexDescriptor->layouts()->object(0);
    layoutDescriptor->setStride(sizeof(Vertex));
    
    mesh.vertexDescriptor = vertexDescriptor;
    
    
    return mesh;
}

Mesh MeshFactory::buildTriangle3D(MTL::Device* device) {
    
    Mesh mesh;
    //Declare the data to send
    simd::float3 positions[3] = {
        {-0.75,-0.75,0.0},
        {0.75,0.0,0.0},
        {-0.75,0.75,0.0}
    };
    simd::float3 colors[3] = {
        {1.0,0.0,0.0},
        {0.0,1.0,0.0},
        {0.0,0.0,1.0}
    };
    
    ushort indices[3] = {0,1,2};
    mesh.indexCount = 3;
    
    Vertex3D vertices[3];
    for(int i = 0; i < 3;i++){
        vertices[i].position = positions[i];
        vertices[i].color = colors[i];
    };
    //vertex buffer
    mesh.vertexBuffer = device->newBuffer(3 * sizeof(Vertex3D), MTL::ResourceStorageModeShared);
    memcpy(mesh.vertexBuffer->contents(), vertices, 3 * sizeof(Vertex3D));
    
    //index buffer
    mesh.indexBuffer = device->newBuffer(3 * sizeof(ushort), MTL::ResourceStorageModeShared);
    memcpy(mesh.indexBuffer->contents(), indices, 3 * sizeof(ushort));
    
    // vertex descriptor
    MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();
    auto attributes = vertexDescriptor->attributes();
    //position: vec3
    auto positionDescriptor = attributes->object(0);
    positionDescriptor->setFormat(MTL::VertexFormat::VertexFormatFloat3);
    positionDescriptor->setBufferIndex(0);
    positionDescriptor->setOffset(0);
    // color: vec3
    auto colorDescriptor = attributes->object(1);
        colorDescriptor->setFormat(MTL::VertexFormat::VertexFormatFloat3);
        colorDescriptor->setBufferIndex(0);
        colorDescriptor->setOffset(offsetof(Vertex3D, color));
    // uv: vec2
    auto uvDescriptor = attributes->object(2);
        uvDescriptor->setFormat(MTL::VertexFormat::VertexFormatFloat2);
        uvDescriptor->setBufferIndex(0);
        uvDescriptor->setOffset(offsetof(Vertex3D, uv));
    // normal: vec3
    auto normalDescriptor = attributes->object(3);
        normalDescriptor->setFormat(MTL::VertexFormat::VertexFormatFloat3);
        normalDescriptor->setBufferIndex(0);
        normalDescriptor->setOffset(offsetof(Vertex3D, normal));
    
    auto layoutDescriptor = vertexDescriptor->layouts()->object(0);
    layoutDescriptor->setStride(sizeof(Vertex3D));
    
    mesh.vertexDescriptor = vertexDescriptor;
    
    
    return mesh;
}

Mesh MeshFactory::buildQuad(MTL::Device* device) {
    
    Mesh mesh;
    
    //Declare the data to send
    Vertex vertices[4] = {
        {{-0.75, -0.75}, {1.0, 0.0, 0.0}},
        {{ 0.75, -0.75}, {0.0, 1.0, 0.0}},
        {{ 0.75,  0.75}, {0.0, 0.0, 1.0}},
        {{-0.75,  0.75}, {0.0, 1.0, 0.0}},
    };
    
    ushort indices[6] = {0, 1, 2, 2, 3, 0};
    mesh.indexCount = 6;
    
    //vertex buffer
    mesh.vertexBuffer = device->newBuffer(4 * sizeof(Vertex), MTL::ResourceStorageModeShared);
    memcpy(mesh.vertexBuffer->contents(), vertices, 4 * sizeof(Vertex));
    
    //index buffer
    mesh.indexBuffer = device->newBuffer(6 * sizeof(ushort), MTL::ResourceStorageModeShared);
    memcpy(mesh.indexBuffer->contents(), indices, 6 * sizeof(ushort));
    
    // vertex descriptor
    MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();
    auto attributes = vertexDescriptor->attributes();
    //position: vec2
    auto positionDescriptor = attributes->object(0);
    positionDescriptor->setFormat(MTL::VertexFormat::VertexFormatFloat2);
    positionDescriptor->setBufferIndex(0);
    positionDescriptor->setOffset(0);
    //color: vec3
    auto colorDescriptor = attributes->object(1);
    colorDescriptor->setFormat(MTL::VertexFormat::VertexFormatFloat3);
    colorDescriptor->setBufferIndex(0);
    colorDescriptor->setOffset(offsetof(Vertex, color));

    auto layoutDescriptor = vertexDescriptor->layouts()->object(0);
    layoutDescriptor->setStride(sizeof(Vertex));
    
    mesh.vertexDescriptor = vertexDescriptor;
    return mesh;
}

Mesh MeshFactory::buildCube(MTL::Device *device){
    Mesh mesh;
    
    simd::float3 positions[] = {
            // Front
            {-0.5, -0.5,  0.5}, { 0.5, -0.5,  0.5}, { 0.5,  0.5,  0.5}, { -0.5,  0.5,  0.5},
            // Back
            {-0.5, -0.5, -0.5}, { 0.5, -0.5, -0.5}, { 0.5,  0.5, -0.5}, { -0.5,  0.5, -0.5}
        };
    simd::float3 normals[] = {
            {0, 0, 1}, {0, 0, -1}, {1, 0, 0},
            {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}
        };
    simd::float2 uvs[] = {
            {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}
        };
    simd::float3 colors[] = {
            {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 0},
            {1, 0, 1}, {0, 1, 1}
        };
    struct Face {
            int v[4];
            simd::float3 n;
            simd::float3 c;
        };

    Face faces[] = {
        {{0, 1, 2, 3}, normals[0], colors[0]}, // front
        {{5, 4, 7, 6}, normals[1], colors[1]}, // back
        {{1, 5, 6, 2}, normals[2], colors[2]}, // right
        {{4, 0, 3, 7}, normals[3], colors[3]}, // left
        {{3, 2, 6, 7}, normals[4], colors[4]}, // top
        {{4, 5, 1, 0}, normals[5], colors[5]}  // bottom
    };
    
    std::vector<Vertex3D> vertices;
    for (auto& f : faces) {
        for (int i = 0; i < 4; i++) {
            Vertex3D v;
            v.position = positions[f.v[i]];
            v.normal = f.n;
            v.color = f.c;
            v.uv = uvs[i];
            vertices.push_back(v);
        }
    }
    
    std::vector<ushort> indices;
    for (ushort i = 0; i < 6; i++) {
        ushort start = i * ushort(4);
        indices.insert(indices.end(), {
            start, static_cast<unsigned short>(start + 1), static_cast<unsigned short>(start + 2),
            start, static_cast<unsigned short>(start + 2), static_cast<unsigned short>(start + 3)
        });
    }
    mesh.indexCount = 36;
    
    //vertex buffer
    mesh.vertexBuffer = device->newBuffer(24 * sizeof(Vertex3D), MTL::ResourceStorageModeShared);
    memcpy(mesh.vertexBuffer->contents(), vertices.data(), 24 * sizeof(Vertex3D));
    
    //index buffer
    mesh.indexBuffer = device->newBuffer(36 * sizeof(ushort), MTL::ResourceStorageModeShared);
    memcpy(mesh.indexBuffer->contents(), indices.data(), 36 * sizeof(ushort));
    
    // vertex descriptor
    MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();
    auto attributes = vertexDescriptor->attributes();
    //position: vec3
    auto positionDescriptor = attributes->object(0);
    positionDescriptor->setFormat(MTL::VertexFormat::VertexFormatFloat3);
    positionDescriptor->setBufferIndex(0);
    positionDescriptor->setOffset(0);
    // color: vec3
    auto colorDescriptor = attributes->object(1);
        colorDescriptor->setFormat(MTL::VertexFormat::VertexFormatFloat3);
        colorDescriptor->setBufferIndex(0);
        colorDescriptor->setOffset(offsetof(Vertex3D, color));
    // uv: vec2
    auto uvDescriptor = attributes->object(2);
        uvDescriptor->setFormat(MTL::VertexFormat::VertexFormatFloat2);
        uvDescriptor->setBufferIndex(0);
        uvDescriptor->setOffset(offsetof(Vertex3D, uv));
    // normal: vec3
    auto normalDescriptor = attributes->object(3);
        normalDescriptor->setFormat(MTL::VertexFormat::VertexFormatFloat3);
        normalDescriptor->setBufferIndex(0);
        normalDescriptor->setOffset(offsetof(Vertex3D, normal));
    
    auto layoutDescriptor = vertexDescriptor->layouts()->object(0);
    layoutDescriptor->setStride(sizeof(Vertex3D));
    
    mesh.vertexDescriptor = vertexDescriptor;
    return mesh;
}

Mesh MeshFactory::buildSphere(MTL::Device* device,
                               uint16_t stacks,
                               uint16_t slices,
                               float radius)
{
    std::vector<Vertex3D> vertices;
    std::vector<uint16_t> indices;

    for (uint16_t i = 0; i <= stacks; ++i)
    {
        float v = float(i) / stacks;
        float phi = v * M_PI; // 0 → π

        for (uint32_t j = 0; j <= slices; ++j)
        {
            float u = float(j) / slices;
            float theta = u * 2.0f * M_PI; // 0 → 2π

            float x = std::sin(phi) * std::cos(theta);
            float y = std::cos(phi);
            float z = std::sin(phi) * std::sin(theta);

            Vertex3D vert;
            vert.position = { x * radius, y * radius, z * radius };
            vert.normal   = simd::normalize(simd::float3{x, y, z});
            vert.uv       = { u, v };
            vert.color    = simd::float3{1.0,1.0,1.0};

            vertices.push_back(vert);
        }
    }

    // Indices
    for (uint16_t i = 0; i < stacks; ++i)
    {
        for (uint16_t j = 0; j < slices; ++j)
        {
            uint16_t row1 = i * (slices + 1);
            uint16_t row2 = (i + 1) * (slices + 1);

            indices.push_back(row1 + j);
            indices.push_back(row2 + j);
            indices.push_back(row1 + j + 1);

            indices.push_back(row1 + j + 1);
            indices.push_back(row2 + j);
            indices.push_back(row2 + j + 1);
        }
    }

    Mesh mesh{};

    // Vertex buffer
    mesh.vertexBuffer = device->newBuffer(
        vertices.data(),
        vertices.size() * sizeof(Vertex3D),
        MTL::ResourceStorageModeManaged
    );

    // Index buffer
    mesh.indexBuffer = device->newBuffer(
        indices.data(),
        indices.size() * sizeof(uint16_t),
        MTL::ResourceStorageModeManaged
    );

    mesh.indexCount = static_cast<uint>(indices.size());

    // vertex descriptor
    MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();
    auto attributes = vertexDescriptor->attributes();
    //position: vec3
    auto positionDescriptor = attributes->object(0);
    positionDescriptor->setFormat(MTL::VertexFormat::VertexFormatFloat3);
    positionDescriptor->setBufferIndex(0);
    positionDescriptor->setOffset(0);
    // color: vec3
    auto colorDescriptor = attributes->object(1);
        colorDescriptor->setFormat(MTL::VertexFormat::VertexFormatFloat3);
        colorDescriptor->setBufferIndex(0);
        colorDescriptor->setOffset(offsetof(Vertex3D, color));
    // uv: vec2
    auto uvDescriptor = attributes->object(2);
        uvDescriptor->setFormat(MTL::VertexFormat::VertexFormatFloat2);
        uvDescriptor->setBufferIndex(0);
        uvDescriptor->setOffset(offsetof(Vertex3D, uv));
    // normal: vec3
    auto normalDescriptor = attributes->object(3);
        normalDescriptor->setFormat(MTL::VertexFormat::VertexFormatFloat3);
        normalDescriptor->setBufferIndex(0);
        normalDescriptor->setOffset(offsetof(Vertex3D, normal));
    
    auto layoutDescriptor = vertexDescriptor->layouts()->object(0);
    layoutDescriptor->setStride(sizeof(Vertex3D));
    
    mesh.vertexDescriptor = vertexDescriptor;

    return mesh;
}
