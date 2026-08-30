#include "Projectile.h"
#include <iostream>

// Member Initializer List
Projectile::Projectile(chai3d::cWorld* world, const std::string& modelPath,
    const chai3d::cVector3d& startPos, const chai3d::cVector3d& direction, double speed)
    : m_world(world),
      m_mesh(nullptr),
      m_lifeTime(0.0),
      m_maxLifeTime(3.0) 
{
    m_mesh = new chai3d::cMultiMesh();
    if (!m_mesh->loadFromFile(modelPath)) {
        std::cerr << "[ERROR] Failed to load:" << modelPath << std::endl;
    }

    // Geometric Pre-processing
    m_mesh->computeAllNormals();
    m_mesh->setUseMaterial(true);
    m_mesh->setShowBoundaryBox(false, false);

    // Normalize the direction vector to ensure consistent speed
    chai3d::cVector3d normDir = direction;
    normDir.normalize();
    
    // Compute the velocity vector
    m_velocity = normDir * speed;

    // Orientation Alignment
    chai3d::cVector3d defaultDir(1.0, 0.0, 0.0);
    double angle = chai3d::cAngle(defaultDir, normDir);
    chai3d::cVector3d axis = chai3d::cCross(defaultDir, normDir);

    if (axis.length() < 0.001) { 
        axis = (defaultDir.dot(normDir) < 0) ? chai3d::cVector3d(0, 1, 0) : chai3d::cVector3d(1, 0, 0);
    } else {
        axis.normalize();
    }

    chai3d::cMatrix3d rot;
    rot.setAxisAngleRotationRad(axis, angle);
    m_mesh->setLocalRot(rot);

    // Set the initial position of the projectile and add it to the world
    m_mesh->setLocalPos(startPos);
    if (m_world) {
        m_world->addChild(m_mesh);
    }
}

Projectile::~Projectile() {
    if (m_world != nullptr && m_mesh != nullptr) {
        m_world->removeChild(m_mesh);
        delete m_mesh;
        m_mesh = nullptr;
    }
}

void Projectile::update(double dt) {
    m_lifeTime += dt;
    chai3d::cVector3d currentPos = m_mesh->getLocalPos();
    m_mesh->setLocalPos(currentPos + m_velocity * dt);
}

bool Projectile::isExpired() const {
    return m_lifeTime >= m_maxLifeTime;
}