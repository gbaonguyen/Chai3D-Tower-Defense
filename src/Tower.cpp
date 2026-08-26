#include "Tower.h"
#include <iostream>

Tower::Tower(chai3d::cWorld* world)
    : m_world(world),
      m_baseMesh(nullptr),
      m_barrelMesh(nullptr),
      m_yaw(0.0)
{
    m_baseMesh = new chai3d::cMultiMesh();
    if (m_world != nullptr) {
        m_world->addChild(m_baseMesh);
    }
}

Tower::~Tower() {
    if (m_world != nullptr && m_baseMesh != nullptr) {
        m_world->removeChild(m_baseMesh);
        delete m_baseMesh;
        m_baseMesh = nullptr;
        m_barrelMesh = nullptr;
    }
}

// bool Tower::loadBase(const std::string& filePath) {
//     if (!m_baseMesh) return false;

//     bool success = m_baseMesh->loadFromFile(filePath);
//     if (!success) {
//         std::cerr << "[ERROR] Khong tim thay file: " << filePath << std::endl;
//         return false;
//     }

//     // Xoay -90 độ quanh trục X để lật úp phần chân đế tròn xuống sàn
//     m_baseMesh->rotateAboutGlobalAxisDeg(chai3d::cVector3d(1, 0, 0), -90.0);

//     // Tính lại Bounding Box
//     m_baseMesh->computeBoundaryBox(true);
//     chai3d::cVector3d center = m_baseMesh->getBoundaryCenter();
//     chai3d::cVector3d minBox = m_baseMesh->getBoundaryMin();

//     // Dịch tâm X, Y về (0,0) và đưa mặt đáy sát Z = 0
//     chai3d::cVector3d offset(-center.x(), -center.y(), -minBox.z());
//     m_baseMesh->rotateAboutGlobalAxisDeg(chai3d::cVector3d(1, 0, 0), 180);

//     for (unsigned int i = 0; i < m_baseMesh->getNumMeshes(); ++i) {
//         chai3d::cMesh* subMesh = m_baseMesh->getMesh(i);
//         if (subMesh != nullptr) {
//             subMesh->offsetVertices(offset);
//         }
//     }

//     m_baseMesh->computeBoundaryBox(true);
//     m_baseMesh->computeAllNormals();
//     m_baseMesh->setUseMaterial(true);

//     return true;
// }

bool Tower::loadBase(const std::string& filePath) {
    if (!m_baseMesh) return false;

    bool success = m_baseMesh->loadFromFile(filePath);
    if (!success) {
        std::cerr << "[ERROR] Khong tim thay file: " << filePath << std::endl;
        return false;
    }

    // 1. Xoay TRỰC TIẾP CÁC ĐỈNH (Vertices) thay vì xoay Object Matrix
    // Dùng ma trận xoay 90 độ quanh trục X
    chai3d::cMatrix3d rotX;
    rotX.setAxisAngleRotationDeg(chai3d::cVector3d(1, 0, 0), 90.0);

    for (unsigned int i = 0; i < m_baseMesh->getNumMeshes(); ++i) {
        chai3d::cMesh* subMesh = m_baseMesh->getMesh(i);
        if (subMesh != nullptr) {
            // Xoay vĩnh viễn dữ liệu đỉnh của Mesh
            for (unsigned int v = 0; v < subMesh->getNumVertices(); ++v) {
                chai3d::cVector3d pos = subMesh->m_vertices->getLocalPos(v);
                subMesh->m_vertices->setLocalPos(v, rotX * pos);
            }
        }
    }

    m_baseMesh->setShowFrame(true); // Hiển thị trục tọa độ cho bệ tháp

    // 2. Tính toán lại Bounding Box trên hệ đỉnh đã xoay
    m_baseMesh->computeBoundaryBox(true);
    chai3d::cVector3d center = m_baseMesh->getBoundaryCenter();
    chai3d::cVector3d minBox = m_baseMesh->getBoundaryMin();

    // 3. Dời tâm xoay (Pivot) kèm giá trị bù trừ sai số
    // Tùy chỉnh 2 con số này để kéo trục mũi tên RGB về đúng đỉnh nón:
    double x_adj = 0.0; // Nếu tâm bị lệch sang phải/trái: tăng/giảm giá trị này (vd: +50.0 hoặc -50.0)
    double y_adj = -150.0; // Nếu tâm bị lệch lên/xuống: tăng/giảm giá trị này (vd: +50.0 hoặc -50.0)

    chai3d::cVector3d maxBox = m_baseMesh->getBoundaryMax();
    chai3d::cVector3d offset(-center.x() + x_adj, -center.y() + y_adj, -maxBox.z());

    for (unsigned int i = 0; i < m_baseMesh->getNumMeshes(); ++i) {
        chai3d::cMesh* subMesh = m_baseMesh->getMesh(i);
        if (subMesh != nullptr) {
            subMesh->offsetVertices(offset);
        }
    }

    m_baseMesh->computeBoundaryBox(true);
    m_baseMesh->computeAllNormals();
    m_baseMesh->setUseMaterial(true);

    return true;
}

bool Tower::loadBarrel(const std::string& filePath, double joinHeightOffset) {
    if (!m_baseMesh) return false;

    m_barrelMesh = new chai3d::cMultiMesh();
    if (!m_barrelMesh->loadFromFile(filePath)) {
        std::cerr << "[ERROR] Khong tim thay file: " << filePath << std::endl;
        delete m_barrelMesh;
        m_barrelMesh = nullptr;
        return false;
    }

    // 1. Ma trận xoay quanh trục Y (180 độ)
    chai3d::cMatrix3d rotY;
    rotY.setAxisAngleRotationDeg(chai3d::cVector3d(0, 1, 0), 180.0);

    // 2. Ma trận xoay quanh trục X (ví dụ 90 độ, bạn thay góc tùy ý)
    chai3d::cMatrix3d rotX;
    rotX.setAxisAngleRotationDeg(chai3d::cVector3d(1, 0, 0), 90.0);

    // 3. Nhân ma trận để kết hợp 2 phép quay (Thứ tự nhân quyết định thứ tự xoay)
    chai3d::cMatrix3d rotCombined = rotX * rotY;

    // 4. Áp dụng ma trận kết hợp lên từng đỉnh
    for (unsigned int i = 0; i < m_barrelMesh->getNumMeshes(); ++i) {
        chai3d::cMesh* subMesh = m_barrelMesh->getMesh(i);
        if (subMesh != nullptr) {
            for (unsigned int v = 0; v < subMesh->getNumVertices(); ++v) {
                chai3d::cVector3d pos = subMesh->m_vertices->getLocalPos(v);
                subMesh->m_vertices->setLocalPos(v, rotCombined * pos);
            }
        }
    }

    m_barrelMesh->setShowFrame(true); // Hiển thị trục tọa độ cho nòng súng
    
    // 2. Tính lại Bounding Box của nòng súng sau khi xoay đỉnh
    m_barrelMesh->computeBoundaryBox(true);
    m_barrelMesh->setShowBoundaryBox(true); // Hiển thị Bounding Box của nòng súng
    chai3d::cVector3d center = m_barrelMesh->getBoundaryCenter();
    chai3d::cVector3d minBox = m_barrelMesh->getBoundaryMin();
    chai3d::cVector3d maxBox = m_barrelMesh->getBoundaryMax();

    std::cout << "[INFO] Nòng súng Bounding Box: Min(" 
              << minBox.x() << ", " << minBox.y() << ", " << minBox.z() 
              << ") Max(" << maxBox.x() << ", " << maxBox.y() << ", " << maxBox.z() 
              << ") Center(" << center.x() << ", " << center.y() << ", " << center.z() 
              << ")" << std::endl;

    // 3. THIẾT LẬP TÂM BẢN LỀ (PIVOT) CỦA NÒNG SÚNG:
    // - Đưa đuôi nòng súng về 0 (minBox.x nếu nòng dọc X, hoặc minBox.y nếu nòng dọc Y)
    // - Căn giữa 2 trục còn lại để trục quay đi xuyên qua lõi nòng súng.

    chai3d::cVector3d offset(-minBox.x(), -minBox.y(), -100); // Dời đuôi nòng súng về 0 và căn giữa trục Z
    for (unsigned int i = 0; i < m_barrelMesh->getNumMeshes(); ++i) {
        chai3d::cMesh* subMesh = m_barrelMesh->getMesh(i);
        if (subMesh != nullptr) {
            subMesh->offsetVertices(offset);
        }
    }

    // 4. Gắn làm con (Child) của Base
    m_baseMesh->addChild(m_barrelMesh);

    // 5. ĐẶT VỊ TRÍ KHỚP NỐI (LOCAL POSITION):
    // Dịch nòng súng lên khe ngàm phía trước của bệ tháp.
    // Tùy chỉnh x_mount và y_mount để nòng súng nằm lọt thỏm vào đúng rãnh ngàm.
    double x_mount = -400.0;   // Dịch tới / lui theo trục Đỏ
    double y_mount = -50.0;    // Dịch trái / phải theo trục Xanh lá
    double z_mount = 0.0; // Chiều cao ngàm

    m_barrelMesh->setLocalPos(x_mount, y_mount, z_mount);

    m_barrelMesh->computeBoundaryBox(true);
    m_barrelMesh->computeAllNormals();
    m_barrelMesh->setUseMaterial(true);

    return true;
}

void Tower::setYaw(double angleRad) {
    m_yaw = angleRad;
    if (m_baseMesh) {
        chai3d::cMatrix3d rot;
        // Rotate the Base around the Z axis. The child (Barrel) will inherently follow.
        rot.setAxisAngleRotationRad(chai3d::cVector3d(0, 0, 1), m_yaw);
        m_baseMesh->setLocalRot(rot);
    }
}

void Tower::setPosition(const chai3d::cVector3d& pos) {
    if (m_baseMesh) {
        m_baseMesh->setLocalPos(pos);
    }
}

void Tower::setScale(double scale) {
    if (m_baseMesh) {
        m_baseMesh->scale(scale);
    }
}

void Tower::setPitch(double angleRad) {
    // Giới hạn góc ngẩng từ -10 độ (-0.17 rad) đến +60 độ (+1.05 rad)
    // để tránh nòng súng bị đâm xuyên sàn hoặc lộn ra sau
    m_pitch = chai3d::cClamp(angleRad, 0.0, M_PI / 4); // Giới hạn từ 0 đến 60 độ (pi/3 rad)
    
    if (m_barrelMesh) {
        chai3d::cMatrix3d rot;
        // Trục quay ngẩng là trục Y (trục ngang cục bộ của nòng súng)
        rot.setAxisAngleRotationRad(chai3d::cVector3d(0, -1, 0), m_pitch);
        m_barrelMesh->setLocalRot(rot);
    }
}