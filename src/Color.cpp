#include "Color.hpp"
#include <cmath>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <iostream>

Color* checkColor(lua_State* L, int index) {
    return static_cast<Color*>(luaL_checkudata(L, index, "Color"));
}

// Constructor helper functions
Color Color::create(float r, float g, float b) {
    return Color(r, g, b);
}

Color Color::fromRGB(float r, float g, float b) {
    return Color(r / 255.0f, g / 255.0f, b / 255.0f);
}

Color Color::fromHSV(float h, float s, float v) {
    float r = 0, g = 0, b = 0;
    h = fmodf(h, 360.0f);
    if (h < 0) h += 360.0f;
    s = fminf(fmaxf(s, 0.0f), 1.0f);
    v = fminf(fmaxf(v, 0.0f), 1.0f);

    int i = static_cast<int>(h / 60.0f) % 6;
    float f = (h / 60.0f) - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - f * s);
    float t = v * (1.0f - (1.0f - f) * s);

    switch (i) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }
    return Color(r, g, b);
}

Color Color::fromHex(const std::string& hex) {
    std::string h = hex;
    if (h[0] == '#') h = h.substr(1);
    if (h.length() != 6) return Color(0, 0, 0); // Invalid hex, return black

    unsigned int value;
    std::stringstream ss;
    ss << std::hex << h;
    ss >> value;

    float r = ((value >> 16) & 0xFF) / 255.0f;
    float g = ((value >> 8) & 0xFF) / 255.0f;
    float b = (value & 0xFF) / 255.0f;
    return Color(r, g, b);
}

std::string Color::toHex() const {
    std::stringstream ss;
    ss << "#"
       << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(r * 255)
       << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(g * 255)
       << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b * 255);
    return ss.str();
}

void Color::toHSV(float& h, float& s, float& v) const {
    float min = fminf(fminf(r, g), b);
    float max = fmaxf(fmaxf(r, g), b);
    v = max;
    
    float delta = max - min;
    if (max == 0 || delta == 0) {
        s = 0;
        h = 0;
    } else {
        s = delta / max;
        
        if (r == max) {
            h = (g - b) / delta;
        } else if (g == max) {
            h = 2 + (b - r) / delta;
        } else {
            h = 4 + (r - g) / delta;
        }
        h *= 60;
        if (h < 0) h += 360;
    }
}

void Color::toRGB(float& r, float& g, float& b) const {
    r = this->r * 255;
    g = this->g * 255;
    b = this->b * 255;
}

Color Color::lerp(const Color& target, float alpha) const {
    alpha = fminf(fmaxf(alpha, 0.0f), 1.0f);
    float invAlpha = 1.0f - alpha;
    return Color(
        r * invAlpha + target.r * alpha,
        g * invAlpha + target.g * alpha,
        b * invAlpha + target.b * alpha
    );
}

float Color::easeLinear(float t) const {
    return t;
}

float Color::easeSine(float t, const std::string& direction) const {
    if (direction == "in") return 1.0f - cosf(t * M_PI * 0.5f);
    if (direction == "out") return sinf(t * M_PI * 0.5f);
    return -0.5f * (cosf(M_PI * t) - 1.0f); // inout
}

float Color::easeQuad(float t, const std::string& direction) const {
    if (direction == "in") return t * t;
    if (direction == "out") return t * (2.0f - t);
    t = t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
    return t;
}

float Color::easeBack(float t, const std::string& direction) const {
    const float s = 1.70158f;
    float result;
    if (direction == "in") {
        result = t * t * ((s + 1.0f) * t - s);
    } else if (direction == "out") {
        t -= 1.0f;
        result = t * t * ((s + 1.0f) * t + s) + 1.0f;
    } else {
        t *= 2.0f;
        const float s2 = s * 1.525f;
        if (t < 1.0f) {
            result = 0.5f * (t * t * ((s2 + 1.0f) * t - s2));
        } else {
            t -= 2.0f;
            result = 0.5f * (t * t * ((s2 + 1.0f) * t + s2) + 2.0f);
        }
    }
    return result;
}

float Color::easeElastic(float t, const std::string& direction) const {
    const float p = 0.3f;
    const float a = 1.0f;
    const float s = p / 4.0f;
    if (t == 0) return 0.0f;
    if (t == 1) return 1.0f;
    float result;
    if (direction == "in") {
        t -= 1.0f;
        result = -(a * powf(2.0f, 10.0f * t) * sinf((t - s) * (2.0f * M_PI) / p));
    } else if (direction == "out") {
        result = a * powf(2.0f, -10.0f * t) * sinf((t - s) * (2.0f * M_PI) / p) + 1.0f;
    } else {
        t *= 2.0f;
        if (t < 1.0f) {
            t -= 1.0f;
            result = -0.5f * (a * powf(2.0f, 10.0f * t) * sinf((t - s) * (2.0f * M_PI) / p));
        } else {
            t -= 1.0f;
            result = 0.5f * (a * powf(2.0f, -10.0f * t) * sinf((t - s) * (2.0f * M_PI) / p)) + 1.0f;
        }
    }
    return result;
}

float Color::easeBounce(float t, const std::string& direction) const {
    auto bounce = [](float t) {
        if (t < 1.0f / 2.75f) return 7.5625f * t * t;
        if (t < 2.0f / 2.75f) {
            t -= 1.5f / 2.75f;
            return 7.5625f * t * t + 0.75f;
        }
        if (t < 2.5f / 2.75f) {
            t -= 2.25f / 2.75f;
            return 7.5625f * t * t + 0.9375f;
        }
        t -= 2.625f / 2.75f;
        return 7.5625f * t * t + 0.984375f;
    };
    
    if (direction == "in") return 1.0f - bounce(1.0f - t);
    if (direction == "out") return bounce(t);
    t = t < 0.5f ? (1.0f - bounce(1.0f - 2.0f * t)) * 0.5f : bounce(2.0f * t - 1.0f) * 0.5f + 0.5f;
    return t;
}

Color Color::tween(const Color& target, float alpha, const std::string& direction, const std::string& style) const {
    alpha = fminf(fmaxf(alpha, 0.0f), 1.0f);
    float t;
    if (style == "linear") t = easeLinear(alpha);
    else if (style == "sine") t = easeSine(alpha, direction);
    else if (style == "quad") t = easeQuad(alpha, direction);
    else if (style == "back") t = easeBack(alpha, direction);
    else if (style == "elastic") t = easeElastic(alpha, direction);
    else if (style == "bounce") t = easeBounce(alpha, direction);
    else t = easeLinear(alpha);
    
    // Calculate raw values without immediate clamping
    float raw_r = r + (target.r - r) * t;
    float raw_g = g + (target.g - g) * t;
    float raw_b = b + (target.b - b) * t;
    
    // Apply clamping only for final output if needed
    // For now, let's allow overshoot to see the effect
    return Color(raw_r, raw_g, raw_b);
}

// Lua functions (unchanged)
int l_color_create(lua_State* L) {
    float r = static_cast<float>(luaL_checknumber(L, 1));
    float g = static_cast<float>(luaL_checknumber(L, 2));
    float b = static_cast<float>(luaL_checknumber(L, 3));

    Color* color = static_cast<Color*>(lua_newuserdata(L, sizeof(Color)));
    *color = Color::create(r, g, b);

    luaL_setmetatable(L, "Color");
    return 1;
}

int l_color_fromRGB(lua_State* L) {
    float r = static_cast<float>(luaL_checknumber(L, 1));
    float g = static_cast<float>(luaL_checknumber(L, 2));
    float b = static_cast<float>(luaL_checknumber(L, 3));

    Color* color = static_cast<Color*>(lua_newuserdata(L, sizeof(Color)));
    *color = Color::fromRGB(r, g, b);

    luaL_setmetatable(L, "Color");
    return 1;
}

int l_color_newHSV(lua_State* L) {
    float h = static_cast<float>(luaL_checknumber(L, 1));
    float s = static_cast<float>(luaL_checknumber(L, 2));
    float v = static_cast<float>(luaL_checknumber(L, 3));

    Color* color = static_cast<Color*>(lua_newuserdata(L, sizeof(Color)));
    *color = Color::fromHSV(h, s, v);

    luaL_setmetatable(L, "Color");
    return 1;
}

int l_color_newHex(lua_State* L) {
    const char* hex = luaL_checkstring(L, 1);

    Color* color = static_cast<Color*>(lua_newuserdata(L, sizeof(Color)));
    *color = Color::fromHex(hex);

    luaL_setmetatable(L, "Color");
    return 1;
}

int l_color_tostring(lua_State* L) {
    Color* color = checkColor(L, 1);
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Color(%.2f, %.2f, %.2f)", color->r, color->g, color->b);
    lua_pushstring(L, buffer);
    return 1;
}

int l_color_toHex(lua_State* L) {
    Color* color = checkColor(L, 1);
    lua_pushstring(L, color->toHex().c_str());
    return 1;
}

int l_color_toHSV(lua_State* L) {
    Color* color = checkColor(L, 1);
    float h, s, v;
    color->toHSV(h, s, v);
    lua_pushnumber(L, h);
    lua_pushnumber(L, s);
    lua_pushnumber(L, v);
    return 3;
}

int l_color_toRGB(lua_State* L) {
    Color* color = checkColor(L, 1);
    float r, g, b;
    color->toRGB(r, g, b);
    lua_pushnumber(L, r);
    lua_pushnumber(L, g);
    lua_pushnumber(L, b);
    return 3;
}

int l_color_lerp(lua_State* L) {
    Color* color = checkColor(L, 1);
    Color* target = checkColor(L, 2);
    float alpha = static_cast<float>(luaL_checknumber(L, 3));
    
    Color* result = static_cast<Color*>(lua_newuserdata(L, sizeof(Color)));
    *result = color->lerp(*target, alpha);
    luaL_setmetatable(L, "Color");
    return 1;
}

int l_color_tween(lua_State* L) {
    Color* color = checkColor(L, 1);
    Color* target = checkColor(L, 2);
    float alpha = static_cast<float>(luaL_checknumber(L, 3));
    const char* direction = luaL_checkstring(L, 4);
    const char* style = luaL_checkstring(L, 5);
    
    Color* result = static_cast<Color*>(lua_newuserdata(L, sizeof(Color)));
    *result = color->tween(*target, alpha, direction, style);
    luaL_setmetatable(L, "Color");
    return 1;
}