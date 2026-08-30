# Tower Defense 3D (C++ / CHAI3D Engine)

A 3D Tower Defense simulation engineered in **C++**, integrating the **CHAI3D** framework alongside **GLFW/OpenGL**. Players command a dual-degree-of-freedom (Yaw/Pitch) artillery turret to intercept and neutralize incoming waves of aerial and ground hostiles.

---

## 🌟 Key Features

* **Turret Mechanics:**
  * Real-time kinematics controlling horizontal rotation (Yaw) and barrel elevation (Pitch)
  * Ballistic missile launching with integrated cooldown latency
* **Enemy:**
  * Supports 4 distinct hostile classes: Ground Tank, Low-altitude Aircraft, Mid-altitude Aircraft, and High-altitude Spaceship
  * Specialized configurations per class, encompassing vertical altitude (Z-Offset), traversal velocity, and geometric scale
  * Autonomous waypoint alignment utilizing directional rotation matrices
* **Collision Detection:**
  * Efficient **Bounding Sphere** collision algorithms ($distance < R_1 + R_2$)
  * Automated entity deallocation upon projectile impact or map boundary transgression

---

## 🎮 Control Scheme

| Input / Keystroke | Operational Action |
| :--- | :--- |
| **Arrow Left / Right (`←` / `→`)** | Rotate Turret Base (Yaw) |
| **Arrow Up / Down (`↑` / `↓`)** | Elevate / Depress Barrel (Pitch) |
| **Spacebar (`SPACE`)** | Launch Missile |
| **Right Mouse Button + Drag** | Orbit Camera View (Disabled) |
| **Mouse Scroll Wheel** | Adjust Focal Distance (Zoom In / Zoom Out) (Disabled) |
| **Escape Key (`ESC`)** | Terminate Execution |

---

## 🛠️ Build and Execution

* **Prerequisites**
  * C++ Compiler with C++11 standard support or higher
  * CHAI3D, GLFW, and OpenGL development packages

* **Compilation Steps**
  * Clone the repository:
    ```bash
    git clone <REPOSITORY_URL>
    cd tower_defense_cpp
    ```
  * Generate build files and compile:
    ```bash
    mkdir build && cd build
    cmake ..
    make
    ```
  * Launch the simulation:
    ```bash
    ./tower_defense_cpp
    ```

---

## 📁 Repository Structure

```text
├── assets/
│   └── new_models/
│       ├── turret_base.obj      # Turret mount chassis
│       ├── turret_barrel.obj    # Elevating cannon barrel
│       ├── missile.obj          # Projectile asset
│       ├── tank.obj             # Ground-based armored unit
│       ├── aircraft_1.obj       # Low-altitude interceptor
│       ├── aircraft_2.obj       # Mid-altitude strike craft
│       └── space_ship.obj       # High-altitude cruiser
│
├── externals/
│   ├── chai3d
│   └── glfw
├── include/
│   ├── Enemy.h                  # Enemy entity declaration & configurations
│   ├── Game.h                   # Core Game Loop, Camera, & Event dispatchers
│   ├── Projectile.h             # Ballistic kinematics & lifespan trackers
│   └── Tower.h                  # Joint kinematics and rotation logic
├── src/
│   ├── Enemy.cpp                # Waypoint-following and rotation matrices
│   ├── Game.cpp                 # Spatial collisions, spawning, & main loop
│   ├── main.cpp                 # System entry point
│   ├── Projectile.cpp           # Projectile displacement updates
│   └── Tower.cpp                # Mesh loading and transformation logic
└── CMakeLists.txt