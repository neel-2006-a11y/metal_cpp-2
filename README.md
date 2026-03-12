# Real-Time Boids Simulation & Adaptive Halftone Renderer

This project implements two independent real-time graphics systems built using **Metal (metal-cpp)** and **C++**:

1. **GPU-accelerated Boids simulation**
2. **Adaptive surface-stable halftone / dithering renderer**

The project explores techniques in **real-time rendering, GPU shading, procedural patterns, and large-scale simulation**.

---

# Features

## 1. Boids Simulation

Implementation of Craig Reynolds' flocking model using GPU compute.

### Behaviors implemented

* **Separation** – avoid crowding neighbors
* **Alignment** – match heading with nearby boids
* **Cohesion** – move toward flock center

### Characteristics

* Real-time simulation of thousands of agents
* GPU-parallel neighbor evaluation
* Spatially coherent motion with emergent flocking behavior
* Adjustable behavioral parameters

---

## 2. Adaptive Halftone / Dither Renderer

A real-time **non-photorealistic rendering pipeline** that converts shaded surfaces into binary halftone patterns.

### Key capabilities

* **Surface-stable dithering**
* **Scale-adaptive Bayer pattern sampling**
* **Multi-resolution tile pyramid**
* **Lighting-aware thresholding**
* **Shadow-mapped directional lighting**

### Rendering pipeline

1. Geometry rendered with standard vertex transforms
2. Lighting computed in fragment stage
3. Pixel footprint estimated using `dfdx/dfdy`
4. Adaptive halftone tile scale selected using log-scale LOD
5. Bayer transition matrices produce smooth LOD transitions
6. Binary dithering applied using luminance thresholding

### Important techniques used

* Ordered dithering (Bayer matrices)
* Adaptive tile resolution
* Logarithmic level selection
* Surface-stable UV space sampling
* Procedural noise jitter for transition smoothing

---

# Technologies

* **C++**
* **Metal / Metal Shading Language**
* **metal-cpp**
* **GLFW**
* **GPU compute shaders**
* **Procedural rendering techniques**

---

# Project Structure

```
boids/
    boidManager.cpp
    boidCompute.metal

view/
    renderer.cpp
    camera.cpp
    mesh_factory.cpp

shaders/
    general3D.metal
    shadow.metal

core/
    main.cpp
```

---

# Example Concepts Demonstrated

### GPU Simulation

Large numbers of autonomous agents updated in parallel using compute shaders.

### Non-Photorealistic Rendering

Binary halftone rendering that mimics stylized shading techniques.

### Adaptive Sampling

Tile scale adjusts dynamically based on screen-space derivatives.

### Procedural Patterns

Bayer matrices and procedural noise generate structured dithering.

---

# Future Improvements

Possible directions for extension:

* Blue-noise halftone patterns
* Hierarchical tile reduction for halftone stability

---

# Build

Requires:

* macOS
* Xcode with Metal support

Build and run using Xcode or compile using `clang++` with metal shader compilation.

---

# Author

Neel Chaudhary
