#pragma once

#include "chai3d.h"
#include <string>
#include <vector>

/**
 * @brief Attributes for each Enemy type
 */
struct EnemyConfig {
    /// @brief File path to the 3D model
    std::string modelPath;

    /// @brief Scale factor to adjust the size of the model in the 3D world
    double scale;

    /// @brief Z-offset to determine the flying height.
    double zOffset;

    /// @brief Linear velocity for movement, measured in units per second.
    double speed;
};

/**
 * @brief Class representing an Enemy entity that navigates through a series of waypoints in the 3D world.
 */
class Enemy {
public:
    /**
     * @brief Constructor that initializes an Enemy with a specific configuration and a path of waypoints.
     * @param world Pointer to the Chai3D world where the Enemy will be instantiated.
     * @param config Attributes include model path, scale, z-offset, and speed.
     * @param path Lists of waypoints that the Enemy will follow.
     */
    Enemy(chai3d::cWorld* world, const EnemyConfig& config, const std::vector<chai3d::cVector3d>& path);

    /**
     * @brief Destructor that safely removes the Enemy's mesh from the world and frees memory.
     */
    ~Enemy();

    /**
     * @brief Updates the Enemy's position along its path based on its speed and the elapsed time.
     * 
     * @param dt Delta time in seconds since the last update call.
     */
    void update(double dt);

    /**
     * @brief Check if the Enemy has reached its final waypoint in the path.
     * 
     * @return True if yes, false otherwise.
     */
    bool hasReachedDestination() const;

    /**
     * @brief Query the current global position of the Enemy in the 3D world.
     * 
     * @return chai3d::cVector3d representing the Enemy's global position.
     */
    chai3d::cVector3d getPosition() const { 
        return (m_mesh != nullptr) ? m_mesh->getLocalPos() : chai3d::cVector3d(0, 0, 0); 
    }

    /**
     * @brief Compute the bounding sphere radius for collision detection based on the mesh's bounding box.
     * 
     * @return a double representing the collision radius.
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

    /// @brief Flag marking whether the Enemy has been destroyed, used for garbage collection.
    bool m_isDead = false;

private:
    /// @brief Pointer to the Chai3D world where the Enemy exists.
    chai3d::cWorld* m_world;

    /// @brief Pointer to the multi-mesh representing the Enemy's 3D model.
    chai3d::cMultiMesh* m_mesh;
    
    /// @brief Configuration profile stored.
    EnemyConfig m_config;

    /// @brief Queue of waypoints for navigation.
    std::vector<chai3d::cVector3d> m_path;

    /// @brief Index of the current target waypoint.
    size_t m_currentWaypointIndex;
};