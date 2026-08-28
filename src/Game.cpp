#include "Game.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

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
      m_camElevation(0.8), // ~ 45 độ
      m_lastMouseX(0.0),
      m_lastMouseY(0.0),
      m_isDragging(false) {}

Game::~Game() {
    cleanup();
}

void Game::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void Game::windowSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

bool Game::init() {

    srand(static_cast<unsigned int>(time(nullptr)));

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

    // Thiết lập liên kết con trỏ Game vào GLFW window để dùng trong Callback tĩnh
    glfwSetWindowUserPointer(m_window, this);

    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);
    glfwSetScrollCallback(m_window, scrollCallback);

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetWindowSizeCallback(m_window, windowSizeCallback);

    // 1. Khởi tạo Thế giới 3D
    m_world = new chai3d::cWorld();
    m_world->m_backgroundColor.set(0.1f, 0.1f, 0.15f);

    // 2. Camera nhìn xéo
    m_camera = new chai3d::cCamera(m_world);
    m_world->addChild(m_camera);
    m_camera->set(
        chai3d::cVector3d(0.0, -14.0, 12.0),
        chai3d::cVector3d(0.0, 0.0, 0.0),
        chai3d::cVector3d(0.0, 0.0, 1.0)
    );
    m_camera->setClippingPlanes(0.1, 100.0);

    // 3. Nguồn sáng
    m_light = new chai3d::cDirectionalLight(m_world);
    m_world->addChild(m_light);
    m_light->setEnabled(true);
    m_light->setDir(-0.5, 0.5, -1.0);

    // 4. Mặt sàn đấu trường
    m_ground = new chai3d::cMesh();
    m_world->addChild(m_ground);
    chai3d::cCreateBox(m_ground, 1000.0, 1000.0, 0.1);
    m_ground->setLocalPos(0.0, 0.0, -1.2);
    m_ground->m_material->setGrayDark();
    m_ground->m_material->m_ambient.set(0.2f, 0.2f, 0.2f);
    m_ground->m_material->m_diffuse.set(0.4f, 0.4f, 0.4f);
    m_ground->m_material->m_specular.set(0.1f, 0.1f, 0.1f);
    m_ground->setUseMaterial(true);

    // Bật hiển thị trục tọa độ cho mặt đất (đang nằm ở quanh gốc 0,0,0)
    m_ground->setShowFrame(true);

    // (Tùy chọn) Điều chỉnh kích thước/chiều dài của trục tọa độ để dễ nhìn hơn
    m_ground->setFrameSize(5.0);

    // 5. Khởi tạo và nạp Tháp (PHẢI NẰM SAU KHI M_WORLD ĐÃ ĐƯỢC NEW)
    m_tower = new Tower(m_world);
    std::string basePath = "../assets/models/turret_base.obj";
    std::string barrelPath = "../assets/models/turret_barrrel.obj";


    if (m_tower->loadBase(basePath)) {  
        // Retrieve the maximum Z-height of the base for relative placement
        double joinHeight = m_tower->getBaseMesh()->getBoundaryMax().z() * 0.55;
        
        if (m_tower->loadBarrel(barrelPath, joinHeight)) {
            m_tower->setScale(0.003);
            m_tower->setPosition(chai3d::cVector3d(0.0, 0.0, 0.0));
        }
    }


    // Add this right before "m_isRunning = true; return true;"[cite: 1]
    
    // Define a sequential trajectory (quỹ đạo tuần tự) across the terrain
    // Chạy từ trong (+15) ra ngoài (-15) dọc theo trục Ox (Y = 0)
    m_pathWaypoints = {
        chai3d::cVector3d(15.0, 0.0, -0.1),  
        chai3d::cVector3d(-15.0, 0.0, -0.1)  
    };

    m_enemySpawnTimer = 2.0; // Wait 2 seconds before the first spawn

    m_isRunning = true;
    return true;
}

void Game::run() {
    m_clock.reset();
    m_clock.start();

    while (!glfwWindowShouldClose(m_window) && m_isRunning) {
        double dt = m_clock.getCurrentTimeSeconds();
        m_clock.reset();
        m_clock.start();

        processInput();
        update(dt);
        render();

        glfwPollEvents();
    }
}

void Game::processInput() {
    if (!m_tower) return;

    static double currentYaw = 0.0;
    static double currentPitch = 0.0;
    const double rotationSpeed = 0.03; // Tốc độ xoay mỗi frame

    // 1. Xoay ngang Base (Yaw) - Phím Trái / Phải
    if (glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        currentYaw += rotationSpeed;
        m_tower->setYaw(currentYaw);
    }

    if (glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        currentYaw -= rotationSpeed;
        m_tower->setYaw(currentYaw);
    }

    // 2. Ngẩng nòng súng (Pitch) - Phím Lên / Xuống
    if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS) {
        currentPitch -= rotationSpeed;
        m_tower->setPitch(currentPitch);
    }

    if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        currentPitch += rotationSpeed;
        m_tower->setPitch(currentPitch);
    }

    if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (m_fireCooldown <= 0.0) {
            // Ép Engine cập nhật ma trận toàn cục trước khi tính toán
            m_world->computeGlobalPositions(true);

            chai3d::cMultiMesh* barrel = m_tower->getBarrelMesh();
            if (barrel) {
                // Lấy vị trí và hướng thật của nòng súng
                chai3d::cMatrix3d globalRot = barrel->getGlobalRot();
                chai3d::cVector3d globalPos = barrel->getGlobalPos();
                
                // Nòng súng ngắm theo trục X dương
                chai3d::cVector3d forward = globalRot * chai3d::cVector3d(1.0, 0.0, 0.0);
                forward.normalize();
                
                // Đẩy vị trí spawn ra đầu nòng súng (cộng thêm một khoảng offset)
                // Bạn có thể cần tăng/giảm số 10.0 để đạn sinh ra ngay mép nòng súng
                chai3d::cVector3d spawnPos = globalPos + forward * 5.0; 
                
                // Sinh tên lửa
                std::string missilePath = "../assets/models/Missile.obj";
                Projectile* p = new Projectile(m_world, missilePath, spawnPos, forward, 40.0); // Tốc độ bay: 40.0
                m_projectiles.push_back(p);
                
                // Đặt thời gian chờ giữa 2 lần bắn (0.2 giây)
                m_fireCooldown = 0.2; 
            }
        }
    }
}

void Game::update(double dt) {
    // 1. Giảm thời gian hồi chiêu
    if (m_fireCooldown > 0.0) {
        m_fireCooldown -= dt;
    }

    // 2. Cập nhật vị trí tên lửa & Thu hồi bộ nhớ
    for (auto it = m_projectiles.begin(); it != m_projectiles.end(); ) {
        Projectile* p = *it;
        if (p == nullptr) {
            it = m_projectiles.erase(it);
            continue;
        }

        p->update(dt);

        if (p->isExpired()) {
            delete p;                      // Destructor của Projectile sẽ tự lo việc gỡ mesh
            it = m_projectiles.erase(it);  // Xóa khỏi danh sách quản lý
        } else {
            ++it;
        }
    }


    m_enemySpawnTimer -= dt;
    if (m_enemySpawnTimer <= 0.0) { 
        std::string models[] = {
            "../assets/models/tank_project.obj", // Index 0
            "../assets/models/aircraft1.obj",    // Index 1
            "../assets/models/aircraft2.obj",    // Index 2
            "../assets/models/spaceship.obj"     // Index 3
        };
        
        int modelIndex = rand() % 4; 
        
        double speed = 0.0;
        double scale = 0.0;
        double pathZ = -0.1; // Default elevation (độ cao mặc định)
        
        chai3d::cMatrix3d preRot;
        preRot.identity();

        if (modelIndex == 0) { // Tank
            speed = 1.5 + (rand() % 15) / 10.0; 
            scale = 0.1;
            pathZ = -0.1; // Terrestrial trajectory (quỹ đạo mặt đất)
            preRot.setAxisAngleRotationDeg(chai3d::cVector3d(1, 0, 0), 90.0);
        }
        else if (modelIndex == 1 ) { // big aircraft
            speed = 3.5 + (rand() % 25) / 10.0; 
            scale = 0.0005;
            pathZ = 4.0; // Aerial trajectory (quỹ đạo trên không)
            chai3d::cMatrix3d rZ, rY;
            rZ.setAxisAngleRotationDeg(chai3d::cVector3d(0, 0, 1), -90.0);
            rY.setAxisAngleRotationDeg(chai3d::cVector3d(0, 1, 0), -90.0);
            preRot = rY * rZ; 
        }

        else if (modelIndex == 2) { // small aircraft
            speed = 3.5 + (rand() % 25) / 10.0; 
            scale = 0.015;
            pathZ = 4.0; // Aerial trajectory (quỹ đạo trên không)
            chai3d::cMatrix3d rX, rZ;
            rX.setAxisAngleRotationDeg(chai3d::cVector3d(1, 0, 0), 90.0);
            rZ.setAxisAngleRotationDeg(chai3d::cVector3d(0, 0, 1), 90.0);
            preRot = rZ * rX;
        }
        else if (modelIndex == 3) { // Spaceship
            speed = 4.0 + (rand() % 30) / 10.0; 
            scale = 0.5;
            pathZ = 5.0; // Higher altitude (độ cao lớn hơn)
            chai3d::cMatrix3d rZ, rY;
            rZ.setAxisAngleRotationDeg(chai3d::cVector3d(0, 0, 1), -90.0);
            rY.setAxisAngleRotationDeg(chai3d::cVector3d(0, 1, 0), -90.0);
            preRot = rY * rZ;
        }
        
        // Determinate linear path along the X-axis (Đường thẳng cố định dọc trục X)
        // Y is strictly set to 0.0 to eliminate lateral randomness
        std::vector<chai3d::cVector3d> fixedPath = {
            chai3d::cVector3d(15.0, 0.0, pathZ),
            chai3d::cVector3d(-15.0, 0.0, pathZ)
        };

        Enemy* newEnemy = new Enemy(m_world, models[modelIndex], fixedPath, speed, scale, preRot);
        m_enemies.push_back(newEnemy);

        newEnemy->setShowFrame(true, 1.0); // Show local axes for debugging

        m_spawnCounter++;
        m_enemySpawnTimer = 1.0 + (rand() % 25) / 10.0; 
    }

    // 4. Enemy Update & Deallocation (Cập nhật & thu hồi bộ nhớ)
    for (auto it = m_enemies.begin(); it != m_enemies.end(); ) {
        Enemy* e = *it;
        if (e == nullptr) {
            it = m_enemies.erase(it);
            continue;
        }

        e->update(dt);

        if (e->hasReachedEnd()) {
            delete e;
            it = m_enemies.erase(it);
        } else {
            ++it;
        }
    }

    m_world->computeGlobalPositions(true);

    for (auto* p : m_projectiles) {
        if (p->isExpired()) continue;
        
        chai3d::cVector3d pPos = p->getMesh()->getGlobalPos();
        double pRadius = p->getCollisionRadius();

        for (auto* e : m_enemies) {
            if (e->hasReachedEnd()) continue;

            chai3d::cVector3d ePos = e->getMesh()->getGlobalPos();
            double eRadius = e->getCollisionRadius();

            // Tính khoảng cách bình phương (Squared distance)
            double distSq = (pPos - ePos).lengthsq();
            
            // Tính tổng bán kính bình phương (Sum of radii squared)
            double radiusSum = pRadius + eRadius;
            double radiusSumSq = radiusSum * radiusSum;

            if (distSq <= radiusSumSq) {
                // Va chạm thành công!
                std::cout << "[COLLISION] Bắn trúng kẻ địch!" << std::endl;
                
                // Tiêu diệt cả hai (Đánh dấu để bộ dọn rác tự xóa ở frame tiếp theo)
                p->kill(); 
                e->kill(); 
                
                // Mỗi viên đạn chỉ trúng 1 quái, thoát vòng lặp kẻ địch hiện tại
                break; 
            }
        }
    }
}

void Game::render() {
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    m_camera->renderView(width, height);
    glfwSwapBuffers(m_window);
}

void Game::cleanup() {
    if (m_tower) {
        delete m_tower;
        m_tower = nullptr;
    }
    if (m_ground) {
        delete m_ground;
        m_ground = nullptr;
    }
    if (m_camera) {
        delete m_camera;
        m_camera = nullptr;
    }
    if (m_light) {
        delete m_light;
        m_light = nullptr;
    }
    if (m_world) {
        delete m_world;
        m_world = nullptr;
    }
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}



void Game::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (!game) return;

    // Giữ chuột phải (hoặc chuột trái) để xoay camera
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

    // Độ nhạy chuột
    double sensitivity = 0.005;

    // Xoay ngang (Azimuth)
    game->m_camAzimuth -= dx * sensitivity;

    // Nâng/hạ góc nhìn (Elevation)
    game->m_camElevation += dy * sensitivity;

    // Giới hạn góc nâng để không bị lộn ngược camera (từ 5 độ đến 85 độ)
    game->m_camElevation = chai3d::cClamp(game->m_camElevation, 0.1, 1.5);

    game->updateCamera();
}

void Game::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (!game) return;

    // Thu phóng (Zoom in / Zoom out)
    game->m_camRadius -= yoffset * 1.0;
    game->m_camRadius = chai3d::cClamp(game->m_camRadius, 3.0, 100.0); // Giới hạn tầm zoom

    game->updateCamera();
}


void Game::updateCamera() {
    if (!m_camera) return;

    double x = m_camRadius * cos(m_camElevation) * sin(m_camAzimuth);
    double y = -m_camRadius * cos(m_camElevation) * cos(m_camAzimuth);
    double z = m_camRadius * sin(m_camElevation);

    m_camera->set(
        chai3d::cVector3d(x, y, z),        // Vị trí mới của Camera
        chai3d::cVector3d(0.0, 0.0, 0.0),  // Luôn nhìn vào tâm thế giới
        chai3d::cVector3d(0.0, 0.0, 1.0)   // Trục Z hướng lên trời
    );
}