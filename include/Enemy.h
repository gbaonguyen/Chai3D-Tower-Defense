#pragma once
#include "chai3d.h"
#include <string>
#include <vector>

// Cấu trúc lưu trữ thông số riêng biệt cho từng loại Kẻ địch
struct EnemyConfig {
    std::string modelPath;
    double scale;
    double zOffset; // Độ cao: 0 cho Tank, >0 cho Aircraft
    double speed;
};

class Enemy {
public:
    Enemy(chai3d::cWorld* world, const EnemyConfig& config, const std::vector<chai3d::cVector3d>& path);
    ~Enemy();

    void update(double dt);
    bool hasReachedDestination() const;

private:
    chai3d::cWorld* m_world;
    chai3d::cMultiMesh* m_mesh;
    
    EnemyConfig m_config;
    std::vector<chai3d::cVector3d> m_path;
    size_t m_currentWaypointIndex;
};