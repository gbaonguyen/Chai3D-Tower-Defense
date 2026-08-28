#include "Enemy.h"
#include <iostream>

Enemy::Enemy(chai3d::cWorld* world, const std::string& modelPath, const std::vector<chai3d::cVector3d>& waypoints, double speed, double scale, const chai3d::cMatrix3d& preRotation)
    : m_world(world), 
      m_mesh(nullptr), 
      m_waypoints(waypoints), 
      m_currentWaypointIndex(0), 
      m_speed(speed), 
      m_reachedEnd(false) 
{
    m_mesh = new chai3d::cMultiMesh();
    if (!m_mesh->loadFromFile(modelPath)) return;

    m_mesh->scale(scale);

    // 1. Áp dụng tiền xử lý xoay (Vertex Pre-rotation) để chuẩn hóa hướng mũi model về trục +X
    for (unsigned int i = 0; i < m_mesh->getNumMeshes(); ++i) {
        chai3d::cMesh* subMesh = m_mesh->getMesh(i);
        if (subMesh != nullptr) {
            for (unsigned int v = 0; v < subMesh->getNumVertices(); ++v) {
                chai3d::cVector3d pos = subMesh->m_vertices->getLocalPos(v);
                subMesh->m_vertices->setLocalPos(v, preRotation * pos);
            }
        }
    }

    // 2. Tính toán Bounding Box và dời model lên mặt đất (như code ở bước trước)
    m_mesh->computeBoundaryBox(true);
    double minZ = m_mesh->getBoundaryMin().z();
    chai3d::cVector3d offset(0.0, 0.0, -minZ); 
    
    for (unsigned int i = 0; i < m_mesh->getNumMeshes(); ++i) {
        chai3d::cMesh* subMesh = m_mesh->getMesh(i);
        if (subMesh != nullptr) {
            subMesh->offsetVertices(offset);
        }
    }
    
    m_mesh->computeBoundaryBox(true); 
    m_mesh->computeAllNormals();
    m_mesh->setUseMaterial(true);

    // Initialize the starting coordinate (tọa độ xuất phát)
    if (!m_waypoints.empty()) {
        m_mesh->setLocalPos(m_waypoints[0]);
        m_currentWaypointIndex = 1; // Target the subsequent node
    } else {
        m_reachedEnd = true;
    }

    if (m_world) {
        m_world->addChild(m_mesh);
    }
}

Enemy::~Enemy() {
    if (m_mesh != nullptr) {
        if (m_mesh->getParent() != nullptr) {
            m_mesh->getParent()->removeChild(m_mesh);
        }
        delete m_mesh;
        m_mesh = nullptr;
    }
}

void Enemy::update(double dt) {
    if (m_reachedEnd || m_waypoints.empty()) return;

    chai3d::cVector3d currentPos = m_mesh->getLocalPos();
    chai3d::cVector3d targetPos = m_waypoints[m_currentWaypointIndex];

    chai3d::cVector3d direction = targetPos - currentPos;
    double distance = direction.length();

    // Proximity threshold (ngưỡng tiệm cận) to determine if a waypoint is reached
    if (distance < 0.1) {
        m_currentWaypointIndex++;
        if (m_currentWaypointIndex >= m_waypoints.size()) {
            m_reachedEnd = true;
        }
        return;
    }

    // Normalize trajectory (chuẩn hóa quỹ đạo) and calculate velocity vector
    direction.normalize();
    chai3d::cVector3d velocity = direction * m_speed;
    
    // Spatial interpolation (nội suy không gian) for movement
    m_mesh->setLocalPos(currentPos + velocity * dt);

    // Dynamic rotation to face the movement vector (hướng trục quay theo vector di chuyển)
    chai3d::cVector3d defaultDir(1.0, 0.0, 0.0); // Assuming the model's forward axis is +X
    double angle = chai3d::cAngle(defaultDir, direction);
    chai3d::cVector3d axis = chai3d::cCross(defaultDir, direction);
    
    if (axis.length() > 0.001) { 
        axis.normalize();
        chai3d::cMatrix3d rot;
        rot.setAxisAngleRotationRad(axis, angle);
        m_mesh->setLocalRot(rot);
    }
}

void Enemy::setShowFrame(bool show, double size) {
    if (m_mesh) {
        m_mesh->setShowFrame(show);
        m_mesh->setFrameSize(size, true);

    }
}