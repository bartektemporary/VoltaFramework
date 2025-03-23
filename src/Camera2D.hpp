#ifndef CAMERA2D_HPP
#define CAMERA2D_HPP

#include "VoltaFramework.hpp"
#include "Matrix.hpp"

class Camera2D {
public:
    Camera2D();
    Camera2D(const Vector2& position, float zoom = 1.0f, float rotation = 0.0f);

    // Getters
    Vector2 getPosition() const { return position; }
    float getZoom() const { return zoom; }
    float getRotation() const { return rotation; }
    Matrix4 getViewMatrix() const;

    // Setters
    void setPosition(const Vector2& newPosition); // Already accepts Vector2
    void setZoom(float newZoom);
    void setRotation(float newRotation);

    // Movement methods
    void move(const Vector2& offset); // Already accepts Vector2
    void zoomBy(float factor);
    void rotateBy(float degrees);

private:
    Vector2 position; // Already using Vector2
    float zoom;
    float rotation; // In degrees

    void updateViewMatrix();
    Matrix4 viewMatrix;
};

// Lua function declarations
int l_camera2d_new(lua_State* L);
int l_camera2d_getPosition(lua_State* L);
int l_camera2d_setPosition(lua_State* L); // Update to handle Vector2
int l_camera2d_getZoom(lua_State* L);
int l_camera2d_setZoom(lua_State* L);
int l_camera2d_getRotation(lua_State* L);
int l_camera2d_setRotation(lua_State* L);
int l_camera2d_move(lua_State* L); // Update to handle Vector2
int l_camera2d_zoomBy(lua_State* L);
int l_camera2d_rotateBy(lua_State* L);
int l_camera2d_tostring(lua_State* L);

Camera2D* checkCamera2D(lua_State* L, int index);

#endif // CAMERA2D_HPP