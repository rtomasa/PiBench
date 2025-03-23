# PiBench

**Raspberry Pi Performance Evaluation Core for libretro/RetroArch**  

A specialized benchmarking core designed to stress-test **Raspberry Pi** hardware:

- **CPU Computational Throughput**  
  _Matrix operations • Floating-point math • SIMD-optimized workloads_
  
- **2D Rendering Efficiency**  
  _Pixel buffers • Scrolling patterns • Geometric primitives • Alpha blending_
  
- **3D Graphics Prowess**  
  _Software-rendered polygons • Vertex transformations • Depth testing • Custom shaders_

## Key Features
- **ARM64-Optimized Pipeline**
  Leverages NEON SIMD intrinsics for maximal CPU utilization
- **GLES 3.1 Rendering Backend**
  Hardware-accelerated graphics with modern feature support
- **Configurable Stress Levels**
  Dynamic test parameters (matrix size/texture res/geometry complexity)
- **Real-Time Metrics Overlay**
  FPS • Frame Time • CPU Load • Thermal Throttling Detection

## Compatibility Matrix
| Device              | CPU Test | 2D Test | 3D Test |
|---------------------|----------|---------|---------|
| Raspberry Pi 3B+    | ✅       | ✅      | ⚠️      |
| Raspberry Pi 4B     | ✅       | ✅      | ✅      |
| Raspberry Pi 400    | ✅       | ✅      | ✅      |
| Raspberry Pi 5      | ✅       | ✅      | ✅      |
| x86/x64 Systems     | ❌       | ❌      | ❌      |

**Requirements:**  
- OpenGL ES 3.1+ compatible GPU
- 64-bit OS (Raspberry Pi OS/Bullseye+ recommended)
- RePlay OS v1.0.0+ or RetroArch v1.9.6+ with GLES context

## Installation
```bash
# Dependencies
sudo apt install libgles2-mesa-dev build-essential

# Build & Install
git clone https://github.com/rtomasa/PiBench
cd pibench
make
```