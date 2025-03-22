#ifndef VECTOR3_HPP
#define VECTOR3_HPP

#include "VoltaFramework.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

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