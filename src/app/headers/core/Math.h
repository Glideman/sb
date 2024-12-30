#pragma once

#include "core/Core.h"

#include <stdint.h>
#include <cstring>
#include <cmath>
#include <numbers>

class Vector2
{
protected:
    float x, y;

public:
    Vector2() { std::memset(this, 0, sizeof(Vector2)); }
    Vector2(float *vec) { std::memcpy(this, vec, sizeof(Vector2)); }
    Vector2(float x, float y) : x(x), y(y) {}
    ~Vector2() { this->clear(); }

    float lenght();
    void normalize();
    void negate();
    void subtract(Vector2 vec);
    static Vector2 subtract(Vector2 a, Vector2 b);
    float dot(Vector2 vec);
    static Vector2 dot(Vector2 a, Vector2 b);
    void clear();
    float getX();
    float getY();
    void setX(float x);
    void setY(float y);
};

class Vector3
{
protected:
    float x, y, z;

public:
    Vector3() { std::memset(this, 0, sizeof(Vector3)); }
    Vector3(float *vec) { std::memcpy(this, vec, sizeof(Vector3)); }
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    ~Vector3() { this->clear(); }

    float lenght();
    void normalize();
    void negate();
    void subtract(Vector3 vec);
    static Vector3 subtract(Vector3 a, Vector3 b);
    void cross(Vector3 vec);
    static Vector3 cross(Vector3 a, Vector3 b);
    float dot(Vector3 vec);
    static Vector3 dot(Vector3 a, Vector3 b);
    void clear();
    float getX();
    float getY();
    float getZ();
    void setX(float x);
    void setY(float y);
    void setZ(float z);
};

class Vector4
{
protected:
    float x, y, z, w;

public:
    Vector4() { std::memset(this, 0, sizeof(Vector4)); }
    Vector4(float *vec) { std::memcpy(this, vec, sizeof(Vector4)); }
    Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    ~Vector4() { this->clear(); }

    float lenght();
    void normalize();
    void negate();
    void subtract(Vector4 vec);
    static Vector4 subtract(Vector4 a, Vector4 b);
    float dot(Vector4 vec);
    static Vector4 dot(Vector4 a, Vector4 b);
    uint32_t toInt();
    void fromInt(uint32_t i);
    void clear();
    float getX();
    float getY();
    float getZ();
    float getW();
    void setX(float x);
    void setY(float y);
    void setZ(float z);
    void setW(float w);
};

class Matrix4x4
{
protected:
    float m[16];

public:
    Matrix4x4() { std::memset(this->m, 0, sizeof(Matrix4x4)); }
    Matrix4x4(float *m) { std::memcpy(this->m, m, sizeof(Matrix4x4)); }
    ~Matrix4x4() { this->clear(); }

    void identity();
    void perspective(float fovy, float aspect, float znear, float zfar);
    void ortho(float width, float height, float znear, float zfar);
    void lookAt(Vector3 eye, Vector3 at, Vector3 up);
    void mul(Matrix4x4 m);
    static Matrix4x4 mul(Matrix4x4 a, Matrix4x4 b);
    void rotate(float x, float y, float z);
    void rotate(Vector3 vec);
    void move(float x, float y, float z);
    void move(Vector3 vec);
    void scale(float x, float y, float z);
    void scale(Vector3 vec);
    void clear();
    float *getM();
    void setM(float *m);
};
