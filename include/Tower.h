#pragma once

#include "chai3d.h"
#include <string>

/**
 * @brief Class representing a defensive tower controlled by the player.
 */
class Tower {
public:
    /**
     * @brief Constructor that initializes the tower within the simulation environment.
     * 
     * @param world Pointer to the Chai3D world where the tower will be instantiated.
     */
    Tower(chai3d::cWorld* world);
    
    /**
     * @brief Destructor that safely removes the tower's meshes from the world and frees memory.
     */
    ~Tower();

    /**
     * @brief Loads and structures the static base of the tower.
     * @param filePath Path to the 3D model file for the base.
     * @return true if loading and initialization are successful, false otherwise.
     */
    bool loadBase(const std::string& filePath);
    
    /**
     * @brief Loads the movable barrel (barrel) and attaches it to the base.
     * 
     * @param filePath Path to the 3D model file for the barrel.
     * @return true if loading and attachment are successful, false otherwise.
     */
    bool loadBarrel(const std::string& filePath);

    /**
     * @brief Sets the global position of the tower in the 3D world.
     * 
     * @param pos Target global coordinates (3D position).
     */
    void setPosition(const chai3d::cVector3d& pos);

    /**
     * @brief Applies a uniform scale transformation to the entire tower.
     * 
     * @param scale The uniform scaling factor.
     */
    void setScale(double scale);

    /**
     * @brief Applies a horizontal rotation (Yaw) transformation to the tower's base.
     * 
     * @param angleRad Target yaw angle in radians.
     */
    void setYaw(double angleRad); 

    /**
     * @brief Applies a vertical rotation (Pitch) transformation to the tower's barrel.
     * 
     * @param angleRad Target pitch angle in radians.
     */
    void setPitch(double angleRad); 

    /**
     * @brief Get the current vertical rotation (pitch) angle of the barrel.
     * 
     * @return double Current pitch angle in radians.
     */
    double getPitch() const { return m_pitch; }

    /**
     * @brief Get the current horizontal rotation (yaw) angle of the tower's base.
     * 
     * @return double Current yaw angle in radians.
     */
    double getYaw() const { return m_yaw; }

    /**
     * @brief Provides access to the base mesh of the tower
     * 
     * @return Pointer to the multi-mesh representing the tower's base.
     */
    chai3d::cMultiMesh* getBaseMesh() const { return m_baseMesh; }

    /**
     * @brief Provides access to the barrel mesh of the tower
     * 
     * @return Pointer to the multi-mesh representing the tower's barrel.
     */
    chai3d::cMultiMesh* getBarrelMesh() const { return m_barrelMesh; }

private:
    /// @brief pointer to the Chai3D world where the tower exists.
    chai3d::cWorld* m_world;

    /// @brief Mesh 3D representing the static base of the tower.
    chai3d::cMultiMesh* m_baseMesh;

    /// @brief Mesh 3D representing the movable barrel of the tower.
    chai3d::cMultiMesh* m_barrelMesh;

    /// @brief Accumulated horizontal rotation (yaw) angle in radians.
    double m_yaw;

    /// @brief Accumulated vertical rotation (pitch) angle in radians.
    double m_pitch;
};