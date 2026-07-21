# N Body simulation
A 2D and 3D gravitational **N-body simulation** of stars built in **C++** using **Raylib**.  
The project simulates simplified galactic dynamics with performance optimization using the **Barnes–Hut algorithm** and GPU instancing.

<img width="1000" height="800" alt="obraz" src="https://github.com/user-attachments/assets/463784ca-6589-48e8-841c-780d09258587" />
<p align="center">
  <b>Figure 1.</b> Beginning of the simulation.
</p>

## Features
### Physics Simulation
- Gravitational N-body simulation in 2D (`2d_simulation.cpp`) and 3D (`main.cpp`)
- **Barnes–Hut algorithm** for optimized force calculations
- Randomized star masses
- Realistic interaction-based motion
- Collision handling with **star merging**
- Dynamic mass, velocity and acceleration updates after collisions

### Simulation scale
- 1 second of real time = **5000 years** of simulation time
- Stars scaled using the factor of 0.2 Light Years and a power-law scaling relative to the Sun's radius

### Rendering and Visualization
- Efficient rendering of thousands of stars using GPU instancing
- Dynamically generated colors for merged stars
- Simple fragment shader implementing Bloom effect for star glow

### Camera System
- Zoom in/out support
- Pan the camera in 2D simulation
- Orbital or first person camera movement in 3D simulation

## Simulation
<img width="1000" height="800" alt="evolution" src="https://github.com/user-attachments/assets/2992b44a-deeb-4d48-8e44-a25391d85607" />
<p align="center">
  <b>Figure 2. </b>Merged stars in 2D simulation.
</p>

<img width="1000" height="800" alt="evolution3d" src="https://github.com/user-attachments/assets/2ff94b17-886c-401a-935d-8b2f874ad459" />
<p align="center">
  <b>Figure 3. </b>Merged stars in 3D simulation with an orbital camera.
</p>

<img width="1000" height="800" alt="evolution_3d_gpu_instancing" src="https://github.com/user-attachments/assets/6fe7d2fd-32c3-47d0-b7f1-af8cb7760235" />

<p align="center">
  <b>Figure 4. </b>3D simulation with 2000 stars using GPU instancing.
</p>

## Algorithm
The simulation combines physics optimization algorithms with GPU rendering techniques.

### Barnes–Hut Algorithm
Naive approach, which calculates direct gravitational force between all stars has computational complexity of **O(n²)**

To improve performance, the simulation uses the Barnes–Hut approximation, reducing the complexity to approximately **~O(n log n)**

The simulation space is recursively subdivided into smaller regions using an Octree structure.
Distant clusters of stars are approximated as a single mass point, reducing the number of required force calculations.

### GPU Instanced Rendering
To limit the number of draw calls in each frame the simulation uses GPU instancing.

### Bloom Shader
The visualization includes a simple shader implemented using a Gaussian filter and produces a realistic glow effect around the stars.

## Controls
| Action | Input |
|--------|-------|
| Zoom in/out | Mouse wheel |
| Pan (2D) | Middle Mouse Button + Drag |

## Getting Started

### Requirements
- C++17 compiler
- Raylib
- Make

### Clone repository
```bash
git clone https://github.com/sz-musialik/N-body-Simulation.git
cd N-body-Simulation
```

### Build and Run
```bash
make run
```
_This project assumes the default Raylib installation directory on Windows (`C:/raylib/raylib/src`)._
