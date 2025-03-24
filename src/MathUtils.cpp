#include "VoltaFramework.hpp"

#include "Tweens.hpp"

#define _USE_MATH_DEFINES
#include <cmath>

static float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

static float grad(int hash, float x, float y, float z) {
    int h {hash & 15};
    float u {h < 8 ? x : y};
    float v {h < 4 ? y : h == 12 || h == 14 ? x : z};
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

static const unsigned char p[] {
    151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
    8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
    35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
    134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
    55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,
    18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,
    250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
    189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,
    172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,
    228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
    107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

float VoltaFramework::lerp(float a, float b, float t) const {
    return a + t * (b - a);
}

float VoltaFramework::noise1d(float x) const {
    int X = static_cast<int>(std::floor(x)) & 255;
    x -= std::floor(x);
    float u = fade(x);
    int a = p[X];
    int b = p[(X + 1) & 255];
    float result = lerp(grad(a, x, 0, 0), grad(b, x - 1, 0, 0), u);
    return result * 0.5f + 0.5f; // Normalize to [0, 1]
}

float VoltaFramework::noise2d(float x, float y) const {
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;
    x -= std::floor(x);
    y -= std::floor(y);
    float u = fade(x);
    float v = fade(y);
    int A = p[X] + Y;
    int AA = p[A & 255];
    int AB = p[(A + 1) & 255];
    int B = p[(X + 1) & 255] + Y;
    int BA = p[B & 255];
    int BB = p[(B + 1) & 255];
    float result = lerp(
        lerp(grad(AA, x, y, 0), grad(BA, x - 1, y, 0), u),
        lerp(grad(AB, x, y - 1, 0), grad(BB, x - 1, y - 1, 0), u),
        v
    );
    return result * 0.5f + 0.5f; // Normalize to [0, 1]
}

float VoltaFramework::noise3d(float x, float y, float z) const {
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;
    int Z = static_cast<int>(std::floor(z)) & 255;
    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);
    float u = fade(x);
    float v = fade(y);
    float w = fade(z);
    int A = p[X] + Y;
    int AA = p[A & 255] + Z;
    int AB = p[(A + 1) & 255] + Z;
    int B = p[(X + 1) & 255] + Y;
    int BA = p[B & 255] + Z;
    int BB = p[(B + 1) & 255] + Z;
    float result = lerp(
        lerp(
            lerp(grad(p[AA & 255], x, y, z), grad(p[BA & 255], x - 1, y, z), u),
            lerp(grad(p[AB & 255], x, y - 1, z), grad(p[BB & 255], x - 1, y - 1, z), u),
            v
        ),
        lerp(
            lerp(grad(p[(AA + 1) & 255], x, y, z - 1), grad(p[(BA + 1) & 255], x - 1, y, z - 1), u),
            lerp(grad(p[(AB + 1) & 255], x, y - 1, z - 1), grad(p[(BB + 1) & 255], x - 1, y - 1, z - 1), u),
            v
        ),
        w
    );
    return result * 0.5f + 0.5f; // Normalize to [0, 1]
}

float VoltaFramework::tween(float start, float end, float t, const std::string& direction, const std::string& style) const {
    t = std::fmin(std::fmax(t, 0.0f), 1.0f);

    float eased = 0.0f;
    if (style == "linear") {
        eased = easeLinear(t);
    } else if (style == "sine") {
        eased = easeSine(t);
    } else if (style == "quad") {
        eased = easeQuad(t);
    } else if (style == "cubic") {
        eased = easeCubic(t);
    } else if (style == "quart") {
        eased = easeQuart(t);
    } else if (style == "quint") {
        eased = easeQuint(t);
    } else if (style == "exponential") {
        eased = easeExponential(t);
    } else if (style == "circular") {
        eased = easeCircular(t);
    } else if (style == "back") {
        eased = easeBack(t);
    } else if (style == "bounce") {
        eased = easeBounce(t);
    } else if (style == "elastic") {
        eased = easeElastic(t);
    } else {
        throw std::invalid_argument("Invalid easing style: " + style);
    }

    eased = applyEasingDirection(t, eased, direction.c_str());
    return start + (end - start) * eased;
}

int l_math_clamp(lua_State* L) {
    double n {luaL_checknumber(L, 1)};
    double minValue {luaL_checknumber(L, 2)};
    double maxValue {luaL_checknumber(L, 3)};
    double result {fmin(fmax(n, minValue), maxValue)};
    lua_pushnumber(L, result);
    return 1;
}

int l_math_round(lua_State* L) {
    double n {luaL_checknumber(L, 1)};
    lua_Integer i {luaL_optinteger(L, 2, 0)};
    double m {pow(10.0, static_cast<double>(i))};
    double result {floor(n * m + 0.5) / m};
    lua_pushnumber(L, result);
    return 1;
}

int l_math_lerp(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        luaL_error(L, "Framework instance not found");
        return 0;
    }
    lua_Number a = luaL_checknumber(L, 1);
    lua_Number b = luaL_checknumber(L, 2);
    lua_Number t = luaL_checknumber(L, 3);
    lua_pushnumber(L, framework->lerp(static_cast<float>(a), static_cast<float>(b), static_cast<float>(t)));
    return 1;
}

int l_math_noise1d(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        luaL_error(L, "Framework instance not found");
        return 0;
    }
    lua_Number x = luaL_checknumber(L, 1);
    lua_pushnumber(L, framework->noise1d(static_cast<float>(x)));
    return 1;
}

int l_math_noise2d(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        luaL_error(L, "Framework instance not found");
        return 0;
    }
    lua_Number x = luaL_checknumber(L, 1);
    lua_Number y = luaL_checknumber(L, 2);
    lua_pushnumber(L, framework->noise2d(static_cast<float>(x), static_cast<float>(y)));
    return 1;
}

int l_math_noise3d(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        luaL_error(L, "Framework instance not found");
        return 0;
    }
    lua_Number x = luaL_checknumber(L, 1);
    lua_Number y = luaL_checknumber(L, 2);
    lua_Number z = luaL_checknumber(L, 3);
    lua_pushnumber(L, framework->noise3d(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)));
    return 1;
}

int l_math_tween(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        luaL_error(L, "Framework instance not found");
        return 0;
    }
    lua_Number start = luaL_checknumber(L, 1);
    lua_Number end = luaL_checknumber(L, 2);
    lua_Number t = luaL_checknumber(L, 3);
    const char* direction = luaL_checkstring(L, 4);
    const char* style = luaL_checkstring(L, 5);
    try {
        float result = framework->tween(
            static_cast<float>(start),
            static_cast<float>(end),
            static_cast<float>(t),
            direction,
            style
        );
        lua_pushnumber(L, result);
        return 1;
    } catch (const std::invalid_argument& e) {
        luaL_error(L, "Tween error: %s", e.what());
        return 0;
    }
}