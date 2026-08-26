#pragma once
#include "chai3d.h"
#include <string>

class Projectile {
public:
    // Thêm tham số modelPath để truyền đường dẫn file Missile.obj
    Projectile(chai3d::cWorld* world, const std::string& modelPath, const chai3d::cVector3d& startPos, const chai3d::cVector3d& direction, double speed);
    ~Projectile();

    void update(double dt);
    bool isExpired() const;

private:
    chai3d::cWorld* m_world;
    chai3d::cMultiMesh* m_mesh;   // Mô hình 3D của tên lửa
    chai3d::cVector3d m_velocity; // Vận tốc bay (velocity)
    
    double m_lifeTime;
    double m_maxLifeTime;
};