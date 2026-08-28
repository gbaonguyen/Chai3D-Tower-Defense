#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <limits>
#include <algorithm>
#include <iomanip>

struct Vector3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct BoundingBox {
    Vector3 minPoint;
    Vector3 maxPoint;

    Vector3 getSize() const {
        return {
            maxPoint.x - minPoint.x,
            maxPoint.y - minPoint.y,
            maxPoint.z - minPoint.z
        };
    }

    Vector3 getCenter() const {
        return {
            (minPoint.x + maxPoint.x) * 0.5f,
            (minPoint.y + maxPoint.y) * 0.5f,
            (minPoint.z + maxPoint.z) * 0.5f
        };
    }
};

bool calculateObjDimensions(const std::string& filePath, BoundingBox& bbox, size_t& vertexCount) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Lỗi: Không thể mở file " << filePath << "\n";
        return false;
    }

    float inf = std::numeric_limits<float>::infinity();
    bbox.minPoint = { inf, inf, inf };
    bbox.maxPoint = { -inf, -inf, -inf };
    vertexCount = 0;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        // Chỉ xử lý các dòng định nghĩa đỉnh 'v' (bỏ qua 'vn', 'vt', 'vp',...)
        if (prefix == "v") {
            Vector3 v;
            if (iss >> v.x >> v.y >> v.z) {
                bbox.minPoint.x = std::min(bbox.minPoint.x, v.x);
                bbox.minPoint.y = std::min(bbox.minPoint.y, v.y);
                bbox.minPoint.z = std::min(bbox.minPoint.z, v.z);

                bbox.maxPoint.x = std::max(bbox.maxPoint.x, v.x);
                bbox.maxPoint.y = std::max(bbox.maxPoint.y, v.y);
                bbox.maxPoint.z = std::max(bbox.maxPoint.z, v.z);

                vertexCount++;
            }
        }
    }

    file.close();
    return vertexCount > 0;
}

int main(int argc, char* argv[]) {
    std::string filePath = "../assets/models/Missile.obj"; // Đường dẫn mặc định nếu không có tham số dòng lệnh

    if (argc > 1) {
        filePath = argv[1];
    } else {
        std::cout << "Gợi ý: Bạn có thể truyền đường dẫn file qua tham số dòng lệnh:\n";
        std::cout << "       ./checksize path/to/model.obj\n\n";
    }

    std::cout << "Đang đọc file: " << filePath << " ...\n";

    BoundingBox bbox;
    size_t vertexCount = 0;

    if (!calculateObjDimensions(filePath, bbox, vertexCount)) {
        std::cerr << "Không tìm thấy đỉnh (vertex) hợp lệ hoặc file rỗng.\n";
        return 1;
    }

    Vector3 size = bbox.getSize();
    Vector3 center = bbox.getCenter();

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "\n========== KẾT QUẢ ĐO ĐẠC ==========\n";
    std::cout << "Tổng số đỉnh (vertices): " << vertexCount << "\n";
    std::cout << "------------------------------------\n";
    std::cout << "Bounding Box Min : (" << bbox.minPoint.x << ", " << bbox.minPoint.y << ", " << bbox.minPoint.z << ")\n";
    std::cout << "Bounding Box Max : (" << bbox.maxPoint.x << ", " << bbox.maxPoint.y << ", " << bbox.maxPoint.z << ")\n";
    std::cout << "------------------------------------\n";
    std::cout << "Kích thước (Size):\n";
    std::cout << "  - Chiều rộng (Width  - X): " << size.x << "\n";
    std::cout << "  - Chiều cao  (Height - Y): " << size.y << "\n";
    std::cout << "  - Chiều sâu  (Depth  - Z): " << size.z << "\n";
    std::cout << "------------------------------------\n";
    std::cout << "Tọa độ tâm (Center): (" << center.x << ", " << center.y << ", " << center.z << ")\n";
    std::cout << "====================================\n";

    return 0;
}