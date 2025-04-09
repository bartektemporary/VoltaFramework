#include "VoltaFramework.hpp"
#include "Camera3D.hpp"
#include <cmath>
#include <cstring> // For snprintf

#define _USE_MATH_DEFINES
#include <math.h>

// Helper function to push Vector3 to Lua stack (assumed missing)
void pushVector3(lua_State* L, const Vector3& vec) {
    Vector3* result = static_cast<Vector3*>(lua_newuserdata(L, sizeof(Vector3)));
    *result = vec;
    luaL_getmetatable(L, "Vector3");
    lua_setmetatable(L, -2);
}

Camera3D::Camera3D()
    : position(0.0f, 0.0f, 0.0f),
      rotation(0.0f, 0.0f, 0.0f),
      fov(60.0f),
      nearPlane(0.1f),
      farPlane(1000.0f) {
    updateViewMatrix();
}

Camera3D::Camera3D(const Vector3& position, const Vector3& rotation, float fov, float nearPlane, float farPlane)
    : position(position),
      rotation(rotation),
      fov(fov),
      nearPlane(nearPlane),
      farPlane(farPlane) {
    updateViewMatrix();
}

Matrix4 Camera3D::getProjectionMatrix(float aspectRatio) const {
    updateProjectionMatrix(aspectRatio); // Update projection matrix with current aspect ratio
    return projectionMatrix;
}

void Camera3D::setPosition(const Vector3& newPosition) {
    position = newPosition;
    updateViewMatrix();
}

void Camera3D::setRotation(const Vector3& newRotation) {
    rotation = newRotation;
    updateViewMatrix();
}

void Camera3D::setFOV(float newFOV) {
    fov = std::max(1.0f, std::min(179.0f, newFOV)); // Clamp FOV to reasonable range
}

void Camera3D::setNearPlane(float newNear) {
    nearPlane = std::max(0.01f, newNear); // Prevent degenerate values
}

void Camera3D::setFarPlane(float newFar) {
    farPlane = std::max(nearPlane + 0.01f, newFar); // Ensure far > near
}

void Camera3D::move(const Vector3& offset) {
    position = position.add(offset);
    updateViewMatrix();
}

void Camera3D::rotateBy(const Vector3& degrees) {
    rotation = rotation.add(degrees);
    updateViewMatrix();
}

void Camera3D::updateViewMatrix() {
    Matrix4 rotX, rotY, rotZ;
    rotateX(rotX, rotation.x * M_PI / 180.0f);
    rotateY(rotY, rotation.y * M_PI / 180.0f);
    rotateZ(rotZ, rotation.z * M_PI / 180.0f);

    Matrix4 rotationMat = rotY * rotX * rotZ; // Yaw * Pitch * Roll
    Matrix4 translateMat;
    translate(translateMat, Vector3(-position.x, -position.y, -position.z));

    viewMatrix = rotationMat * translateMat;
}

void Camera3D::updateProjectionMatrix(float aspectRatio) const {
    float fovRad = fov * M_PI / 180.0f;
    float tanHalfFOV = std::tan(fovRad / 2.0f);
    float zRange = farPlane - nearPlane;

    projectionMatrix = Matrix4(); // Reset to identity
    // Assuming Matrix4 uses a flat array (m[16]) for OpenGL column-major order
    projectionMatrix.m[0] = 1.0f / (aspectRatio * tanHalfFOV); // m[0][0]
    projectionMatrix.m[5] = 1.0f / tanHalfFOV;                  // m[1][1]
    projectionMatrix.m[10] = -(farPlane + nearPlane) / zRange;  // m[2][2]
    projectionMatrix.m[11] = -1.0f;                             // m[2][3]
    projectionMatrix.m[14] = -(2.0f * farPlane * nearPlane) / zRange; // m[3][2]
    projectionMatrix.m[15] = 0.0f;                              // m[3][3]
}

std::string Camera3D::toString() const {
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "Camera3D(pos: (%.2f, %.2f, %.2f), rot: (%.2f, %.2f, %.2f), fov: %.2f, near: %.2f, far: %.2f)",
             position.x, position.y, position.z,
             rotation.x, rotation.y, rotation.z,
             fov, nearPlane, farPlane);
    return std::string(buffer);
}

void VoltaFramework::setCamera3D(Camera3D* camera) {
    currentCamera3D = camera;
    if (camera) {
        glEnable(GL_DEPTH_TEST);
        glUseProgram(shape3DShaderProgram);
        Matrix4 viewMatrix = camera->getViewMatrix();
        Matrix4 projMatrix = camera->getProjectionMatrix(static_cast<float>(width) / height);
        glUniformMatrix4fv(shape3DViewUniform, 1, GL_FALSE, viewMatrix.m);
        glUniformMatrix4fv(shape3DProjectionUniform, 1, GL_FALSE, projMatrix.m);
        view3D = viewMatrix;
        projection3D = projMatrix;
    } else {
        glDisable(GL_DEPTH_TEST);
        glUseProgram(shape3DShaderProgram);
        glUniformMatrix4fv(shape3DViewUniform, 1, GL_FALSE, Matrix4().m); // Identity
        glUniformMatrix4fv(shape3DProjectionUniform, 1, GL_FALSE, Matrix4().m); // Identity
        view3D = Matrix4();
        projection3D = Matrix4();
    }
}

// Lua bindings
Camera3D* checkCamera3D(lua_State* L, int index) {
    return static_cast<Camera3D*>(luaL_checkudata(L, index, "Camera3D"));
}

int l_camera3d_new(lua_State* L) {
    Vector3* pos = nullptr;
    Vector3* rot = nullptr;
    float x = 0.0f, y = 0.0f, z = 0.0f;
    float rx = 0.0f, ry = 0.0f, rz = 0.0f;
    float fov = static_cast<float>(luaL_optnumber(L, 4, 60.0));
    float near_ = static_cast<float>(luaL_optnumber(L, 5, 0.1));
    float far_ = static_cast<float>(luaL_optnumber(L, 6, 1000.0));

    if (lua_isuserdata(L, 1) && luaL_testudata(L, 1, "Vector3")) {
        pos = static_cast<Vector3*>(luaL_checkudata(L, 1, "Vector3"));
        x = pos->x; y = pos->y; z = pos->z;
    } else {
        x = static_cast<float>(luaL_optnumber(L, 1, 0.0));
        y = static_cast<float>(luaL_optnumber(L, 2, 0.0));
        z = static_cast<float>(luaL_optnumber(L, 3, 0.0));
    }

    if (lua_isuserdata(L, 2) && luaL_testudata(L, 2, "Vector3")) {
        rot = static_cast<Vector3*>(luaL_checkudata(L, 2, "Vector3"));
        rx = rot->x; ry = rot->y; rz = rot->z;
    } else {
        rx = static_cast<float>(luaL_optnumber(L, 2, 0.0));
        ry = static_cast<float>(luaL_optnumber(L, 3, 0.0));
        rz = static_cast<float>(luaL_optnumber(L, 4, 0.0));
    }

    Camera3D* camera = static_cast<Camera3D*>(lua_newuserdata(L, sizeof(Camera3D)));
    new (camera) Camera3D(Vector3(x, y, z), Vector3(rx, ry, rz), fov, near_, far_);

    luaL_getmetatable(L, "Camera3D");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        luaL_error(L, "Camera3D metatable not found");
        return 0;
    }
    lua_setmetatable(L, -2);

    return 1;
}

int l_camera3d_getPosition(lua_State* L) {
    Camera3D* camera = checkCamera3D(L, 1);
    Vector3 pos = camera->getPosition();
    pushVector3(L, pos);
    return 1;
}

int l_camera3d_setPosition(lua_State* L) {
    Camera3D* camera = checkCamera3D(L, 1);
    Vector3* pos = static_cast<Vector3*>(luaL_checkudata(L, 2, "Vector3"));
    camera->setPosition(*pos);
    return 0;
}

int l_camera3d_getRotation(lua_State* L) {
    Camera3D* camera = checkCamera3D(L, 1);
    Vector3 rot = camera->getRotation();
    pushVector3(L, rot);
    return 1;
}

int l_camera3d_setRotation(lua_State* L) {
    Camera3D* camera = checkCamera3D(L, 1);
    Vector3* rot = static_cast<Vector3*>(luaL_checkudata(L, 2, "Vector3"));
    camera->setRotation(*rot);
    return 0;
}

int l_camera3d_getFOV(lua_State* L) {
    Camera3D* camera = checkCamera3D(L, 1);
    lua_pushnumber(L, camera->getFOV());
    return 1;
}

int l_camera3d_setFOV(lua_State* L) {
    Camera3D* camera = checkCamera3D(L, 1);
    float fov = static_cast<float>(luaL_checknumber(L, 2));
    camera->setFOV(fov);
    return 0;
}

int l_camera3d_getNearPlane(lua_State* L) {
    Camera3D* camera = checkCamera3D(L, 1);
    lua_pushnumber(L, camera->getNearPlane());
    return 1;
}

int l_camera3d_setNearPlane(lua_State* L) {
    Camera3D* camera = checkCamera3D(L, 1);
    float near_ = static_cast<float>(luaL_checknumber(L, 2));
    camera->setNearPlane(near_);
    return 0;
}

int l_camera3d_getFarPlane(lua_State* L) {
    Camera3D* camera = checkCamera3D(L, 1);
    lua_pushnumber(L, camera->getFarPlane());
    return 1;
}

int l_camera3d_setFarPlane(lua_State* L) {
    Camera3D* camera = checkCamera3D(L, 1);
    float far_ = static_cast<float>(luaL_checknumber(L, 2));
    camera->setFarPlane(far_);
    return 0;
}

int l_camera3d_move(lua_State* L) {
    Camera3D* camera = checkCamera3D(L, 1);
    Vector3* offset = static_cast<Vector3*>(luaL_checkudata(L, 2, "Vector3"));
    camera->move(*offset);
    return 0;
}

int l_camera3d_rotateBy(lua_State* L) {
    Camera3D* camera = checkCamera3D(L, 1);
    Vector3* degrees = static_cast<Vector3*>(luaL_checkudata(L, 2, "Vector3"));
    camera->rotateBy(*degrees);
    return 0;
}

int l_camera3d_tostring(lua_State* L) {
    Camera3D* camera = checkCamera3D(L, 1);
    lua_pushstring(L, camera->toString().c_str());
    return 1;
}

int l_setCamera3D(lua_State* L) {
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        luaL_error(L, "Framework instance not found");
        return 0;
    }
    if (lua_isnil(L, 1)) {
        framework->setCamera3D(nullptr);
    } else {
        Camera3D* camera = checkCamera3D(L, 1);
        framework->setCamera3D(camera);
    }
    return 0;
}