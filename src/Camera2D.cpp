#include "VoltaFramework.hpp"
#include "Camera2D.hpp"
#include <cmath>
#include <cstring> // For snprintf

Camera2D::Camera2D() : position(0.0f, 0.0f), zoom(1.0f), rotation(0.0f) {
    updateViewMatrix();
}

Camera2D::Camera2D(const Vector2& position, float zoom, float rotation)
    : position(position), zoom(zoom), rotation(rotation) {
    updateViewMatrix();
}

Matrix4 Camera2D::getViewMatrix() const {
    return viewMatrix;
}

void Camera2D::setPosition(const Vector2& newPosition) {
    position = newPosition;
    updateViewMatrix();
}

void Camera2D::setZoom(float newZoom) {
    zoom = std::max(0.01f, newZoom); // Prevent zoom from going too small or negative
    updateViewMatrix();
}

void Camera2D::setRotation(float newRotation) {
    rotation = newRotation;
    updateViewMatrix();
}

void Camera2D::move(const Vector2& offset) {
    position.x += offset.x;
    position.y += offset.y;
    updateViewMatrix();
}

void Camera2D::zoomBy(float factor) {
    zoom *= factor;
    zoom = std::max(0.01f, zoom); // Ensure zoom stays reasonable
    updateViewMatrix();
}

void Camera2D::rotateBy(float degrees) {
    rotation += degrees;
    updateViewMatrix();
}

void Camera2D::updateViewMatrix() {
    Matrix4 scaleMat;
    scale(scaleMat, Vector3(zoom, zoom, 1.0f));
    
    Matrix4 rotateMat;
    rotateZ(rotateMat, rotation * M_PI / 180.0f);
    
    Matrix4 translateMat;
    translate(translateMat, Vector3(-position.x * zoom, -position.y * zoom, 0.0f)); // Scale translation by zoom
    
    viewMatrix = scaleMat * rotateMat * translateMat; // Scale -> Rotate -> Translate
}

// New C++ API method
std::string Camera2D::toString() const {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Camera2D(pos: (%.2f, %.2f), zoom: %.2f, rotation: %.2f)",
             position.x, position.y, zoom, rotation);
    return std::string(buffer);
}

// Lua bindings

Camera2D* checkCamera2D(lua_State* L, int index) {
    return static_cast<Camera2D*>(luaL_checkudata(L, index, "Camera2D"));
}

int l_camera2d_new(lua_State* L) {
    Vector2* pos = nullptr;
    float x = 0.0f, y = 0.0f;
    float zoom = static_cast<float>(luaL_optnumber(L, 3, 1.0));
    float rotation = static_cast<float>(luaL_optnumber(L, 4, 0.0));

    if (lua_isuserdata(L, 1) && luaL_checkudata(L, 1, "Vector2")) {
        pos = checkVector2(L, 1);
        x = pos->x;
        y = pos->y;
    } else {
        x = static_cast<float>(luaL_optnumber(L, 1, 0.0));
        y = static_cast<float>(luaL_optnumber(L, 2, 0.0));
    }

    Camera2D* camera = static_cast<Camera2D*>(lua_newuserdata(L, sizeof(Camera2D)));
    new (camera) Camera2D(Vector2(x, y), zoom, rotation);

    luaL_getmetatable(L, "Camera2D");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        luaL_error(L, "Camera2D metatable not found");
        return 0;
    }
    lua_setmetatable(L, -2);

    return 1;
}

int l_camera2d_getPosition(lua_State* L) {
    Camera2D* camera = checkCamera2D(L, 1);
    Vector2 pos = camera->getPosition();
    Vector2* result = static_cast<Vector2*>(lua_newuserdata(L, sizeof(Vector2)));
    *result = pos; // Use assignment operator
    luaL_getmetatable(L, "Vector2");
    lua_setmetatable(L, -2);
    return 1;
}

int l_camera2d_setPosition(lua_State* L) {
    Camera2D* camera = checkCamera2D(L, 1);
    Vector2* pos = checkVector2(L, 2); // Expect a Vector2 object
    camera->setPosition(*pos);
    return 0;
}

int l_camera2d_getZoom(lua_State* L) {
    Camera2D* camera = checkCamera2D(L, 1);
    lua_pushnumber(L, camera->getZoom());
    return 1;
}

int l_camera2d_setZoom(lua_State* L) {
    Camera2D* camera = checkCamera2D(L, 1);
    float zoom = static_cast<float>(luaL_checknumber(L, 2));
    camera->setZoom(zoom);
    return 0;
}

int l_camera2d_getRotation(lua_State* L) {
    Camera2D* camera = checkCamera2D(L, 1);
    lua_pushnumber(L, camera->getRotation());
    return 1;
}

int l_camera2d_setRotation(lua_State* L) {
    Camera2D* camera = checkCamera2D(L, 1);
    float rotation = static_cast<float>(luaL_checknumber(L, 2));
    camera->setRotation(rotation);
    return 0;
}

int l_camera2d_move(lua_State* L) {
    Camera2D* camera = checkCamera2D(L, 1);
    Vector2* offset = checkVector2(L, 2); // Expect a Vector2 object
    camera->move(*offset);
    return 0;
}

int l_camera2d_zoomBy(lua_State* L) {
    Camera2D* camera = checkCamera2D(L, 1);
    float factor = static_cast<float>(luaL_checknumber(L, 2));
    camera->zoomBy(factor);
    return 0;
}

int l_camera2d_rotateBy(lua_State* L) {
    Camera2D* camera = checkCamera2D(L, 1);
    float degrees = static_cast<float>(luaL_checknumber(L, 2));
    camera->rotateBy(degrees);
    return 0;
}

int l_camera2d_tostring(lua_State* L) {
    Camera2D* camera = checkCamera2D(L, 1);
    lua_pushstring(L, camera->toString().c_str()); // Use new C++ API method
    return 1;
}

int l_setCamera2D(lua_State* L) {
    VoltaFramework* framework = getFramework(L); // Assuming getFramework is defined elsewhere
    if (!framework) {
        luaL_error(L, "Framework instance not found");
        return 0;
    }
    if (lua_isnil(L, 1)) {
        framework->setCamera2D(nullptr); // Clear camera
    } else {
        Camera2D* camera = checkCamera2D(L, 1);
        framework->setCamera2D(camera);
    }
    return 0;
}

void VoltaFramework::setCamera2D(Camera2D* camera) {
    currentCamera = camera;
    if (camera) {
        glUseProgram(shape2DShaderProgram);
        Matrix4 viewMatrix = camera->getViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shape2DShaderProgram, "view"), 1, GL_FALSE, viewMatrix.m);
        // Update projection matrix for 2D (orthographic)
        Matrix4 ortho;
        orthographic(ortho, 0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height), -1.0f, 1.0f);
        projection2D = ortho * viewMatrix;
        glUniformMatrix4fv(glGetUniformLocation(shape2DShaderProgram, "projection"), 1, GL_FALSE, projection2D.m);
    } else {
        glUseProgram(shape2DShaderProgram);
        Matrix4 ortho;
        orthographic(ortho, 0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height), -1.0f, 1.0f);
        projection2D = ortho;
        glUniformMatrix4fv(glGetUniformLocation(shape2DShaderProgram, "projection"), 1, GL_FALSE, projection2D.m);
        glUniformMatrix4fv(glGetUniformLocation(shape2DShaderProgram, "view"), 1, GL_FALSE, Matrix4().m); // Identity
    }
}

bool VoltaFramework::isRectInView(const Rect& objectBounds) const {
    return !(objectBounds.right < cachedViewBounds.left ||
             objectBounds.left > cachedViewBounds.right ||
             objectBounds.top < cachedViewBounds.bottom ||
             objectBounds.bottom > cachedViewBounds.top);
}