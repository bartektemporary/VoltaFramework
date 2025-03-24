#ifndef VECTOR2_HPP
#define VECTOR2_HPP

#include <lua.hpp> // Include Lua directly for bindings
#include <cmath>
#include <cstring>
#include <string>
#include <stdexcept>

// Define Vector2 with C++ functionality
struct Vector2 {
    float x, y;

    // Constructors
    Vector2(float x_ = 0.0f, float y_ = 0.0f) : x(x_), y(y_) {}

    // Arithmetic operators
    Vector2 operator+(const Vector2& other) const { return Vector2(x + other.x, y + other.y); }
    Vector2 operator-(const Vector2& other) const { return Vector2(x - other.x, y - other.y); }
    Vector2 operator*(float scalar) const { return Vector2(x * scalar, y * scalar); }
    Vector2 operator*(const Vector2& other) const { return Vector2(x * other.x, y * other.y); }
    Vector2 operator/(float scalar) const {
        if (scalar == 0) throw std::runtime_error("Division by zero");
        return Vector2(x / scalar, y / scalar);
    }
    Vector2 operator/(const Vector2& other) const {
        if (other.x == 0 || other.y == 0) throw std::runtime_error("Division by zero");
        return Vector2(x / other.x, y / other.y);
    }

    // Compound assignment operators
    Vector2& operator+=(const Vector2& other) { x += other.x; y += other.y; return *this; }
    Vector2& operator-=(const Vector2& other) { x -= other.x; y -= other.y; return *this; }
    Vector2& operator*=(float scalar) { x *= scalar; y *= scalar; return *this; }
    Vector2& operator/=(float scalar) {
        if (scalar == 0) throw std::runtime_error("Division by zero");
        x /= scalar; y /= scalar; return *this;
    }

    // Utility methods
    float magnitude() const { return std::sqrt(x * x + y * y); }
    Vector2 normalized() const {
        float mag = magnitude();
        if (mag == 0) return Vector2(0, 0);
        return Vector2(x / mag, y / mag);
    }
    float dot(const Vector2& other) const { return x * other.x + y * other.y; }
    Vector2 lerp(const Vector2& target, float t) const {
        return Vector2(x + (target.x - x) * t, y + (target.y - y) * t);
    }
    float distance(const Vector2& other) const {
        float dx = other.x - x;
        float dy = other.y - y;
        return std::sqrt(dx * dx + dy * dy);
    }
    float angle(const Vector2& other) const {
        float d = dot(other);
        float mags = magnitude() * other.magnitude();
        if (mags == 0) return 0.0f;
        float cosTheta = d / mags;
        cosTheta = std::max(-1.0f, std::min(1.0f, cosTheta));
        return std::acos(cosTheta) * 180.0f / M_PI;
    }
    std::string toString() const {
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "Vector2(%.2f, %.2f)", x, y);
        return std::string(buffer);
    }

    // Tween method with easing (forward declaration for implementation in .cpp)
    Vector2 tween(const Vector2& target, float t, const char* direction, const char* style) const;
};

// Lua function declarations
Vector2* checkVector2(lua_State* L, int index);
int l_vector2_new(lua_State* L);
int l_vector2_add(lua_State* L);
int l_vector2_subtract(lua_State* L);
int l_vector2_multiply(lua_State* L);
int l_vector2_divide(lua_State* L);
int l_vector2_magnitude(lua_State* L);
int l_vector2_normalize(lua_State* L);
int l_vector2_dot(lua_State* L);
int l_vector2_lerp(lua_State* L);
int l_vector2_distance(lua_State* L);
int l_vector2_angle(lua_State* L);
int l_vector2_tostring(lua_State* L);
int l_vector2_tween(lua_State* L);

#endif // VECTOR2_HPP