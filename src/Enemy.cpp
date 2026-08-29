#include "Enemy.h"
#include <iostream>

Enemy::Enemy(chai3d::cWorld* world, const EnemyConfig& config, const std::vector<chai3d::cVector3d>& path)
    : m_world(world), m_config(config), m_path(path), m_currentWaypointIndex(0), m_mesh(nullptr) 
{
    m_mesh = new chai3d::cMultiMesh();
    if (!m_mesh->loadFromFile(m_config.modelPath)) {
        std::cerr << "[ERROR] Không thể load Enemy: " << m_config.modelPath << std::endl;
        return;
    }

    m_mesh->scale(m_config.scale);
    m_mesh->computeAllNormals();
    m_mesh->setUseMaterial(true);
    m_mesh->setShowBoundaryBox(true, true);
    m_mesh->setShowFrame(true);

    if (!m_path.empty()) {
        chai3d::cVector3d startPos = m_path[0];
        startPos.z(startPos.z() + m_config.zOffset); // Áp dụng độ cao (Offset)
        m_mesh->setLocalPos(startPos);
    }

    m_world->addChild(m_mesh);
}

Enemy::~Enemy() {
    if (m_world != nullptr && m_mesh != nullptr) {
        // Ép buộc thế giới 3D gỡ bỏ mô hình này
        m_world->removeChild(m_mesh); 
        delete m_mesh;
        m_mesh = nullptr;
    }
}

void Enemy::update(double dt) {
    if (hasReachedDestination() || !m_mesh) return;

    chai3d::cVector3d currentPos = m_mesh->getLocalPos();
    chai3d::cVector3d targetPos = m_path[m_currentWaypointIndex];
    targetPos.z(targetPos.z() + m_config.zOffset); // Giữ nguyên độ cao mong muốn

    chai3d::cVector3d dir = targetPos - currentPos;
    double distanceToTarget = dir.length();

    // Nếu đã đến gần Waypoint (Sai số < 1.0 unit), chuyển sang điểm tiếp theo
    if (distanceToTarget < 1.0) {
        m_currentWaypointIndex++;
        return; 
    }

    // Bình chuẩn hóa vector hướng (Normalize) và di chuyển
    dir.normalize();
    m_mesh->setLocalPos(currentPos + dir * m_config.speed * dt);

    // Tính toán góc xoay để đầu kẻ địch luôn hướng về Waypoint
    chai3d::cVector3d defaultForward(1.0, 0.0, 0.0); // Mặc định model hướng trục +Ox
    double angle = chai3d::cAngle(defaultForward, dir);
    chai3d::cVector3d axis = chai3d::cCross(defaultForward, dir);
    
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