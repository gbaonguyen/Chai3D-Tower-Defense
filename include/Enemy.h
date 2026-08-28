#pragma once

#include "chai3d.h"
#include <vector>
#include <string>

class Enemy {
public:
    Enemy(chai3d::cWorld* world, const std::string& modelPath, const std::vector<chai3d::cVector3d>& waypoints,
          double speed, double scale, const chai3d::cMatrix3d& preRotation);
    ~Enemy();

    void update(double dt);
    
    // Check if the entity has successfully traversed (di chuyển qua) all waypoints
    bool hasReachedEnd() const { return m_reachedEnd; }
    void setShowFrame(bool show, double size); 

    chai3d::cMultiMesh* getMesh() const { return m_mesh; }
    double getCollisionRadius() const { return m_collisionRadius; }
    void kill() { m_reachedEnd = true; } // Mark the entity as "killed" or "removed"

private:
    chai3d::cWorld* m_world;
    chai3d::cMultiMesh* m_mesh;
    
    std::vector<chai3d::cVector3d> m_waypoints;
    int m_currentWaypointIndex;
    
    double m_speed;
    bool m_reachedEnd;

    double m_collisionRadius; // Radius for collision detection
};