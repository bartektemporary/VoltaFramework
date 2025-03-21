#ifndef COLOR_H
#define COLOR_H

#include "VoltaFramework.hpp"

struct Color {
    float r, g, b;

    Color(float r_ = 0.0f, float g_ = 0.0f, float b_ = 0.0f) 
        : r(fminf(fmaxf(r_, 0.0f), 1.0f)), 
          g(fminf(fmaxf(g_, 0.0f), 1.0f)), 
          b(fminf(fmaxf(b_, 0.0f), 1.0f)) {}

    static Color create(float r, float g, float b);
    static Color fromRGB(float r, float g, float b);
    static Color fromHSV(float h, float s, float v);
    static Color fromHex(const std::string& hex);

    std::string toHex() const;
    void toHSV(float& h, float& s, float& v) const;
    void toRGB(float& r, float& g, float& b) const;
    Color lerp(const Color& target, float alpha) const;
    Color tween(const Color& target, float alpha, const std::string& direction, const std::string& style) const;

private:
    float easeLinear(float t) const;
    float easeSine(float t, const std::string& direction) const;
    float easeQuad(float t, const std::string& direction) const;
    float easeBack(float t, const std::string& direction) const;
    float easeElastic(float t, const std::string& direction) const;
    float easeBounce(float t, const std::string& direction) const;
};

Color* checkColor(lua_State* L, int index);

int l_color_create(lua_State* L);
int l_color_fromRGB(lua_State* L);
int l_color_newHSV(lua_State* L);
int l_color_newHex(lua_State* L);
int l_color_tostring(lua_State* L);

int l_color_toHex(lua_State* L);
int l_color_toHSV(lua_State* L);
int l_color_toRGB(lua_State* L);
int l_color_lerp(lua_State* L);
int l_color_tween(lua_State* L);

#endif // COLOR_H