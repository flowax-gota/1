#ifndef VECTOR3_H
#define VECTOR3_H

#include <cmath>
#include <iostream>

namespace ElectronicSandbox {

/**
 * @brief Simple 3D vector class for position and transformations
 */
class Vector3 {
public:
    float x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    // Operators
    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }

    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }

    Vector3 operator*(float scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }

    Vector3& operator+=(const Vector3& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    bool operator==(const Vector3& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    // Utility functions
    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vector3 normalize() const {
        float len = length();
        if (len > 0) {
            return Vector3(x / len, y / len, z / len);
        }
        return *this;
    }

    float dot(const Vector3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    Vector3 cross(const Vector3& other) const {
        return Vector3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    // String representation for debugging
    std::string toString() const {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
    }
};

} // namespace ElectronicSandbox

#endif // VECTOR3_H
