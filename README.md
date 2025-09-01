# 2D Raytracing Engine

[![C](https://img.shields.io/badge/Language-C-blue.svg)](https://www.cprogramming.com/)
[![SDL](https://img.shields.io/badge/SDL-2.0-orange.svg)](https://www.libsdl.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

A real-time 2D raytracing engine built in C using SDL2. The engine features dynamic lighting, realistic shadows, and interactive controls. Light rays cast shadows as objects move through the scene.

## Features

- **Real-time raytracing** - 500 light rays rendered at 60+ FPS
- **Dynamic shadows** - Realistic shadow casting with moving objects
- **Interactive lighting** - Mouse-controlled light source
- **Smooth animations** - Obstacle moves along an elliptical path
- **Optimized rendering** - Analytical ray-circle intersections for performance
- **Visual effects** - Ray blur effects and efficient circle rendering
- **Cross-platform** - Works on Windows, Linux, and macOS

## Architecture

The engine is divided into three main components:

```
2D Raytracing Engine
├── Ray System
│   ├── Analytical intersection detection
│   ├── Pre-computed direction vectors
│   └── Optimized ray casting
├── Geometry Engine
│   ├── Circle primitives (light and obstacles)
│   ├── Scanline circle rendering
│   └── Bounds checking
└── Rendering Pipeline
    ├── Frame rate control
    ├── Double buffering
    └── Real-time scene updates
```

### Key Algorithms

- **Ray-Circle Intersection** - Uses quadratic formula for precise collision detection
- **Scanline Rendering** - Efficient circle filling without pixel-by-pixel iteration
- **Viewport Clipping** - Ray-boundary intersection for screen edge detection
- **Temporal Optimization** - Rays are regenerated only when the light source moves

## Controls

- **Mouse movement** - Move the light source around the screen
- **Close window** - Exit the simulation

## Quick Start

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get install libsdl2-dev

# macOS (with Homebrew)
brew install sdl2

# Windows
# Download SDL2 development libraries from https://www.libsdl.org/
```

### Building

```bash
# Clone the repository
git clone https://github.com/yourusername/2d-raytracing.git
cd 2d-raytracing

# Compile
gcc -o raytracing main.c -lSDL2 -lm

# Run
./raytracing
```

### Alternative Build (with Makefile)

```bash
make
make run
```

## Configuration

Modify constants in main.c to customize the simulation:

```c
#define WIDTH 1920          // Screen width
#define HEIGHT 1080         // Screen height
#define RAYS_NUMBER 500     // Number of light rays
#define RAY_THICKNESS 1     // Ray thickness
#define TARGET_FPS 60       // Target framerate
```

## Performance

- **Framerate**: 60+ FPS at 1920x1080 with 500 rays
- **Memory**: ~50MB RAM usage
- **CPU**: Single-threaded, optimized for modern processors
- **Scalability**: Linear performance scaling with ray count

## Benchmarks

(To be added)

## Technical Details

### Ray Casting Algorithm

1. **Ray Generation** - Pre-compute direction vectors from the light source
2. **Intersection Testing** - Analytical ray-circle collision detection
3. **Shadow Determination** - Find closest intersection point
4. **Rendering** - Draw rays from source to intersection or screen boundary

### Optimization Techniques

- **Analytical Math** - No iterative ray marching
- **Cached Calculations** - Pre-computed trigonometric values
- **Conditional Updates** - Ray regeneration only when needed
- **Efficient Memory Usage** - Minimal runtime allocations
