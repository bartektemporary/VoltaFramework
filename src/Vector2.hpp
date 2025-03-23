#ifndef VECTOR2_HPP
#define VECTOR2_HPP

#include <lua.hpp> // Include Lua directly for bindings
#include <cstring>

// Define Vector2 here as the canonical definition
struct Vector2 {
    float x, y;
    Vector2(float x_ = 0.0f, float y_ = 0.0f) : x(x_), y(y_) {}
};

// Function declarations
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