# Technical Report: 3D Tower Defense Simulation

## 1. Project Overview
The primary objective of this project is to engineer a 3D tower defense simulation utilizing **C++**, the **CHAI3D** framework, and **GLFW/OpenGL**.

The core features of the gameplay encompass:
* **2-DOF Turret Articulation:** Dual-degree-of-freedom rotation for the turret.
* **Missile Mechanics:** Real-time missile launching capabilities.
* **Navigation:** Hostile entities navigating predefined paths via waypoints.
* **Collision Detection:** Accurate spatial intersection algorithms to register hits.

---

## 2. System Architecture & OOP Design
The system adopts a robust Object-Oriented Programming (OOP) paradigm, modeling interactions between distinct entity classes:

* **`Game`**: The central orchestrator, governing the Update-Render loop and executing memory garbage collection.
* **`Tower`**: A composite class managing the parent-child structural relationship of the turret.
* **`Projectile`**: Encapsulates the kinematic properties of the flying missiles.
* **`Enemy`**: Represents autonomous hostiles utilizing procedural movement algorithms.

### Scenegraph Hierarchy
A critical architectural decision is the implementation of a hierarchical Scenegraph. For example, the barrel mesh (`m_barrelMesh`) is instantiated as a **child node** of the base chassis (`m_baseMesh`). Consequently, when the base rotates horizontally, the barrel automatically inherits the Yaw angle via the transformation matrix, eliminating the need for redundant trigonometric calculations.

---

## 3. Mathematical Foundations & Algorithms

### Geometric Centering & Pivot Point Offsetting
Raw 3D mesh assets typically possess arbitrary local origins or centroid-aligned bounding boxes. Directly rotating or placing these models leads to unnatural pivoting and ground clipping. The system resolves this through two dedicated vertex-offset transformations:
* **Base Ground Alignment:** For `m_baseMesh`, the system computes the geometric center and minimum boundary point ($Z_{\min}$), applying an offset vector $\vec{O}_{\text{base}} = (-X_{\text{center}}, -Y_{\text{center}}, -Z_{\min})$ across all submesh vertices. This aligns the turret's bottom surface flush with the ground plane ($Z = 0$) and centers its rotational axis at $(0, 0)$.
* **Barrel Pivot Realignment:** Rotating the cannon barrel around its geometric center causes the midsection to clip awkwardly into the turret housing. The system shifts the local origin backward to the breech/hinge joint using $\vec{O}_{\text{barrel}} = (-X_{\text{center}} + 1.5, -Y_{\text{center}}, -Z_{\text{center}})$, establishing an anatomically correct pivot point for Pitch elevation.

### Rotational Kinematics
To ensure enemies and projectiles face their traversal trajectories, the system calculates orientation using 3D vector mathematics:
* **Dot Product:** Utilized to compute the angular displacement $\theta$ between the canonical forward vector and the target directional vector.
* **Cross Product:** Employed to derive the orthogonal rotation axis $\vec{axis}$.

### Singularity Handling
When an entity needs to perform a strict $180^\circ$ inversion, the Cross Product yields a magnitude of $\approx 0$ (a mathematical singularity). The algorithm proactively circumvents this by assigning a default vertical axis $(0, 0, 1)$, ensuring uninterrupted matrix generation.

### Bounding Sphere Collision
Intersection logic relies on rapid scalar distance evaluation between two geometric centers:
$$|\vec{P}_{\text{bullet}} - \vec{P}_{\text{enemy}}| < R_1 + R_2$$
This methodology provides a highly performant alternative to complex polygon intersection tests.

### Muzzle Vector & Spawn Offset Extraction
To determine the exact firing trajectory, the global rotation matrix of the barrel is multiplied by the default $+Ox$ vector $(1, 0, 0)$, extracting the unit **forward vector** $\vec{f}$. To prevent newly instantiated projectiles from immediately colliding with or visually intersecting the internal geometry of the cannon, the spawn point is offset forward along the muzzle direction:
$$\vec{P}_{\text{spawn}} = \vec{P}_{\text{barrel\_global}} + \vec{f} \cdot L_{\text{offset}}$$
where $L_{\text{offset}} = 2.0\text{ units}$.

---

## 4. Memory Lifecycle Management

### Mark-and-Sweep Garbage Collection
The `update()` cycle implements a sophisticated Mark-and-Sweep protocol.
* **Mark Phase:** During collision detection, interacting entities are merely flagged (`m_isDead = true`).
* **Sweep Phase:** A secondary loop iterates through the containers to `erase` and `delete` flagged pointers.
This decoupling strictly prevents **Iterator Invalidation** which would otherwise induce fatal runtime crashes.

### Resource Reclamation
Dynamic heap allocations are rigorously managed. The `cleanup()` function and class **Destructors** systematically deallocate meshes and detach them from the Chai3D `cWorld` to prevent memory leaks.

---

## 5. Future Improvements
Future iterations of this simulation could integrate several advanced paradigms:
* **Health Point (HP) Mechanics:** Implementing localized damage rather than immediate destruction.
* **Particle Systems:** Rendering dynamic volumetric explosions upon impact.
* **Turret Variety:** Multiple turret types with distinct ballistic profiles.
* **Spatial Partitioning:** Optimizing the $O(N \times M)$ collision detection algorithm by implementing **BVH (Bounding Volume Hierarchies)** or **Octrees** to efficiently cull distant entities.