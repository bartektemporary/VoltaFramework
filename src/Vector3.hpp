#ifndef VECTOR3_HPP
#define VECTOR3_HPP

#include <lua.hpp> // For lua_State*
#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>
#include <cstring>

struct Vector3 {
    float x, y, z;
    Vector3(float x_ = 0.0f, float y_ = 0.0f, float z_ = 0.0f) : x(x_), y(y_), z(z_) {}
};

// Function declarations
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