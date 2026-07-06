# N Body simulation
A 2D gravitational **N-body simulation** of stars built in **C++** using **Raylib**.  
The project simulates simplified galactic dynamics with performance optimization using the **Barnes–Hut algorithm**.

<img width="1000" height="800" alt="obraz" src="https://github.com/user-attachments/assets/463784ca-6589-48e8-841c-780d09258587" />
<p align="center">
  <em><b>Figure 1.</b> Beginning of the simulation</em>
</p>

## Features
### Physics Simulation
- Gravitational N-body simulation in 2D
- **Barnes–Hut algorithm** for optimized force calculations
- Randomized star masses
- Realistic interaction-based motion
- Collision handling with **star merging**

### Simulation scale
- 1 second of real time = **5000 years** of simulation time
- Stars scaled using power-law scaling relative to the Sun's radius

### Visualization
- Merged stars dynamically colored

### Camera System
- Zoom in/out support
- Pan the camera

## Screenshots
<img width="1000" height="800" alt="evolution" src="https://github.com/user-attachments/assets/2992b44a-deeb-4d48-8e44-a25391d85607" />
<p align="center">
  <em><b>Figure 2.</b> Merged stars</em>
</p>

## Algorithm
This project uses the **Barnes–Hut approximation** to reduce computational complexity
- Naive approach: **O(n²)**
- Barnes-Hut: ~**O(n log n)**

The simulation space is recursively divided into quadrants using a quadtree structure.  
Distant clusters of stars are approximated as a single mass point.

## Controls
| Action | Input |
|--------|-------|
| Zoom in/out | Mouse wheel |
| Pan | Middle Mouse Button + Drag |

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
