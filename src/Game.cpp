#include "Game.h"
#include <iostream>

// Member Initializer List
Game::Game() 
    : m_window(nullptr),
      m_windowWidth(1280),
      m_windowHeight(720),
      m_world(nullptr),
      m_camera(nullptr),
      m_light(nullptr),
      m_ground(nullptr),
      m_clock(),
      m_tower(nullptr),
      m_isRunning(false),
      m_camRadius(16.0),
      m_camAzimuth(0.0),
      m_camElevation(0.8), 
      m_lastMouseX(0.0),
      m_lastMouseY(0.0),
      m_isDragging(false) {}

Game::~Game() {
    cleanup();
}

bool Game::init() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return false;
    }

    m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, "Tower Defense 3D - Chai3D Engine", nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(m_window, this);
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetWindowSizeCallback(m_window, windowSizeCallback);

    // glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    // glfwSetCursorPosCallback(m_window, cursorPosCallback);
    // glfwSetScrollCallback(m_window, scrollCallback);

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    m_world = new chai3d::cWorld();
    m_world->m_backgroundColor.set(0.1f, 0.1f, 0.15f);

    // camera setup
    m_camera = new chai3d::cCamera(m_world);
    m_world->addChild(m_camera);
    m_camera->set(
        chai3d::cVector3d(-25.0, 0.0, 5.0),
        chai3d::cVector3d(0.0, 0.0, 3.0),
        chai3d::cVector3d(0.0, 0.0, 1.0)
    );
    m_camera->setClippingPlanes(0.1, 100.0);

    m_light = new chai3d::cDirectionalLight(m_world);
    m_world->addChild(m_light);
    m_light->setEnabled(true);
    m_light->setDir(-0.5, 0.5, -1.0);

    // Ground setup
    m_ground = new chai3d::cMesh();
    m_world->addChild(m_ground);
    chai3d::cCreateBox(m_ground, 16.0, 16.0, 0.1);
    m_ground->setLocalPos(0.0, 0.0, -0.1);
    m_ground->m_material->setGrayDark();
    m_ground->m_material->m_ambient.set(0.2f, 0.2f, 0.2f);
    m_ground->m_material->m_diffuse.set(0.4f, 0.4f, 0.4f);
    m_ground->m_material->m_specular.set(0.1f, 0.1f, 0.1f);
    m_ground->setUseMaterial(true);
    m_ground->setShowFrame(false); 
    m_ground->setFrameSize(5.0);

    // Load Tower
    m_tower = new Tower(m_world);
    
    std::string basePath = "../assets/new_models/turret_base.obj";
    std::string barrelPath = "../assets/new_models/turret_barrel.obj";

    if (m_tower->loadBase(basePath)) {  
        if (m_tower->loadBarrel(barrelPath)) {
            m_tower->setPosition(chai3d::cVector3d(0.0, 0.0, 0.0));
        }
    }

    srand(static_cast<unsigned int>(time(nullptr)));

    m_isRunning = true;
    return true;
}

void Game::run() {
    m_clock.reset();
    m_clock.start();
    
    double lastTime = m_clock.getCurrentTimeSeconds();
    int frameCount = 0;

    while (!glfwWindowShouldClose(m_window) && m_isRunning) {
        // Trích xuất delta time để đảm bảo tốc độ mô phỏng độc lập với FPS phần cứng
        double dt = m_clock.getCurrentTimeSeconds();
        m_clock.reset();
        m_clock.start();

        processInput();
        update(dt);
        render();

        glfwPollEvents();

        // update FPS counter in window title every second
        frameCount++;
        double currentTime = glfwGetTime();
        if (currentTime - lastTime >= 1.0) {
            std::string title = "Tower Defense 3D - FPS: " + std::to_string(frameCount);
            glfwSetWindowTitle(m_window, title.c_str());
            frameCount = 0;
            lastTime = currentTime;
        }
    }
}

void Game::processInput() {
    if (!m_tower) return;

    // rotate the tower keyboard input
    static double currentYaw = 0.0;
    static double currentPitch = 0.0;
    const double rotationSpeed = 0.03; 

    if (glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        currentYaw += rotationSpeed;
        m_tower->setYaw(currentYaw);
    }
    if (glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        currentYaw -= rotationSpeed;
        m_tower->setYaw(currentYaw);
    }
    if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS) {
        currentPitch += rotationSpeed;
        m_tower->setPitch(currentPitch);
    }
    if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        currentPitch -= rotationSpeed;
        m_tower->setPitch(currentPitch);
    }
    
    // Fire projectile on spacebar press with cooldown
    if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (m_fireCooldown <= 0.0) {
            // Force the world to update global positions before spawning a projectile
            m_world->computeGlobalPositions(true);

            chai3d::cMultiMesh* barrel = m_tower->getBarrelMesh();
            if (barrel) {
                chai3d::cMatrix3d globalRot = barrel->getGlobalRot();
                chai3d::cVector3d globalPos = barrel->getGlobalPos();
                
                // Calculate the forward direction of the barrel in world coordinates
                chai3d::cVector3d forward = globalRot * chai3d::cVector3d(1.0, 0.0, 0.0);
                forward.normalize();
                
                // Push the spawn position forward by 2.0 units to prevent the projectile from colliding with the tower itself
                double barrelLengthOffset = 2.0; 
                chai3d::cVector3d spawnPos = globalPos + forward * barrelLengthOffset; 
                
                std::string missilePath = "../assets/new_models/missile.obj";
                Projectile* p = new Projectile(m_world, missilePath, spawnPos, forward, 20.0);
                m_projectiles.push_back(p);
                
                m_fireCooldown = 0.2; 
            }
        }
    }
}

void Game::update(double dt) {
    if (m_fireCooldown > 0.0) {
        m_fireCooldown -= dt;
    }
    
    m_enemySpawnTimer -= dt;
    if (m_enemySpawnTimer <= 0.0) {
        spawnRandomEnemy();
        m_enemySpawnTimer = m_enemySpawnInterval; 
    }

    const double MAP_RADIUS = 150.0; 

    for (auto it = m_projectiles.begin(); it != m_projectiles.end(); ) {
        Projectile* p = *it;
        if (p == nullptr) {
            it = m_projectiles.erase(it);
            continue;
        }

        p->update(dt);
        
        // Check for out-of-bounds projectiles using distance from origin
        double distanceFromOrigin = p->getPosition().length();
        bool isOutOfBounds = distanceFromOrigin > MAP_RADIUS;
        
        if (p->isExpired() || isOutOfBounds) {
            delete p;                      
            it = m_projectiles.erase(it);  
        } else {
            ++it;
        }
    }

    for (auto it = m_enemies.begin(); it != m_enemies.end(); ) {
        Enemy* e = *it;
        if (e == nullptr) {
            it = m_enemies.erase(it);
            continue;
        }

        e->update(dt);
        if (e->hasReachedDestination()) {
            delete e;
            it = m_enemies.erase(it);
        } else {
            ++it;
        }
    }

    // Thuật toán Bounding Sphere Collision độ phức tạp O(N*M)
    for (auto p : m_projectiles) {
        if (p->m_isDead) continue; 

        for (auto e : m_enemies) {
            if (e->m_isDead) continue; 

            chai3d::cVector3d diff = p->getPosition() - e->getPosition();
            double distance = diff.length();
            double sumRadius = p->getRadius() + e->getRadius();

            if (distance < sumRadius) {
                p->m_isDead = true; 
                e->m_isDead = true; 
                std::cout << "[COLLISION] Muc tieu bi tieu diet tai X: " << e->getPosition().x() << std::endl;
                break; 
            }
        }
    }

    // Chu trình Mark-and-Sweep Garbage Collection dọn dẹp các con trỏ đã bị đánh cờ
    for (auto it = m_projectiles.begin(); it != m_projectiles.end(); ) {
        if ((*it)->m_isDead || (*it)->isExpired()) {
            delete *it;
            it = m_projectiles.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = m_enemies.begin(); it != m_enemies.end(); ) {
        if ((*it)->m_isDead || (*it)->hasReachedDestination()) {
            delete *it;
            it = m_enemies.erase(it);
        } else {
            ++it;
        }
    }
}

void Game::spawnRandomEnemy() {
    int enemyType = rand() % 4; 

    EnemyConfig config;
    std::vector<chai3d::cVector3d> path;

    double startX = 70.0;
    double endX = 3.0;

    switch (enemyType) {
        case 0:
            path = { chai3d::cVector3d(startX, 0.0, 0.0), chai3d::cVector3d(endX, 0.0, 0.0) };
            config = {"../assets/new_models/tank.obj", 1.0, 0.0, 2.0};
            break;
        case 1: 
            path = { chai3d::cVector3d(startX, 0.0, 0.0), chai3d::cVector3d(endX, 0.0, 0.0) };
            config = {"../assets/new_models/aircraft_1.obj", 0.2, 10.0, 2.5};
            break;
        case 2:
            path = { chai3d::cVector3d(startX, 0.0, 0.0), chai3d::cVector3d(endX, 0.0, 0.0) };
            config = {"../assets/new_models/aircraft_2.obj", 1.0, 5.0, 3.0};
            break;
        case 3:
            path = { chai3d::cVector3d(startX, 0.0, 0.0), chai3d::cVector3d(endX, 0.0, 0.0) };
            config = {"../assets/new_models/space_ship.obj", 1.0, 7.5, 4.0};
            break;
    }

    m_enemies.push_back(new Enemy(m_world, config, path));
}

void Game::render() {
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    
    // Request the camera to render the scene from its perspective into the OpenGL framebuffer
    m_camera->renderView(width, height);
    
    // Swap Buffers to display the rendered frame on the screen
    glfwSwapBuffers(m_window);
}

void Game::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void Game::windowSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void Game::cleanup() {
    if (m_tower) { delete m_tower; m_tower = nullptr; }
    if (m_ground) { delete m_ground; m_ground = nullptr; }
    if (m_camera) { delete m_camera; m_camera = nullptr; }
    if (m_light) { delete m_light; m_light = nullptr; }
    if (m_world) { delete m_world; m_world = nullptr; }
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}


// Functions for controlling camera (Not important)
void Game::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (!game) return;

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            game->m_isDragging = true;
            glfwGetCursorPos(window, &game->m_lastMouseX, &game->m_lastMouseY);
        } else if (action == GLFW_RELEASE) {
            game->m_isDragging = false;
        }
    }
}

void Game::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (!game || !game->m_isDragging) return;

    double dx = xpos - game->m_lastMouseX;
    double dy = ypos - game->m_lastMouseY;

    game->m_lastMouseX = xpos;
    game->m_lastMouseY = ypos;

    double sensitivity = 0.005;
    game->m_camAzimuth -= dx * sensitivity;
    game->m_camElevation += dy * sensitivity;

    game->m_camElevation = chai3d::cClamp(game->m_camElevation, 0.1, 1.5);
    game->updateCamera();
}

void Game::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (!game) return;

    game->m_camRadius -= yoffset * 1.0;
    game->m_camRadius = chai3d::cClamp(game->m_camRadius, 3.0, 40.0); 

    game->updateCamera();
}

void Game::updateCamera() {
    if (!m_camera) return;

    double x = m_camRadius * cos(m_camElevation) * sin(m_camAzimuth);
    double y = -m_camRadius * cos(m_camElevation) * cos(m_camAzimuth);
    double z = m_camRadius * sin(m_camElevation);

    m_camera->set(
        chai3d::cVector3d(x, y, z),        
        chai3d::cVector3d(0.0, 0.0, 0.0),  
        chai3d::cVector3d(0.0, 0.0, 1.0)   
    );
}

