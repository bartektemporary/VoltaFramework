#ifndef CAMERA3D_HPP
#define CAMERA3D_HPP

#include "Vector3.hpp"
#include "Matrix.hpp"

class Camera3D {
public:
    Camera3D();
    Camera3D(const Vector3& position, const Vector3& rotation, float fov = 60.0f, float nearPlane = 0.1f, float farPlane = 1000.0f);

    Matrix4 getViewMatrix() const { return viewMatrix; }
    Matrix4 getProjectionMatrix(float aspectRatio) const;

    void setPosition(const Vector3& newPosition);
    void setRotation(const Vector3& newRotation); // Euler angles in degrees (pitch, yaw, roll)
    void setFOV(float newFOV);
    void setNearPlane(float newNear);
    void setFarPlane(float newFar);

    Vector3 getPosition() const { return position; }
    Vector3 getRotation() const { return rotation; }
    float getFOV() const { return fov; }
    float getNearPlane() const { return nearPlane; }
    float getFarPlane() const { return farPlane; }

    void move(const Vector3& offset);
    void rotateBy(const Vector3& degrees); // Adds to current rotation

    std::string toString() const;

private:
    Vector3 position;       // Camera position in 3D space
    Vector3 rotation;       // Euler angles (pitch, yaw, roll) in degrees
    float fov;              // Field of view in degrees
    float nearPlane;        // Near clipping plane
    float farPlane;         // Far clipping plane

    Matrix4 viewMatrix;     // Cached view matrix
    mutable Matrix4 projectionMatrix; // Cached projection matrix (mutable for const access)

    void updateViewMatrix();
    void updateProjectionMatrix(float aspectRatio) const; // Made const and mutable
};

#endif // CAMERA3D_HPP