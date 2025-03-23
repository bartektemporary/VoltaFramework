#include "Vector2.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

Vector2* checkVector2(lua_State* L, int index) {
    return static_cast<Vector2*>(luaL_checkudata(L, index, "Vector2"));
}

int l_vector2_new(lua_State* L) {
    float x = static_cast<float>(luaL_checknumber(L, 1));
    float y = static_cast<float>(luaL_checknumber(L, 2));
    
    Vector2* vec = static_cast<Vector2*>(lua_newuserdata(L, sizeof(Vector2)));
    vec->x = x;
    vec->y = y;
    
    luaL_getmetatable(L, "Vector2");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        luaL_error(L, "Vector2 metatable not found");
        return 0;
    }
    lua_setmetatable(L, -2);

    return 1;
}

int l_vector2_add(lua_State* L) {
    Vector2* a = checkVector2(L, 1);
    Vector2* b = checkVector2(L, 2);
    
    Vector2* result = static_cast<Vector2*>(lua_newuserdata(L, sizeof(Vector2)));
    result->x = a->x + b->x;
    result->y = a->y + b->y;
    
    luaL_getmetatable(L, "Vector2");
    lua_setmetatable(L, -2);
    return 1;
}

int l_vector2_subtract(lua_State* L) {
    Vector2* a = checkVector2(L, 1);
    Vector2* b = checkVector2(L, 2);
    
    Vector2* result = static_cast<Vector2*>(lua_newuserdata(L, sizeof(Vector2)));
    result->x = a->x - b->x;
    result->y = a->y - b->y;
    
    luaL_getmetatable(L, "Vector2");
    lua_setmetatable(L, -2);
    return 1;
}

int l_vector2_multiply(lua_State* L) {
    Vector2* a = checkVector2(L, 1);
    if (lua_isnumber(L, 2)) {
        float scalar = static_cast<float>(lua_tonumber(L, 2));
        Vector2* result = static_cast<Vector2*>(lua_newuserdata(L, sizeof(Vector2)));
        result->x = a->x * scalar;
        result->y = a->y * scalar;
        luaL_getmetatable(L, "Vector2");
        lua_setmetatable(L, -2);
        return 1;
    }
    Vector2* b = checkVector2(L, 2);
    Vector2* result = static_cast<Vector2*>(lua_newuserdata(L, sizeof(Vector2)));
    result->x = a->x * b->x;
    result->y = a->y * b->y;
    
    luaL_getmetatable(L, "Vector2");
    lua_setmetatable(L, -2);
    return 1;
}

int l_vector2_divide(lua_State* L) {
    Vector2* a = checkVector2(L, 1);
    if (lua_isnumber(L, 2)) {
        float scalar = static_cast<float>(lua_tonumber(L, 2));
        if (scalar == 0) luaL_error(L, "Division by zero");
        Vector2* result = static_cast<Vector2*>(lua_newuserdata(L, sizeof(Vector2)));
        result->x = a->x / scalar;
        result->y = a->y / scalar;
        luaL_getmetatable(L, "Vector2");
        lua_setmetatable(L, -2);
        return 1;
    }
    Vector2* b = checkVector2(L, 2);
    if (b->x == 0 || b->y == 0) luaL_error(L, "Division by zero");
    Vector2* result = static_cast<Vector2*>(lua_newuserdata(L, sizeof(Vector2)));
    result->x = a->x / b->x;
    result->y = a->y / b->y;
    
    luaL_getmetatable(L, "Vector2");
    lua_setmetatable(L, -2);
    return 1;
}

int l_vector2_magnitude(lua_State* L) {
    Vector2* vec = checkVector2(L, 1);
    float mag = sqrt(vec->x * vec->x + vec->y * vec->y);
    lua_pushnumber(L, mag);
    return 1;
}

int l_vector2_normalize(lua_State* L) {
    Vector2* vec = checkVector2(L, 1);
    float mag = sqrt(vec->x * vec->x + vec->y * vec->y);
    if (mag == 0) {
        lua_pushnil(L);
        return 1;
    }
    Vector2* result = static_cast<Vector2*>(lua_newuserdata(L, sizeof(Vector2)));
    result->x = vec->x / mag;
    result->y = vec->y / mag;
    
    luaL_getmetatable(L, "Vector2");
    lua_setmetatable(L, -2);
    return 1;
}

int l_vector2_dot(lua_State* L) {
    Vector2* a = checkVector2(L, 1);
    Vector2* b = checkVector2(L, 2);
    float dot = a->x * b->x + a->y * b->y;
    lua_pushnumber(L, dot);
    return 1;
}

int l_vector2_lerp(lua_State* L) {
    Vector2* a = checkVector2(L, 1);
    Vector2* b = checkVector2(L, 2);
    float t = static_cast<float>(luaL_checknumber(L, 3));
    
    Vector2* result = static_cast<Vector2*>(lua_newuserdata(L, sizeof(Vector2)));
    result->x = a->x + (b->x - a->x) * t;
    result->y = a->y + (b->y - a->y) * t;
    
    luaL_getmetatable(L, "Vector2");
    lua_setmetatable(L, -2);
    return 1;
}

int l_vector2_distance(lua_State* L) {
    Vector2* a = checkVector2(L, 1);
    Vector2* b = checkVector2(L, 2);
    float dx = b->x - a->x;
    float dy = b->y - a->y;
    float dist = sqrt(dx * dx + dy * dy);
    lua_pushnumber(L, dist);
    return 1;
}

int l_vector2_angle(lua_State* L) {
    Vector2* a = checkVector2(L, 1);
    Vector2* b = checkVector2(L, 2);
    float dot = a->x * b->x + a->y * b->y;
    float magA = sqrt(a->x * a->x + a->y * a->y);
    float magB = sqrt(b->x * b->x + b->y * b->y);
    if (magA == 0 || magB == 0) {
        lua_pushnumber(L, 0);
        return 1;
    }
    float cosTheta = dot / (magA * magB);
    if (cosTheta > 1.0f) cosTheta = 1.0f;
    if (cosTheta < -1.0f) cosTheta = -1.0f;
    float angle = acos(cosTheta) * 180.0f / M_PI;
    lua_pushnumber(L, angle);
    return 1;
}

int l_vector2_tostring(lua_State* L) {
    Vector2* vec = checkVector2(L, 1);
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Vector2(%.2f, %.2f)", vec->x, vec->y);
    lua_pushstring(L, buffer);
    return 1;
}

// Easing functions
static float easeLinear(float t) {
    return t;
}

static float easeSineIn(float t) {
    return 1.0f - std::cos(t * M_PI / 2.0f);
}

static float easeSineOut(float t) {
    return std::sin(t * M_PI / 2.0f);
}

static float easeSineInOut(float t) {
    return -0.5f * (std::cos(M_PI * t) - 1.0f);
}

static float easeQuadIn(float t) {
    return t * t;
}

static float easeQuadOut(float t) {
    return t * (2.0f - t);
}

static float easeQuadInOut(float t) {
    if (t < 0.5f) return 2.0f * t * t;
    return -1.0f + (4.0f - 2.0f * t) * t;
}

static float easeCubicIn(float t) {
    return t * t * t;
}

static float easeCubicOut(float t) {
    float t1 = t - 1.0f;
    return t1 * t1 * t1 + 1.0f;
}

static float easeCubicInOut(float t) {
    if (t < 0.5f) return 4.0f * t * t * t;
    float t1 = t - 1.0f;
    return 4.0f * t1 * t1 * t1 + 1.0f;
}

static float easeQuartIn(float t) {
    return t * t * t * t;
}

static float easeQuartOut(float t) {
    float t1 = t - 1.0f;
    return 1.0f - t1 * t1 * t1 * t1;
}

static float easeQuartInOut(float t) {
    if (t < 0.5f) return 8.0f * t * t * t * t;
    float t1 = t - 1.0f;
    return 1.0f - 8.0f * t1 * t1 * t1 * t1;
}

static float easeQuintIn(float t) {
    return t * t * t * t * t;
}

static float easeQuintOut(float t) {
    float t1 = t - 1.0f;
    return 1.0f + t1 * t1 * t1 * t1 * t1;
}

static float easeQuintInOut(float t) {
    if (t < 0.5f) return 16.0f * t * t * t * t * t;
    float t1 = t - 1.0f;
    return 1.0f + 16.0f * t1 * t1 * t1 * t1 * t1;
}

static float easeExponentialIn(float t) {
    if (t == 0.0f) return 0.0f;
    return std::pow(2.0f, 10.0f * (t - 1.0f));
}

static float easeExponentialOut(float t) {
    if (t == 1.0f) return 1.0f;
    return 1.0f - std::pow(2.0f, -10.0f * t);
}

static float easeExponentialInOut(float t) {
    if (t == 0.0f) return 0.0f;
    if (t == 1.0f) return 1.0f;
    if (t < 0.5f) return 0.5f * std::pow(2.0f, 10.0f * (2.0f * t - 1.0f));
    return 0.5f * (2.0f - std::pow(2.0f, -10.0f * (2.0f * t - 1.0f)));
}

static float easeCircularIn(float t) {
    return 1.0f - std::sqrt(1.0f - t * t);
}

static float easeCircularOut(float t) {
    float t1 = t - 1.0f;
    return std::sqrt(1.0f - t1 * t1);
}

static float easeCircularInOut(float t) {
    if (t < 0.5f) return 0.5f * (1.0f - std::sqrt(1.0f - 4.0f * t * t));
    float t1 = 2.0f * t - 2.0f;
    return 0.5f * (std::sqrt(1.0f - t1 * t1) + 1.0f);
}

static float easeBackIn(float t) {
    const float c1 = 1.70158f;
    return t * t * ((c1 + 1.0f) * t - c1);
}

static float easeBackOut(float t) {
    const float c1 = 1.70158f;
    float t1 = t - 1.0f;
    return 1.0f + t1 * t1 * ((c1 + 1.0f) * t1 + c1);
}

static float easeBackInOut(float t) {
    const float c1 = 1.70158f;
    const float c2 = c1 * 1.525f;
    if (t < 0.5f) return 0.5f * (4.0f * t * t * ((c2 + 1.0f) * 2.0f * t - c2));
    float t1 = 2.0f * t - 2.0f;
    return 0.5f * (t1 * t1 * ((c2 + 1.0f) * t1 + c2) + 2.0f);
}

static float easeBounceOut(float t) {
    if (t < 1.0f / 2.75f) {
        return 7.5625f * t * t;
    } else if (t < 2.0f / 2.75f) {
        float t1 = t - 1.5f / 2.75f;
        return 7.5625f * t1 * t1 + 0.75f;
    } else if (t < 2.5f / 2.75f) {
        float t1 = t - 2.25f / 2.75f;
        return 7.5625f * t1 * t1 + 0.9375f;
    } else {
        float t1 = t - 2.625f / 2.75f;
        return 7.5625f * t1 * t1 + 0.984375f;
    }
}

static float easeBounceIn(float t) {
    return 1.0f - easeBounceOut(1.0f - t);
}

static float easeBounceInOut(float t) {
    if (t < 0.5f) return 0.5f * easeBounceIn(2.0f * t);
    return 0.5f * easeBounceOut(2.0f * t - 1.0f) + 0.5f;
}

static float easeElasticIn(float t) {
    if (t == 0.0f) return 0.0f;
    if (t == 1.0f) return 1.0f;
    const float p = 0.3f;
    const float a = 1.0f;
    return -a * std::pow(2.0f, 10.0f * (t - 1.0f)) * std::sin((t - 1.0f - p / (2.0f * M_PI) * std::asin(1.0f / a)) * (2.0f * M_PI) / p);
}

static float easeElasticOut(float t) {
    if (t == 0.0f) return 0.0f;
    if (t == 1.0f) return 1.0f;
    const float p = 0.3f;
    const float a = 1.0f;
    return a * std::pow(2.0f, -10.0f * t) * std::sin((t - p / (2.0f * M_PI) * std::asin(1.0f / a)) * (2.0f * M_PI) / p) + 1.0f;
}

static float easeElasticInOut(float t) {
    if (t == 0.0f) return 0.0f;
    if (t == 1.0f) return 1.0f;
    const float p = 0.3f * 1.5f;
    const float a = 1.0f;
    float t_adj = t * 2.0f;
    if (t_adj < 1.0f) {
        return -0.5f * a * std::pow(2.0f, 10.0f * (t_adj - 1.0f)) * std::sin((t_adj - 1.0f - p / (2.0f * M_PI) * std::asin(1.0f / a)) * (2.0f * M_PI) / p);
    } else {
        t_adj -= 1.0f;
        return 0.5f * a * std::pow(2.0f, -10.0f * t_adj) * std::sin((t_adj - p / (2.0f * M_PI) * std::asin(1.0f / a)) * (2.0f * M_PI) / p) + 1.0f;
    }
}

int l_vector2_tween(lua_State* L) {
    Vector2* self = checkVector2(L, 1);
    Vector2* target = checkVector2(L, 2);
    float t = static_cast<float>(luaL_checknumber(L, 3));
    const char* direction = luaL_checkstring(L, 4);
    const char* style = luaL_checkstring(L, 5);

    // Clamp t between 0 and 1
    t = std::max(0.0f, std::min(1.0f, t));

    // Easing function selection based on style and direction
    float easedT = 0.0f;
    if (strcmp(style, "linear") == 0) {
        easedT = easeLinear(t);
    } else if (strcmp(style, "sine") == 0) {
        if (strcmp(direction, "in") == 0) easedT = easeSineIn(t);
        else if (strcmp(direction, "out") == 0) easedT = easeSineOut(t);
        else if (strcmp(direction, "inout") == 0) easedT = easeSineInOut(t);
        else luaL_error(L, "Invalid direction: %s", direction);
    } else if (strcmp(style, "quad") == 0) {
        if (strcmp(direction, "in") == 0) easedT = easeQuadIn(t);
        else if (strcmp(direction, "out") == 0) easedT = easeQuadOut(t);
        else if (strcmp(direction, "inout") == 0) easedT = easeQuadInOut(t);
        else luaL_error(L, "Invalid direction: %s", direction);
    } else if (strcmp(style, "cubic") == 0) {
        if (strcmp(direction, "in") == 0) easedT = easeCubicIn(t);
        else if (strcmp(direction, "out") == 0) easedT = easeCubicOut(t);
        else if (strcmp(direction, "inout") == 0) easedT = easeCubicInOut(t);
        else luaL_error(L, "Invalid direction: %s", direction);
    } else if (strcmp(style, "quart") == 0) {
        if (strcmp(direction, "in") == 0) easedT = easeQuartIn(t);
        else if (strcmp(direction, "out") == 0) easedT = easeQuartOut(t);
        else if (strcmp(direction, "inout") == 0) easedT = easeQuartInOut(t);
        else luaL_error(L, "Invalid direction: %s", direction);
    } else if (strcmp(style, "quint") == 0) {
        if (strcmp(direction, "in") == 0) easedT = easeQuintIn(t);
        else if (strcmp(direction, "out") == 0) easedT = easeQuintOut(t);
        else if (strcmp(direction, "inout") == 0) easedT = easeQuintInOut(t);
        else luaL_error(L, "Invalid direction: %s", direction);
    } else if (strcmp(style, "exponential") == 0) {
        if (strcmp(direction, "in") == 0) easedT = easeExponentialIn(t);
        else if (strcmp(direction, "out") == 0) easedT = easeExponentialOut(t);
        else if (strcmp(direction, "inout") == 0) easedT = easeExponentialInOut(t);
        else luaL_error(L, "Invalid direction: %s", direction);
    } else if (strcmp(style, "circular") == 0) {
        if (strcmp(direction, "in") == 0) easedT = easeCircularIn(t);
        else if (strcmp(direction, "out") == 0) easedT = easeCircularOut(t);
        else if (strcmp(direction, "inout") == 0) easedT = easeCircularInOut(t);
        else luaL_error(L, "Invalid direction: %s", direction);
    } else if (strcmp(style, "back") == 0) {
        if (strcmp(direction, "in") == 0) easedT = easeBackIn(t);
        else if (strcmp(direction, "out") == 0) easedT = easeBackOut(t);
        else if (strcmp(direction, "inout") == 0) easedT = easeBackInOut(t);
        else luaL_error(L, "Invalid direction: %s", direction);
    } else if (strcmp(style, "bounce") == 0) {
        if (strcmp(direction, "in") == 0) easedT = easeBounceIn(t);
        else if (strcmp(direction, "out") == 0) easedT = easeBounceOut(t);
        else if (strcmp(direction, "inout") == 0) easedT = easeBounceInOut(t);
        else luaL_error(L, "Invalid direction: %s", direction);
    } else if (strcmp(style, "elastic") == 0) {
        if (strcmp(direction, "in") == 0) easedT = easeElasticIn(t);
        else if (strcmp(direction, "out") == 0) easedT = easeElasticOut(t);
        else if (strcmp(direction, "inout") == 0) easedT = easeElasticInOut(t);
        else luaL_error(L, "Invalid direction: %s", direction);
    } else {
        luaL_error(L, "Invalid easing style: %s", style);
        return 0;
    }

    // Create result Vector2
    Vector2* result = static_cast<Vector2*>(lua_newuserdata(L, sizeof(Vector2)));
    result->x = self->x + (target->x - self->x) * easedT;
    result->y = self->y + (target->y - self->y) * easedT;
    
    luaL_getmetatable(L, "Vector2");
    lua_setmetatable(L, -2);
    return 1;
}