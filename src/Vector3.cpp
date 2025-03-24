#include "Vector3.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

// Helper function to check Vector3 userdata
Vector3* checkVector3(lua_State* L, int index) {
    return static_cast<Vector3*>(luaL_checkudata(L, index, "Vector3"));
}

// Push a new Vector3 to Lua stack
static void pushVector3(lua_State* L, const Vector3& vec) {
    Vector3* result = static_cast<Vector3*>(lua_newuserdata(L, sizeof(Vector3)));
    *result = vec;
    luaL_getmetatable(L, "Vector3");
    lua_setmetatable(L, -2);
}

// Easing functions (unchanged)
static float easeLinear(float t) { return t; }
static float easeSineIn(float t) { return 1.0f - std::cos(t * M_PI / 2.0f); }
static float easeSineOut(float t) { return std::sin(t * M_PI / 2.0f); }
static float easeSineInOut(float t) { return -0.5f * (std::cos(M_PI * t) - 1.0f); }
static float easeQuadIn(float t) { return t * t; }
static float easeQuadOut(float t) { return t * (2.0f - t); }
static float easeQuadInOut(float t) { return (t < 0.5f) ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t; }
static float easeCubicIn(float t) { return t * t * t; }
static float easeCubicOut(float t) { float t1 = t - 1.0f; return t1 * t1 * t1 + 1.0f; }
static float easeCubicInOut(float t) { return (t < 0.5f) ? 4.0f * t * t * t : 4.0f * (t - 1.0f) * (t - 1.0f) * (t - 1.0f) + 1.0f; }
static float easeQuartIn(float t) { return t * t * t * t; }
static float easeQuartOut(float t) { float t1 = t - 1.0f; return 1.0f - t1 * t1 * t1 * t1; }
static float easeQuartInOut(float t) { return (t < 0.5f) ? 8.0f * t * t * t * t : 1.0f - 8.0f * (t - 1.0f) * (t - 1.0f) * (t - 1.0f) * (t - 1.0f); }
static float easeQuintIn(float t) { return t * t * t * t * t; }
static float easeQuintOut(float t) { float t1 = t - 1.0f; return 1.0f + t1 * t1 * t1 * t1 * t1; }
static float easeQuintInOut(float t) { return (t < 0.5f) ? 16.0f * t * t * t * t * t : 1.0f + 16.0f * (t - 1.0f) * (t - 1.0f) * (t - 1.0f) * (t - 1.0f) * (t - 1.0f); }
static float easeExponentialIn(float t) { return (t == 0.0f) ? 0.0f : std::pow(2.0f, 10.0f * (t - 1.0f)); }
static float easeExponentialOut(float t) { return (t == 1.0f) ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t); }
static float easeExponentialInOut(float t) { if (t == 0.0f) return 0.0f; if (t == 1.0f) return 1.0f; return (t < 0.5f) ? 0.5f * std::pow(2.0f, 10.0f * (2.0f * t - 1.0f)) : 0.5f * (2.0f - std::pow(2.0f, -10.0f * (2.0f * t - 1.0f))); }
static float easeCircularIn(float t) { return 1.0f - std::sqrt(1.0f - t * t); }
static float easeCircularOut(float t) { float t1 = t - 1.0f; return std::sqrt(1.0f - t1 * t1); }
static float easeCircularInOut(float t) { return (t < 0.5f) ? 0.5f * (1.0f - std::sqrt(1.0f - 4.0f * t * t)) : 0.5f * (std::sqrt(1.0f - (2.0f * t - 2.0f) * (2.0f * t - 2.0f)) + 1.0f); }
static float easeBackIn(float t) { const float c1 = 1.70158f; return t * t * ((c1 + 1.0f) * t - c1); }
static float easeBackOut(float t) { const float c1 = 1.70158f; float t1 = t - 1.0f; return 1.0f + t1 * t1 * ((c1 + 1.0f) * t1 + c1); }
static float easeBackInOut(float t) { const float c1 = 1.70158f; const float c2 = c1 * 1.525f; return (t < 0.5f) ? 0.5f * (4.0f * t * t * ((c2 + 1.0f) * 2.0f * t - c2)) : 0.5f * ((2.0f * t - 2.0f) * (2.0f * t - 2.0f) * ((c2 + 1.0f) * (2.0f * t - 2.0f) + c2) + 2.0f); }
static float easeBounceOut(float t) { if (t < 1.0f / 2.75f) return 7.5625f * t * t; else if (t < 2.0f / 2.75f) { float t1 = t - 1.5f / 2.75f; return 7.5625f * t1 * t1 + 0.75f; } else if (t < 2.5f / 2.75f) { float t1 = t - 2.25f / 2.75f; return 7.5625f * t1 * t1 + 0.9375f; } else { float t1 = t - 2.625f / 2.75f; return 7.5625f * t1 * t1 + 0.984375f; } }
static float easeBounceIn(float t) { return 1.0f - easeBounceOut(1.0f - t); }
static float easeBounceInOut(float t) { return (t < 0.5f) ? 0.5f * easeBounceIn(2.0f * t) : 0.5f * easeBounceOut(2.0f * t - 1.0f) + 0.5f; }
static float easeElasticIn(float t) { if (t == 0.0f) return 0.0f; if (t == 1.0f) return 1.0f; const float p = 0.3f; const float a = 1.0f; return -a * std::pow(2.0f, 10.0f * (t - 1.0f)) * std::sin((t - 1.0f - p / (2.0f * M_PI) * std::asin(1.0f / a)) * (2.0f * M_PI) / p); }
static float easeElasticOut(float t) { if (t == 0.0f) return 0.0f; if (t == 1.0f) return 1.0f; const float p = 0.3f; const float a = 1.0f; return a * std::pow(2.0f, -10.0f * t) * std::sin((t - p / (2.0f * M_PI) * std::asin(1.0f / a)) * (2.0f * M_PI) / p) + 1.0f; }
static float easeElasticInOut(float t) { if (t == 0.0f) return 0.0f; if (t == 1.0f) return 1.0f; const float p = 0.3f * 1.5f; const float a = 1.0f; float t_adj = t * 2.0f; if (t_adj < 1.0f) return -0.5f * a * std::pow(2.0f, 10.0f * (t_adj - 1.0f)) * std::sin((t_adj - 1.0f - p / (2.0f * M_PI) * std::asin(1.0f / a)) * (2.0f * M_PI) / p); t_adj -= 1.0f; return 0.5f * a * std::pow(2.0f, -10.0f * t_adj) * std::sin((t_adj - p / (2.0f * M_PI) * std::asin(1.0f / a)) * (2.0f * M_PI) / p) + 1.0f; }

// Vector3 tween implementation
Vector3 Vector3::tween(const Vector3& target, float t, const char* direction, const char* style) const {
    t = std::max(0.0f, std::min(1.0f, t));
    float easedT = 0.0f;

    if (strcmp(style, "linear") == 0) {
        easedT = easeLinear(t);
    } else if (strcmp(style, "sine") == 0) {
        if (strcmp(direction, "in") == 0) easedT = easeSineIn(t);
        else if (strcmp(direction, "out") == 0) easedT = easeSineOut(t);
        else if (strcmp(direction, "inout") == 0) easedT = easeSineInOut(t);
        else throw std::runtime_error("Invalid direction");
    } else if (strcmp(style, "quad") == 0) {
        if (strcmp(direction, "in") == 0) easedT = easeQuadIn(t);
        else if (strcmp(direction, "out") == 0) easedT = easeQuadOut(t);
        else if (strcmp(direction, "inout") == 0) easedT = easeQuadInOut(t);
        else throw std::runtime_error("Invalid direction");
    } else if (strcmp(style, "cubic") == 0) {
        if (strcmp(direction, "in") == 0) easedT = easeCubicIn(t);
        else if (strcmp(direction, "out") == 0) easedT = easeCubicOut(t);
        else if (strcmp(direction, "inout") == 0) easedT = easeCubicInOut(t);
        else throw std::runtime_error("Invalid direction");
    } else if (strcmp(style, "quart") == 0) {
        if (strcmp(direction, "in") == 0) easedT = easeQuartIn(t);
        else if (strcmp(direction, "out") == 0) easedT = easeQuartOut(t);
        else if (strcmp(direction, "inout") == 0) easedT = easeQuartInOut(t);
        else throw std::runtime_error("Invalid direction");
    } else if (strcmp(style, "quint") == 0) {
        if (strcmp(direction, "in") == 0) easedT = easeQuintIn(t);
        else if (strcmp(direction, "out") == 0) easedT = easeQuintOut(t);
        else if (strcmp(direction, "inout") == 0) easedT = easeQuintInOut(t);
        else throw std::runtime_error("Invalid direction");
    } else if (strcmp(style, "exponential") == 0) {
        if (strcmp(direction, "in") == 0) easedT = easeExponentialIn(t);
        else if (strcmp(direction, "out") == 0) easedT = easeExponentialOut(t);
        else if (strcmp(direction, "inout") == 0) easedT = easeExponentialInOut(t);
        else throw std::runtime_error("Invalid direction");
    } else if (strcmp(style, "circular") == 0) {
        if (strcmp(direction, "in") == 0) easedT = easeCircularIn(t);
        else if (strcmp(direction, "out") == 0) easedT = easeCircularOut(t);
        else if (strcmp(direction, "inout") == 0) easedT = easeCircularInOut(t);
        else throw std::runtime_error("Invalid direction");
    } else if (strcmp(style, "back") == 0) {
        if (strcmp(direction, "in") == 0) easedT = easeBackIn(t);
        else if (strcmp(direction, "out") == 0) easedT = easeBackOut(t);
        else if (strcmp(direction, "inout") == 0) easedT = easeBackInOut(t);
        else throw std::runtime_error("Invalid direction");
    } else if (strcmp(style, "bounce") == 0) {
        if (strcmp(direction, "in") == 0) easedT = easeBounceIn(t);
        else if (strcmp(direction, "out") == 0) easedT = easeBounceOut(t);
        else if (strcmp(direction, "inout") == 0) easedT = easeBounceInOut(t);
        else throw std::runtime_error("Invalid direction");
    } else if (strcmp(style, "elastic") == 0) {
        if (strcmp(direction, "in") == 0) easedT = easeElasticIn(t);
        else if (strcmp(direction, "out") == 0) easedT = easeElasticOut(t);
        else if (strcmp(direction, "inout") == 0) easedT = easeElasticInOut(t);
        else throw std::runtime_error("Invalid direction");
    } else {
        throw std::runtime_error("Invalid easing style");
    }

    return Vector3(x + (target.x - x) * easedT, y + (target.y - y) * easedT, z + (target.z - z) * easedT);
}

// Lua bindings
int l_vector3_new(lua_State* L) {
    float x = static_cast<float>(luaL_checknumber(L, 1));
    float y = static_cast<float>(luaL_checknumber(L, 2));
    float z = static_cast<float>(luaL_checknumber(L, 3));
    pushVector3(L, Vector3(x, y, z));
    return 1;
}

int l_vector3_add(lua_State* L) {
    Vector3* a = checkVector3(L, 1);
    Vector3* b = checkVector3(L, 2);
    pushVector3(L, *a + *b);
    return 1;
}

int l_vector3_subtract(lua_State* L) {
    Vector3* a = checkVector3(L, 1);
    Vector3* b = checkVector3(L, 2);
    pushVector3(L, *a - *b);
    return 1;
}

int l_vector3_multiply(lua_State* L) {
    Vector3* a = checkVector3(L, 1);
    if (lua_isnumber(L, 2)) {
        float scalar = static_cast<float>(lua_tonumber(L, 2));
        pushVector3(L, *a * scalar);
    } else {
        Vector3* b = checkVector3(L, 2);
        pushVector3(L, *a * *b);
    }
    return 1;
}

int l_vector3_divide(lua_State* L) {
    Vector3* a = checkVector3(L, 1);
    if (lua_isnumber(L, 2)) {
        float scalar = static_cast<float>(lua_tonumber(L, 2));
        if (scalar == 0) luaL_error(L, "Division by zero");
        pushVector3(L, *a / scalar);
    } else {
        Vector3* b = checkVector3(L, 2);
        if (b->x == 0 || b->y == 0 || b->z == 0) luaL_error(L, "Division by zero");
        pushVector3(L, *a / *b);
    }
    return 1;
}

int l_vector3_magnitude(lua_State* L) {
    Vector3* vec = checkVector3(L, 1);
    lua_pushnumber(L, vec->magnitude());
    return 1;
}

int l_vector3_normalize(lua_State* L) {
    Vector3* vec = checkVector3(L, 1);
    pushVector3(L, vec->normalized());
    return 1;
}

int l_vector3_dot(lua_State* L) {
    Vector3* a = checkVector3(L, 1);
    Vector3* b = checkVector3(L, 2);
    lua_pushnumber(L, a->dot(*b));
    return 1;
}

int l_vector3_lerp(lua_State* L) {
    Vector3* a = checkVector3(L, 1);
    Vector3* b = checkVector3(L, 2);
    float t = static_cast<float>(luaL_checknumber(L, 3));
    pushVector3(L, a->lerp(*b, t));
    return 1;
}

int l_vector3_distance(lua_State* L) {
    Vector3* a = checkVector3(L, 1);
    Vector3* b = checkVector3(L, 2);
    lua_pushnumber(L, a->distance(*b));
    return 1;
}

int l_vector3_angle(lua_State* L) {
    Vector3* a = checkVector3(L, 1);
    Vector3* b = checkVector3(L, 2);
    lua_pushnumber(L, a->angle(*b));
    return 1;
}

int l_vector3_tostring(lua_State* L) {
    Vector3* vec = checkVector3(L, 1);
    lua_pushstring(L, vec->toString().c_str());
    return 1;
}

int l_vector3_tween(lua_State* L) {
    Vector3* self = checkVector3(L, 1);
    Vector3* target = checkVector3(L, 2);
    float t = static_cast<float>(luaL_checknumber(L, 3));
    const char* direction = luaL_checkstring(L, 4);
    const char* style = luaL_checkstring(L, 5);
    pushVector3(L, self->tween(*target, t, direction, style));
    return 1;
}