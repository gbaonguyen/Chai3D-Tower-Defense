#pragma once

#include "chai3d.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <vector>
#include <cstdlib>
#include <ctime>

#include "Tower.h"
#include "Projectile.h" 
#include "Enemy.h"

/**
 * @brief Centeral class that manages the main game loop, rendering, and user input for the Tower Defense game.
 */
class Game {
public:
    /**
     * @brief Initializes the game state, including the 3D world, camera, lighting, and player-controlled tower.
     */
    Game();

    /**
     * @brief Destructor that cleans up all dynamically allocated resources and terminates GLFW.
     */
    ~Game();

    /**
     * @brief Initializes the GLFW window, Chai3D world, camera, lighting, and other game components.
     * 
     * @return True if initialization is successful, false otherwise.
     */
    bool init();

    /**
     * @brief Starts the main game loop, which handles input, updates game state, and renders frames until the window is closed.
     */
    void run();

    /**
     * @brief Releases all allocated resources
     */
    void cleanup();

private:
    /**
     * @brief Polls and processes user input events, including keyboard and mouse interactions.
     */
    void processInput();

    /**
     * @brief Updates the simulation physics, detects collisions, and manages the lifecycle of game entities.
     * 
     * @param dt Time step (delta time) in seconds since the last frame.
     */
    void update(double dt);

    /**
     * @brief Renders the current frame by issuing draw calls to the Chai3D camera.
     */
    void render();

    /**
     * @brief Callback of GLFW used to capture changes in the keyboard's physical state.
     */
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    /**
     * @brief Callback of GLFW used to capture changes in the window size and adjust the viewport accordingly.
     */
    static void windowSizeCallback(GLFWwindow* window, int width, int height);

    /**
     * @brief Initializes a new enemy entity with randomized attributes.
     */
    void spawnRandomEnemy();

private:
    /// @brief pointer to the GLFW window context used for rendering and input handling.
    GLFWwindow* m_window;
    
    /// @brief Width of the viewport.
    int m_windowWidth;

    /// @brief Height of the viewport.
    int m_windowHeight;

    /// @brief root node of the Chai3D scene graph representing the 3D world.
    chai3d::cWorld* m_world;

    /// @brief perspective renderer of the camera.
    chai3d::cCamera* m_camera;

    /// @brief main directional light source.
    chai3d::cDirectionalLight* m_light;

    /// @brief Ground Mesh
    chai3d::cMesh* m_ground;

    /// @brief High precision clock used to measure delta time between frames for consistent simulation updates.
    chai3d::cPrecisionClock m_clock;

    /// @brief Flag indicating whether the game loop is currently running. When set to false, the main loop will exit and the game will terminate.
    bool m_isRunning;

private:
    /// @brief Pointer to the player-controlled tower object, which can rotate and fire projectiles.
    Tower* m_tower = nullptr;

    /// @brief dynamic array that tracks all active projectiles in the game world.
    std::vector<Projectile*> m_projectiles;

    /// @brief High precision timer used to control the rate of fire.
    double m_fireCooldown = 0.0;

    /// @brief Dynamic array tracking all active enemy entities.
    std::vector<Enemy*> m_enemies;

    /// @brief High precision timer used to track the time until the next enemy spawn.
    double m_enemySpawnTimer = 0.0;

    double m_enemySpawnInterval = 2.0;


// Not related to the game logic, but used for camera control to debug and visualize
private:
    /// @brief Tọa độ cầu (spherical coordinate) đại diện cho khoảng cách từ Camera tới tâm.
    double m_camRadius;

    /// @brief Tọa độ cầu đại diện cho góc xoay ngang của Camera.
    double m_camAzimuth;

    /// @brief Tọa độ cầu đại diện cho góc nâng dọc của Camera.
    double m_camElevation;

    /// @brief Tọa độ X của con trỏ (cursor) được lưu trữ để tính toán thao tác kéo chuột (drag).
    double m_lastMouseX;

    /// @brief Tọa độ Y của con trỏ được lưu trữ để tính toán thao tác kéo chuột.
    double m_lastMouseY;

    /// @brief Cờ biểu thị trạng thái đang giữ và kéo chuột.
    bool m_isDragging;

    /**
     * @brief Callback của GLFW xử lý các thao tác nhấn nút chuột.
     */
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    /**
     * @brief Callback của GLFW xử lý các vector chuyển động của con trỏ chuột.
     */
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

    /**
     * @brief Callback của GLFW xử lý thao tác cuộn bánh xe chuột để thu phóng (zoom) Camera.
     */
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    /**
     * @brief Tính toán lại và áp dụng tọa độ Descartes (Cartesian) cho Camera từ các giá trị tọa độ cầu.
     */
    void updateCamera();
};