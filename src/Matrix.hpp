#ifndef MATRIX_H
#define MATRIX_H

#include "Vector3.hpp"

// Forward declaration of Matrix4
struct Matrix4;

// Declare multiply with forward-declared Matrix4
Matrix4 multiply(const Matrix4& a, const Matrix4& b);

struct Matrix4 {
    float m[16];
    
    Matrix4();
    void setIdentity();
    
    // Declare operator* without defining it inline
    Matrix4 operator*(const Matrix4& other) const;
    
    float* data() { return m; }  // New method to access raw data for OpenGL
};

void setPerspective(Matrix4& mat, float fov, float aspect, float nearPlane, float farPlane);
void setLookAt(Matrix4& mat, const Vector3& eye, const Vector3& center, const Vector3& up);
void translate(Matrix4& mat, const Vector3& translation);
void rotateX(Matrix4& mat, float angle);
void rotateY(Matrix4& mat, float angle);
void rotateZ(Matrix4& mat, float angle);
void scale(Matrix4& mat, const Vector3& scaleFactors);
void orthographic(Matrix4& mat, float left, float right, float bottom, float top, float nearPlane, float farPlane);

#endif // MATRIX_H