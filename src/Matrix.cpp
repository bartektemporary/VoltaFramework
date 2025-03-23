#include "Matrix.hpp"
#define _USE_MATH_DEFINES
#include <math.h>

Matrix4::Matrix4() {
    for (int i = 0; i < 16; i++) m[i] = 0.0f;
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

void Matrix4::setIdentity() {
    for (int i = 0; i < 16; i++) m[i] = 0.0f;
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

// New implementation of operator*
Matrix4 Matrix4::operator*(const Matrix4& other) const {
    return multiply(*this, other);
}

Matrix4 multiply(const Matrix4& a, const Matrix4& b) {
    Matrix4 result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.m[i + j * 4] = 0.0f;
            for (int k = 0; k < 4; k++) {
                result.m[i + j * 4] += a.m[i + k * 4] * b.m[k + j * 4];
            }
        }
    }
    return result;
}

void setPerspective(Matrix4& mat, float fov, float aspect, float nearPlane, float farPlane) {
    float tanHalfFov = tanf(fov / 2.0f);
    mat.setIdentity();
    mat.m[0] = 1.0f / (aspect * tanHalfFov);
    mat.m[5] = 1.0f / tanHalfFov;
    mat.m[10] = -(farPlane + nearPlane) / (farPlane - nearPlane);
    mat.m[11] = -1.0f;
    mat.m[14] = -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane);
    mat.m[15] = 0.0f;
}

void setLookAt(Matrix4& mat, const Vector3& eye, const Vector3& center, const Vector3& up) {
    Vector3 f = {center.x - eye.x, center.y - eye.y, center.z - eye.z};
    float len = sqrtf(f.x * f.x + f.y * f.y + f.z * f.z);
    f.x /= len; f.y /= len; f.z /= len;

    Vector3 s = {f.y * up.z - f.z * up.y, f.z * up.x - f.x * up.z, f.x * up.y - f.y * up.x};
    len = sqrtf(s.x * s.x + s.y * s.y + s.z * s.z);
    s.x /= len; s.y /= len; s.z /= len;

    Vector3 u = {s.y * f.z - s.z * f.y, s.z * f.x - s.x * f.z, s.x * f.y - s.y * f.x};

    mat.setIdentity();
    mat.m[0] = s.x; mat.m[4] = s.y; mat.m[8] = s.z;
    mat.m[1] = u.x; mat.m[5] = u.y; mat.m[9] = u.z;
    mat.m[2] = -f.x; mat.m[6] = -f.y; mat.m[10] = -f.z;
    mat.m[12] = -(s.x * eye.x + s.y * eye.y + s.z * eye.z);
    mat.m[13] = -(u.x * eye.x + u.y * eye.y + u.z * eye.z);
    mat.m[14] = (f.x * eye.x + f.y * eye.y + f.z * eye.z);
}

void translate(Matrix4& mat, const Vector3& translation) {
    Matrix4 t;
    t.m[12] = translation.x;
    t.m[13] = translation.y;
    t.m[14] = translation.z;
    mat = multiply(mat, t);
}

void rotateX(Matrix4& mat, float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    Matrix4 rot;
    rot.setIdentity();
    rot.m[5] = c;  rot.m[6] = -s;  // Row 1: [0,  c, -s, 0]
    rot.m[9] = s;  rot.m[10] = c;  // Row 2: [0,  s,  c, 0]
    mat = multiply(mat, rot);
}

void rotateY(Matrix4& mat, float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    Matrix4 rot;
    rot.setIdentity();
    rot.m[0] = c;  rot.m[2] = s;   // Row 0: [c, 0, s, 0]
    rot.m[8] = -s; rot.m[10] = c;  // Row 2: [-s, 0, c, 0]
    mat = multiply(mat, rot);
}

void rotateZ(Matrix4& mat, float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    Matrix4 rot;
    rot.setIdentity();
    rot.m[0] = c;  rot.m[1] = -s;  // Row 0: [c, -s, 0, 0]
    rot.m[4] = s;  rot.m[5] = c;   // Row 1: [s,  c, 0, 0]
    mat = multiply(mat, rot);
}

void scale(Matrix4& mat, const Vector3& scaleFactors) {
    Matrix4 s;
    s.m[0] = scaleFactors.x;
    s.m[5] = scaleFactors.y;
    s.m[10] = scaleFactors.z;
    mat = multiply(mat, s);
}

void orthographic(Matrix4& mat, float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    mat.setIdentity();
    mat.m[0] = 2.0f / (right - left);              // Scale x
    mat.m[5] = 2.0f / (top - bottom);              // Scale y
    mat.m[10] = -2.0f / (farPlane - nearPlane);    // Scale z
    mat.m[12] = -(right + left) / (right - left);  // Translate x
    mat.m[13] = -(top + bottom) / (top - bottom);  // Translate y
    mat.m[14] = -(farPlane + nearPlane) / (farPlane - nearPlane); // Translate z
}