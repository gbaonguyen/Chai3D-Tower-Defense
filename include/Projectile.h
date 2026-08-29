#pragma once
#include "chai3d.h"
#include <string>

class Projectile {
public:
    Projectile(chai3d::cWorld* world, const std::string& modelPath, const chai3d::cVector3d& startPos, const chai3d::cVector3d& direction, double speed);
    ~Projectile();  

    void update(double dt);
    bool isExpired() const;

    bool m_isDead = false;

    // Lấy tọa độ toàn cục để tính toán chính xác
    chai3d::cVector3d getPosition() const { 
        return (m_mesh != nullptr) ? m_mesh->getLocalPos() : chai3d::cVector3d(0, 0, 0); 
    }

    // Lấy bán kính Bounding Sphere (Khối cầu bao quanh)
    double getRadius() const {
        if (m_mesh != nullptr) {
            // Extract the spatial extremities of the mesh (Trích xuất các điểm cực đại không gian của lưới)
            chai3d::cVector3d minBox = m_mesh->getBoundaryMin();
            chai3d::cVector3d maxBox = m_mesh->getBoundaryMax();
            
            // The radius corresponds to half the magnitude of the box's diagonal 
            // (Bán kính tương đương một nửa độ lớn đường chéo của hộp)
            chai3d::cVector3d diagonal = maxBox - minBox;
            return 0.4 * diagonal.length(); 
        }
        return 1.0; // Fallback default value (Giá trị mặc định dự phòng)
    }

private:
    chai3d::cWorld* m_world;
    chai3d::cMultiMesh* m_mesh;   
    chai3d::cVector3d m_velocity; 
    
    double m_lifeTime;
    double m_maxLifeTime;
};