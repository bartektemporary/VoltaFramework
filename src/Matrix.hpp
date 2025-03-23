#ifndef MATRIX_H
#define MATRIX_H

#include "Vector3.hpp"

struct Matrix4 {
    float m[16];
    
    Matrix4();
    void setIdentity();
};

Matrix4 multiply(const Matrix4& a, const Matrix4& b);
void setPerspective(Matrix4& mat, float fov, float aspect, float nearPlane, float farPlane);
void setLookAt(Matrix4& mat, const Vector3& eye, const Vector3& center, const Vector3& up);
void translate(Matrix4& mat, const Vector3& translation);
void rotateX(Matrix4& mat, float angle);
void rotateY(Matrix4& mat, float angle);
void rotateZ(Matrix4& mat, float angle);
void scale(Matrix4& mat, const Vector3& scaleFactors);

#endif // MATRIX_H