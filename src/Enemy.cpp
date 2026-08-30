#include "Enemy.h"
#include <iostream>

// Member Initializer List
Enemy::Enemy(chai3d::cWorld* world, const EnemyConfig& config,
            const std::vector<chai3d::cVector3d>& path)
    : m_world(world),
      m_config(config),
      m_path(path),
      m_currentWaypointIndex(0),
      m_mesh(nullptr) 
{
    m_mesh = new chai3d::cMultiMesh();
    if (!m_mesh->loadFromFile(m_config.modelPath)) {
        std::cerr << "[ERROR] Failed to load: " << m_config.modelPath << std::endl;
        return;
    }

    // Geometric Pre-processing
    m_mesh->scale(m_config.scale);
    m_mesh->computeAllNormals();
    m_mesh->setUseMaterial(true);
    m_mesh->setShowBoundaryBox(false, false);
    m_mesh->setShowFrame(false); 

    if (!m_path.empty()) {
        chai3d::cVector3d startPos = m_path[0];
        // Apply Z-offset to the starting position for each enemy type
        startPos.z(startPos.z() + m_config.zOffset); 
        m_mesh->setLocalPos(startPos);
    }

    m_world->addChild(m_mesh);
}

Enemy::~Enemy() {
    if (m_world != nullptr && m_mesh != nullptr) {
        m_world->removeChild(m_mesh); 
        delete m_mesh;
        m_mesh = nullptr;
    }
}

void Enemy::update(double dt) {
    // 
    if (hasReachedDestination() || !m_mesh) return;

    chai3d::cVector3d currentPos = m_mesh->getLocalPos();
    chai3d::cVector3d targetPos = m_path[m_currentWaypointIndex];
    targetPos.z(targetPos.z() + m_config.zOffset); 

    // Compute the direction vector and distance to the next waypoint
    chai3d::cVector3d dir = targetPos - currentPos;
    double distanceToTarget = dir.length();

    // If the enemy is close enough to the target waypoint, move to the next waypoint
    if (distanceToTarget < 1.0) {
        m_currentWaypointIndex++;
        return; 
    }

    dir.normalize();
    // Update the position based on linear velocity and delta time
    m_mesh->setLocalPos(currentPos + dir * m_config.speed * dt);

    // Calculate the rotational kinematics
    chai3d::cVector3d defaultForward(1.0, 0.0, 0.0); 
    
    // Find the angle between the default forward vector and the current direction
    double angle = chai3d::cAngle(defaultForward, dir);
    
    // Find the axis of rotation using the cross product
    chai3d::cVector3d axis = chai3d::cCross(defaultForward, dir);
    
    // Handle the degenerate case when the two vectors are parallel or antiparallel
    if (axis.length() < 0.001) {
        axis = (defaultForward.dot(dir) < 0) ? chai3d::cVector3d(0, 0, 1) : chai3d::cVector3d(1, 0, 0);
    } else {
        axis.normalize();
    }

    chai3d::cMatrix3d rot;
    rot.setAxisAngleRotationRad(axis, angle);
    m_mesh->setLocalRot(rot);
}

bool Enemy::hasReachedDestination() const {
    return m_currentWaypointIndex >= m_path.size();
}