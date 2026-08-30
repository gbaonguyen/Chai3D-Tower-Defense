#pragma once
#include "chai3d.h"
#include <string>

/**
 * @brief Class representing a projectile that is fired and flies through the 3D space.
 */
class Projectile {
public:
    /**
     * @brief Initializes a projectile
     * 
     * @param world Pointer to the Chai3D world where the projectile will be instantiated.
     * @param modelPath File path to the 3D model file (Mesh asset).
     * @param startPos Initial global coordinates when the projectile is created.
     * @param direction Forward vector (normalized).
     * @param speed Linear velocity in units/second.
     */
    Projectile(chai3d::cWorld* world, const std::string& modelPath, const chai3d::cVector3d& startPos, const chai3d::cVector3d& direction, double speed);
    
    /**
     * @brief Destructor that safely removes the projectile's mesh from the world and frees memory.
     */
    ~Projectile();  

    /**
     * @brief Push the projectile forward based on its velocity.
     * 
     * @param dt Time step (delta time) in seconds since the last update.
     */
    void update(double dt);

    /**
     * @brief Check if the projectile has exceeded its maximum lifetime and should be removed from the world.
     * 
     * @return True if the projectile has expired, false otherwise.
     */
    bool isExpired() const;

    /// @brief Flag marking whether the projectile has been destroyed, used for garbage collection.
    bool m_isDead = false;

    /**
     * @brief Query the current global position of the projectile in the 3D world.
     * 
     * @return chai3d::cVector3d representing the projectile's global position.
     */
    chai3d::cVector3d getPosition() const { 
        return (m_mesh != nullptr) ? m_mesh->getLocalPos() : chai3d::cVector3d(0, 0, 0); 
    }

    /**
     * @brief Compute the bounding sphere radius for collision detection based on the mesh's bounding box.
     * 
     * @return A double -> representing the collision radius.
     */
    double getRadius() const {
        if (m_mesh != nullptr) {
            chai3d::cVector3d minBox = m_mesh->getBoundaryMin();
            chai3d::cVector3d maxBox = m_mesh->getBoundaryMax();
            chai3d::cVector3d diagonal = maxBox - minBox;
            return 0.4 * diagonal.length(); 
        }
        return 1.0; 
    }

private:
    /// @brief Pointer to the Chai3D world where the projectile exists.
    chai3d::cWorld* m_world;

    /// @brief Pointer to the multi-mesh representing the projectile's 3D model. 
    chai3d::cMultiMesh* m_mesh;   

    /// @brief Velocity vector representing the projectile's linear motion in 3D space.
    chai3d::cVector3d m_velocity; 
    
    /// @brief Accumulated lifetime since instantiation.
    double m_lifeTime;

    /// @brief Maximum allowed lifetime before automatic expiration.
    double m_maxLifeTime;
};