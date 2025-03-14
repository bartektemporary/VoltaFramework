#include "VoltaFramework.hpp"
#include "Tweens.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

// Helper function to check and get Vector2 from Lua
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

int l_vector2_tween(lua_State* L) {
    Vector2* self = checkVector2(L, 1);
    Vector2* target = checkVector2(L, 2);
    lua_Number t = luaL_checknumber(L, 3);
    const char* direction = luaL_checkstring(L, 4);
    const char* style = luaL_checkstring(L, 5);

    t = fmin(fmax(t, 0.0), 1.0);

    float eased = 0.0f;
    if (strcmp(style, "linear") == 0) {
        eased = easeLinear(static_cast<float>(t));
    } else if (strcmp(style, "sine") == 0) {
        eased = easeSine(static_cast<float>(t));
    } else if (strcmp(style, "quad") == 0) {
        eased = easeQuad(static_cast<float>(t));
    } else if (strcmp(style, "cubic") == 0) {
        eased = easeCubic(static_cast<float>(t));
    } else if (strcmp(style, "quart") == 0) {
        eased = easeQuart(static_cast<float>(t));
    } else if (strcmp(style, "quint") == 0) {
        eased = easeQuint(static_cast<float>(t));
    } else if (strcmp(style, "exponential") == 0) {
        eased = easeExponential(static_cast<float>(t));
    } else if (strcmp(style, "circular") == 0) {
        eased = easeCircular(static_cast<float>(t));
    } else if (strcmp(style, "back") == 0) {
        eased = easeBack(static_cast<float>(t));
    } else if (strcmp(style, "bounce") == 0) {
        eased = easeBounce(static_cast<float>(t));
    } else if (strcmp(style, "elastic") == 0) {
        eased = easeElastic(static_cast<float>(t));
    } else {
        luaL_error(L, "Invalid easing style: %s", style);
        return 0;
    }

    eased = applyEasingDirection(static_cast<float>(t), eased, direction);

    Vector2* result = static_cast<Vector2*>(lua_newuserdata(L, sizeof(Vector2)));
    result->x = self->x + (target->x - self->x) * eased;
    result->y = self->y + (target->y - self->y) * eased;
    
    luaL_getmetatable(L, "Vector2");
    lua_setmetatable(L, -2);
    return 1;
}