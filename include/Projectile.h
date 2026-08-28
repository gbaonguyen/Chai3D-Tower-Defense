#pragma once
#include "chai3d.h"
#include <string>

class Projectile {
public:
    Projectile(chai3d::cWorld* world, const std::string& modelPath, const chai3d::cVector3d& startPos, const chai3d::cVector3d& direction, double speed);
    ~Projectile();  

    void update(double dt);
    bool isExpired() const;

    // Trích xuất tọa độ cục bộ (Local Position) hiện tại của viên đạn
    chai3d::cVector3d getPosition() const { 
        return (m_mesh != nullptr) ? m_mesh->getLocalPos() : chai3d::cVector3d(0, 0, 0); 
    }

private:
    chai3d::cWorld* m_world;
    chai3d::cMultiMesh* m_mesh;   
    chai3d::cVector3d m_velocity; 
    
    double m_lifeTime;
    double m_maxLifeTime;
};