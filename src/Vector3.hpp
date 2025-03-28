#ifndef VECTOR3_HPP
#define VECTOR3_HPP

#include <lua.hpp>
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstring>
#include <string>
#include <stdexcept>

struct Vector3 {
    float x, y, z;

    // Constructor (immutable after creation)
    Vector3(float x_ = 0.0f, float y_ = 0.0f, float z_ = 0.0f) : x(x_), y(y_), z(z_) {}

    // Arithmetic operations (return new instances)
    Vector3 add(const Vector3& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
    Vector3 subtract(const Vector3& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }
    Vector3 multiply(float scalar) const { return Vector3(x * scalar, y * scalar, z * scalar); }
    Vector3 multiply(const Vector3& other) const { return Vector3(x * other.x, y * other.y, z * other.z); }
    Vector3 divide(float scalar) const {
        if (scalar == 0) throw std::runtime_error("Division by zero");
        return Vector3(x / scalar, y / scalar, z / scalar);
    }
    Vector3 divide(const Vector3& other) const {
        if (other.x == 0 || other.y == 0 || other.z == 0) throw std::runtime_error("Division by zero");
        return Vector3(x / other.x, y / other.y, z / other.z);
    }

    // Utility methods (all const, return new instances where applicable)
    float magnitude() const { return std::sqrt(x * x + y * y + z * z); }
    Vector3 normalized() const {
        float mag = magnitude();
        if (mag == 0) return Vector3(0, 0, 0);
        return Vector3(x / mag, y / mag, z / mag);
    }
    float dot(const Vector3& other) const { return x * other.x + y * other.y + z * other.z; }
    Vector3 lerp(const Vector3& target, float t) const {
        return Vector3(x + (target.x - x) * t, y + (target.y - y) * t, z + (target.z - z) * t);
    }
    float distance(const Vector3& other) const {
        float dx = other.x - x;
        float dy = other.y - y;
        float dz = other.z - z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }
    float angle(const Vector3& other) const {
        float d = dot(other);
        float mags = magnitude() * other.magnitude();
        if (mags == 0) return 0.0f;
        float cosTheta = d / mags;
        cosTheta = std::max(-1.0f, std::min(1.0f, cosTheta));
        return std::acos(cosTheta) * 180.0f / M_PI;
    }
    std::string toString() const {
        char buffer[96];
        snprintf(buffer, sizeof(buffer), "Vector3(%.2f, %.2f, %.2f)", x, y, z);
        return std::string(buffer);
    }

    // Tween method (returns new instance)
    Vector3 tween(const Vector3& target, float t, const char* direction, const char* style) const;
};

// Lua function declarations
Vector3* checkVector3(lua_State* L, int index);
int l_vector3_new(lua_State* L);
int l_vector3_add(lua_State* L);
int l_vector3_subtract(lua_State* L);
int l_vector3_multiply(lua_State* L);
int l_vector3_divide(lua_State* L);
int l_vector3_magnitude(lua_State* L);
int l_vector3_normalize(lua_State* L);
int l_vector3_dot(lua_State* L);
int l_vector3_lerp(lua_State* L);
int l_vector3_distance(lua_State* L);
int l_vector3_angle(lua_State* L);
int l_vector3_tostring(lua_State* L);
int l_vector3_tween(lua_State* L);

#endif // VECTOR3_HPP