#include "Tower.h"
#include <iostream>

// Member Initializer List
Tower::Tower(chai3d::cWorld* world)
    : m_world(world),
      m_baseMesh(nullptr),
      m_barrelMesh(nullptr),
      m_yaw(0.0),
      m_pitch(0.0) 
{
    m_baseMesh = new chai3d::cMultiMesh();
    if (m_world != nullptr) {
        m_world->addChild(m_baseMesh);
    }
}

Tower::~Tower() {
    if (m_world != nullptr && m_baseMesh != nullptr) {
        m_world->removeChild(m_baseMesh);
        delete m_baseMesh;
        m_baseMesh = nullptr;
        m_barrelMesh = nullptr;
    }
}

bool Tower::loadBase(const std::string& filePath) {
    if (!m_baseMesh) return false;

    bool success = m_baseMesh->loadFromFile(filePath);
    if (!success) {
        std::cerr << "[ERROR] Failed to load: " << filePath << std::endl;
        return false;
    }

    // Centering Alignment
    m_baseMesh->computeBoundaryBox(true);
    chai3d::cVector3d center = m_baseMesh->getBoundaryCenter();
    chai3d::cVector3d minBox = m_baseMesh->getBoundaryMin();
    chai3d::cVector3d offset(-center.x(), -center.y(), -minBox.z());
    
    for (unsigned int i = 0; i < m_baseMesh->getNumMeshes(); ++i) {
        chai3d::cMesh* subMesh = m_baseMesh->getMesh(i);
        if (subMesh) subMesh->offsetVertices(offset);
    }

    // Geometric Pre-processing
    m_baseMesh->setShowFrame(false); 
    m_baseMesh->setFrameSize(5.0); 
    m_baseMesh->computeBoundaryBox(true);
    m_baseMesh->computeAllNormals();
    m_baseMesh->setUseMaterial(true);

    return true;
}

bool Tower::loadBarrel(const std::string& filePath) {
    if (!m_baseMesh) return false;

    m_barrelMesh = new chai3d::cMultiMesh();
    if (!m_barrelMesh->loadFromFile(filePath)) {
        std::cerr << "[ERROR] Failed to load: " << filePath << std::endl;
        delete m_barrelMesh;
        m_barrelMesh = nullptr;
        return false;
    }

    // Pivot Point Adjustment
    m_barrelMesh->computeBoundaryBox(true);
    chai3d::cVector3d center = m_barrelMesh->getBoundaryCenter();
    
    // Correct the pitvot offset 
    chai3d::cVector3d offset(-center.x() + 1.5, -center.y(), -center.z());
    for (unsigned int i = 0; i < m_barrelMesh->getNumMeshes(); ++i) {
        chai3d::cMesh* subMesh = m_barrelMesh->getMesh(i);
        if (subMesh) {
            subMesh->offsetVertices(offset);
        }
    }

    // Paremt-Child Relationship
    m_baseMesh->addChild(m_barrelMesh);
    
    // Set the initial position of the barrel relative to the base
    m_barrelMesh->setLocalPos(2.0, 0.0, 1.2);

    // Geometric Pre-processing
    m_barrelMesh->setShowFrame(false); 
    m_barrelMesh->setFrameSize(5.0);
    m_barrelMesh->computeBoundaryBox(true);
    m_barrelMesh->computeAllNormals();
    m_barrelMesh->setUseMaterial(true);

    return true;
}

void Tower::setYaw(double angleRad) {
    m_yaw = angleRad;
    if (m_baseMesh) {
        chai3d::cMatrix3d rot;
        // Applies a rotation around the local Z-axis to turn the tower base
        rot.setAxisAngleRotationRad(chai3d::cVector3d(0, 0, 1), m_yaw);
        m_baseMesh->setLocalRot(rot);
    }
}

void Tower::setPitch(double angleRad) {
    // Fixed the pitch angle to be clamped between 0 and 60 degrees (in radians)
    m_pitch = chai3d::cClamp(angleRad, 0.0, M_PI / 3); 
    if (m_barrelMesh) {
        chai3d::cMatrix3d rot;
        // Applys a rotation around the local Y-axis to tilt the barrel up and down
        rot.setAxisAngleRotationRad(chai3d::cVector3d(0, -1, 0), m_pitch);
        m_barrelMesh->setLocalRot(rot);
    }
}
void Tower::setPosition(const chai3d::cVector3d& pos) {
    if (m_baseMesh) {
        m_baseMesh->setLocalPos(pos);
    }
}

void Tower::setScale(double scale) {
    if (m_baseMesh) {
        m_baseMesh->scale(scale);
    }
}
