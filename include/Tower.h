#pragma once

#include "chai3d.h"
#include <string>

class Tower {
public:
    Tower(chai3d::cWorld* world);
    ~Tower();

    bool loadBase(const std::string& filePath);
    
    // Removed joinHeightOffset parameter
    bool loadBarrel(const std::string& filePath);

    void setPosition(const chai3d::cVector3d& pos);
    void setScale(double scale);

    void setYaw(double angleRad); 
    void setPitch(double angleRad); 
    double getPitch() const { return m_pitch; }
    double getYaw() const { return m_yaw; }

    chai3d::cMultiMesh* getBaseMesh() const { return m_baseMesh; }
    chai3d::cMultiMesh* getBarrelMesh() const { return m_barrelMesh; }

private:
    chai3d::cWorld* m_world;
    chai3d::cMultiMesh* m_baseMesh;
    chai3d::cMultiMesh* m_barrelMesh;

    double m_yaw;
    double m_pitch;
};