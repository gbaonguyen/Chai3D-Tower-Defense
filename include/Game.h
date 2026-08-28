#pragma once

#include "chai3d.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <vector>
#include <cstdlib> // Thêm thư viện hỗ trợ rand()
#include <ctime>   // Thêm thư viện hỗ trợ time()

#include "Tower.h"
#include "Projectile.h" // Thêm include này
#include "Enemy.h"

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

    void spawnRandomEnemy();

private:
    GLFWwindow* m_window;
    int m_windowWidth;
    int m_windowHeight;

    chai3d::cWorld* m_world;
    chai3d::cCamera* m_camera;
    chai3d::cDirectionalLight* m_light;
    chai3d::cMesh* m_ground;

    chai3d::cPrecisionClock m_clock;
    bool m_isRunning;

private:
    Tower* m_tower = nullptr;

    std::vector<Projectile*> m_projectiles;
    double m_fireCooldown = 0.0;

    std::vector<Enemy*> m_enemies;
    double m_enemySpawnTimer = 0.0;
    double m_enemySpawnInterval = 2.0; // Spawn every 1 second



private:
    double m_camRadius;
    double m_camAzimuth;
    double m_camElevation;

    double m_lastMouseX;
    double m_lastMouseY;
    bool   m_isDragging;

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    void updateCamera();
};