#pragma once

#include "chai3d.h"
#include <GLFW/glfw3.h>
#include <memory>
#include "Tower.h"
#include "Projectile.h"
#include <vector>


class Game {
public:
    Game();
    ~Game();

    bool init();
    void run();
    void cleanup();

private:
    void processInput();
    void update(double dt);
    void render();

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void windowSizeCallback(GLFWwindow* window, int width, int height);

private:
    GLFWwindow* m_window;
    int m_windowWidth;
    int m_windowHeight;

    // Chai3D Core Components
    chai3d::cWorld* m_world;
    chai3d::cCamera* m_camera;
    chai3d::cDirectionalLight* m_light;

    // Game Objects
    chai3d::cMesh* m_ground; // <--- Thêm mặt sàn

    chai3d::cPrecisionClock m_clock;
    bool m_isRunning;

private:
    Tower* m_tower = nullptr;

private:
    std::vector<Projectile*> m_projectiles;
    double m_fireCooldown = 0.0;



private:
// Thêm các biến quỹ đạo Camera
double m_camRadius;      // Khoảng cách camera tới tâm (mặc định ~ 15.0)
double m_camAzimuth;     // Góc xoay ngang (mặc định 0.0)
double m_camElevation;   // Góc nâng cao (mặc định ~ 45 độ = 0.785 rad)

// Trạng thái chuột
double m_lastMouseX;
double m_lastMouseY;
bool   m_isDragging;

// Callbacks sự kiện chuột từ GLFW
static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// Cập nhật ma trận vị trí Camera
void updateCamera();
};