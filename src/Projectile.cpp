#include "Projectile.h"
#include <iostream>

Projectile::Projectile(chai3d::cWorld* world, const std::string& modelPath, const chai3d::cVector3d& startPos, const chai3d::cVector3d& direction, double speed)
    : m_world(world), m_mesh(nullptr), m_lifeTime(0.0), m_maxLifeTime(3.0) 
{
    m_mesh = new chai3d::cMultiMesh();
    bool success = m_mesh->loadFromFile(modelPath);
    
    if (!success) {
        std::cerr << "[ERROR] Khong the load model dan tai: " << modelPath << std::endl;
    } else {
        std::cout << "[INFO] Da sinh ten lua tai toa do: " 
                  << startPos.x() << ", " << startPos.y() << ", " << startPos.z() << std::endl;
    }

    // 1. TĂNG SCALE: Thử số lớn hơn (ví dụ 1.0 hoặc 0.5) để dễ nhìn thấy trước
    m_mesh->scale(0.5); 

    // Tính Bounding Box và lấy bán kính cho đạn
    m_mesh->computeBoundaryBox(true);
    m_mesh->setShowBoundaryBox(true); // Hiển thị Bounding Box để kiểm tra
    m_collisionRadius = m_mesh->getBoundaryMax().length() * 0.5;

    // QUAN TRỌNG: Bật tính toán vật liệu để ánh sáng chiếu vào không bị đen thui
    m_mesh->computeAllNormals();
    m_mesh->setUseMaterial(true);

    // 2. Chuẩn hóa hướng bay
    chai3d::cVector3d normDir = direction;
    normDir.normalize();
    m_velocity = normDir * speed;

    // 3. Căn chỉnh hướng
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

    // 4. Đặt vị trí xuất phát
    m_mesh->setLocalPos(startPos);

    if (m_world) {
        m_world->addChild(m_mesh);
    }
}

Projectile::~Projectile() {
    // Tự động gỡ mesh ra khỏi thế giới / node cha khi đối tượng bị hủy
    if (m_mesh != nullptr) {
        if (m_mesh->getParent() != nullptr) {
            m_mesh->getParent()->removeChild(m_mesh);
        }
        delete m_mesh;
        m_mesh = nullptr;
    }
}

void Projectile::update(double dt) {
    m_lifeTime += dt;
    // Cập nhật quỹ đạo bay (trajectory update)
    chai3d::cVector3d currentPos = m_mesh->getLocalPos();
    m_mesh->setLocalPos(currentPos + m_velocity * dt);
}

bool Projectile::isExpired() const {
    return m_lifeTime >= m_maxLifeTime;
}