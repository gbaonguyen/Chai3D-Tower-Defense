#include "Game.h"
#include <iostream>

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
    chai3d::cCreateBox(m_ground, 16.0, 16.0, 0.1);
    m_ground->setLocalPos(0.0, 0.0, -0.1);
    m_ground->m_material->setGrayDark();
    m_ground->m_material->m_ambient.set(0.2f, 0.2f, 0.2f);
    m_ground->m_material->m_diffuse.set(0.4f, 0.4f, 0.4f);
    m_ground->m_material->m_specular.set(0.1f, 0.1f, 0.1f);
    m_ground->setUseMaterial(true);

    // Bật hiển thị trục tọa độ cho mặt đất (đang nằm ở quanh gốc 0,0,0)
    m_ground->setShowFrame(false); // Ban đầu tắt hiển thị trục tọa độ

    // (Tùy chọn) Điều chỉnh kích thước/chiều dài của trục tọa độ để dễ nhìn hơn
    m_ground->setFrameSize(5.0);

    // 5. Khởi tạo và nạp Tháp (PHẢI NẰM SAU KHI M_WORLD ĐÃ ĐƯỢC NEW)
    m_tower = new Tower(m_world);
    
    // Update paths to the new directory structure
    std::string basePath = "../assets/new_models/turret_base.obj";
    std::string barrelPath = "../assets/new_models/turret_barrel.obj";

    if (m_tower->loadBase(basePath)) {  
        // Removed the manual joinHeight calculation since the new barrel is pre-positioned (đã được định vị sẵn).
        if (m_tower->loadBarrel(barrelPath)) {
            // Removed m_tower->setScale() assuming the new models are pre-scaled.
            m_tower->setPosition(chai3d::cVector3d(0.0, 0.0, 0.0));
        }
    }

    
    // THIẾT LẬP ĐƯỜNG BAY THỬ NGHIỆM (TESTING WAYPOINTS)
    // Mọi kẻ địch đều xuất phát từ xa trên trục +Ox (Y = 0) và tiến thẳng về phía tháp (dừng ở X = 3.0)

    // 1. Xe Tăng (Dưới đất, Z = 0) - Xuất phát gần nhất
    std::vector<chai3d::cVector3d> pathTank = {
        chai3d::cVector3d(20.0, 0.0, 0.0), 
        chai3d::cVector3d(3.0, 0.0, 0.0)
    };
    EnemyConfig configTank = {"../assets/new_models/tank.obj", 1.0, 0.0, 1.5};

    // 2. Aircraft 1 (Bay thấp, Z = 3.0) - Xuất phát xa hơn một chút
    std::vector<chai3d::cVector3d> pathAir1 = {
        chai3d::cVector3d(25.0, 0.0, 0.0), 
        chai3d::cVector3d(3.0, 0.0, 0.0)
    };
    EnemyConfig configAir1 = {"../assets/new_models/aircraft_1.obj", 1.0, 3.0, 2.0};

    // 3. Aircraft 2 (Bay vừa, Z = 6.0)
    std::vector<chai3d::cVector3d> pathAir2 = {
        chai3d::cVector3d(30.0, 0.0, 0.0), 
        chai3d::cVector3d(3.0, 0.0, 0.0)
    };
    EnemyConfig configAir2 = {"../assets/new_models/aircraft_2.obj", 1.0, 6.0, 2.5};

    // 4. Spaceship (Bay cao nhất, Z = 9.0) - Xuất phát xa nhất
    std::vector<chai3d::cVector3d> pathSpace = {
        chai3d::cVector3d(35.0, 0.0, 0.0), 
        chai3d::cVector3d(3.0, 0.0, 0.0)
    };
    EnemyConfig configSpace = {"../assets/new_models/space_ship.obj", 1.0, 9.0, 3.0};

    // Nạp vào hệ thống (Load into system)
    // m_enemies.push_back(new Enemy(m_world, configTank, pathTank));
    // m_enemies.push_back(new Enemy(m_world, configAir1, pathAir1));
    // m_enemies.push_back(new Enemy(m_world, configAir2, pathAir2));
    // m_enemies.push_back(new Enemy(m_world, configSpace, pathSpace));

    srand(static_cast<unsigned int>(time(nullptr)));

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
        currentPitch += rotationSpeed;
        m_tower->setPitch(currentPitch);
    }
    if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        currentPitch -= rotationSpeed;
        m_tower->setPitch(currentPitch);
    }



    if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (m_fireCooldown <= 0.0) {
            // Mandate a global position update to fetch accurate orientation (Bắt buộc cập nhật vị trí toàn cục để lấy hướng chính xác)
            m_world->computeGlobalPositions(true);

            chai3d::cMultiMesh* barrel = m_tower->getBarrelMesh();
            if (barrel) {
                chai3d::cMatrix3d globalRot = barrel->getGlobalRot();
                chai3d::cVector3d globalPos = barrel->getGlobalPos();
                
                // Formulate the forward vector by transforming the +Ox vector (Tạo vector tiến bằng cách biến đổi vector +Ox)
                chai3d::cVector3d forward = globalRot * chai3d::cVector3d(1.0, 0.0, 0.0);
                forward.normalize();
                
                // Preclude clipping (Ngăn chặn xuyên thấu) by projecting the spawn point to the barrel's apex. 
                // Adjust the '400.0' multiplier based on your barrel's actual length.
                double barrelLengthOffset = 2.0; 
                chai3d::cVector3d spawnPos = globalPos + forward * barrelLengthOffset; 
                
                std::string missilePath = "../assets/new_models/missile.obj";
                Projectile* p = new Projectile(m_world, missilePath, spawnPos, forward, 20.0); // Speed = 500.0
                m_projectiles.push_back(p);
                
                m_fireCooldown = 0.2; 
            }
        }
    }
}

void Game::update(double dt) {
    // 1. Quản lý thời gian bắn đạn
    if (m_fireCooldown > 0.0) {
        m_fireCooldown -= dt;
    }

    // 2. Logic sinh kẻ địch ngẫu nhiên theo thời gian
    m_enemySpawnTimer -= dt;
    if (m_enemySpawnTimer <= 0.0) {
        spawnRandomEnemy();
        // Đặt lại thời gian đếm ngược (Reset the timer)
        m_enemySpawnTimer = m_enemySpawnInterval; 
    }

    // Xác định bán kính tối đa của bản đồ (có thể tùy chỉnh theo kích thước sân đấu của bạn)
    const double MAP_RADIUS = 150.0; 

    // Duyệt qua std::vector chứa các viên đạn
    for (auto it = m_projectiles.begin(); it != m_projectiles.end(); ) {
        Projectile* p = *it;
        
        // Kiểm tra an toàn để tránh lỗi con trỏ rỗng (Null Pointer Exception)
        if (p == nullptr) {
            it = m_projectiles.erase(it);
            continue;
        }

        // 1. Cập nhật vị trí viên đạn di chuyển lên phía trước dựa trên dt
        p->update(dt);

        // 2. Tính toán khoảng cách vô hướng từ gốc tọa độ (0,0,0) đến vị trí viên đạn
        double distanceFromOrigin = p->getPosition().length();

        // 3. Đánh giá xem đạn đã bay vượt ranh giới hay chưa
        bool isOutOfBounds = distanceFromOrigin > MAP_RADIUS;
        
        // Nếu đạn đã hết vòng đời (Time-to-live) HOẶC bay quá giới hạn bản đồ
        if (p->isExpired() || isOutOfBounds) {
            // Lệnh 'delete' sẽ kích hoạt Destructor trong Projectile.cpp, 
            // tự động gỡ mảng lưới (Mesh) khỏi cWorld để thu hồi bộ nhớ (Memory Allocation).
            delete p;                      
            
            // Xóa con trỏ đạn khỏi std::vector
            it = m_projectiles.erase(it);  
        } else {
            // Tiếp tục kiểm tra viên đạn tiếp theo
            ++it;
        }
    }


    // Cập nhật di chuyển cho Kẻ địch
    for (auto it = m_enemies.begin(); it != m_enemies.end(); ) {
        Enemy* e = *it;
        if (e == nullptr) {
            it = m_enemies.erase(it);
            continue;
        }

        e->update(dt);

        // Xóa kẻ địch nếu đã bay đến trạm cuối cùng (Waypoints completed)
        if (e->hasReachedDestination()) {
            delete e;
            it = m_enemies.erase(it);
        } else {
            ++it;
        }
    }
}

void Game::spawnRandomEnemy() {
    // Sinh số ngẫu nhiên từ 0 đến 3 (Random integer between 0 and 3)
    int enemyType = rand() % 4; 

    EnemyConfig config;
    std::vector<chai3d::cVector3d> path;

    // Thiết lập chung: Tất cả đều đi từ X=35.0 tiến về X=3.0 trên trục +Ox
    double startX = 35.0;
    double endX = 3.0;

    switch (enemyType) {
        case 0: // Tank (Mặt đất)
            path = { chai3d::cVector3d(startX, 0.0, 0.0), chai3d::cVector3d(endX, 0.0, 0.0) };
            config = {"../assets/new_models/tank.obj", 1.0, 0.0, 2.0};
            break;
        case 1: // Aircraft 1 (Tầm thấp)
            path = { chai3d::cVector3d(startX, 0.0, 0.0), chai3d::cVector3d(endX, 0.0, 0.0) };
            config = {"../assets/new_models/aircraft_1.obj", 0.2, 3.0, 2.5};
            break;
        case 2: // Aircraft 2 (Tầm trung)
            path = { chai3d::cVector3d(startX, 0.0, 0.0), chai3d::cVector3d(endX, 0.0, 0.0) };
            config = {"../assets/new_models/aircraft_2.obj", 1.0, 6.0, 3.0};
            break;
        case 3: // Spaceship (Tầm cao)
            path = { chai3d::cVector3d(startX, 0.0, 0.0), chai3d::cVector3d(endX, 0.0, 0.0) };
            config = {"../assets/new_models/space_ship.obj", 1.0, 9.0, 4.0};
            break;
    }

    m_enemies.push_back(new Enemy(m_world, config, path));
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
    game->m_camRadius = chai3d::cClamp(game->m_camRadius, 3.0, 40.0); // Giới hạn tầm zoom

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