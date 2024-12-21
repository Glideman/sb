// #define SB_COORDINATE_SYSTEM_LH

#include "core/Math.h"

float Vector2::lenght()
{
    return std::sqrt(this->x * this->x + this->y * this->y);
}

void Vector2::normalize()
{
    float len = this->lenght();

    this->x /= len;
    this->y /= len;
}

void Vector2::negate()
{
    this->x = -this->x;
    this->y = -this->y;
}

void Vector2::subtract(Vector2 vec)
{
    this->x -= vec.getX();
    this->y -= vec.getY();
}

Vector2 Vector2::subtract(Vector2 a, Vector2 b)
{
    a.subtract(b);
    return a;
}

float Vector2::dot(Vector2 vec)
{
    return this->x * vec.getX() + this->y * vec.getY();
}

Vector2 Vector2::dot(Vector2 a, Vector2 b)
{
    a.dot(b);
    return a;
}

void Vector2::clear()
{
    std::memset(this, 0, sizeof(Vector2));
}

float Vector2::getX()
{
    return this->x;
}

float Vector2::getY()
{
    return this->y;
}

void Vector2::setX(float x)
{
    this->x = x;
}

void Vector2::setY(float y)
{
    this->y = y;
}

float Vector3::lenght()
{
    return std::sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
}

void Vector3::normalize()
{
    float len = this->lenght();

    this->x /= len;
    this->y /= len;
    this->z /= len;
}

void Vector3::negate()
{
    this->x = -this->x;
    this->y = -this->y;
    this->z = -this->z;
}

void Vector3::subtract(Vector3 vec)
{
    this->x -= vec.getX();
    this->y -= vec.getY();
    this->z -= vec.getZ();
}

Vector3 Vector3::subtract(Vector3 a, Vector3 b)
{
    a.subtract(b);
    return a;
}

void Vector3::cross(Vector3 vec)
{
    this->x = this->y * vec.getZ() - this->z * vec.getY();
    this->y = this->z * vec.getX() - this->x * vec.getZ();
    this->z = this->x * vec.getY() - this->y * vec.getX();
}

Vector3 Vector3::cross(Vector3 a, Vector3 b)
{
    a.cross(b);
    return a;
}

float Vector3::dot(Vector3 vec)
{
    return this->x * vec.getX() + this->y * vec.getY() + this->z * vec.getZ();
}

Vector3 Vector3::dot(Vector3 a, Vector3 b)
{
    a.dot(b);
    return a;
}

void Vector3::clear()
{
    memset(this, 0, sizeof(Vector3));
}

float Vector3::getX()
{
    return this->x;
}

float Vector3::getY()
{
    return this->y;
}

float Vector3::getZ()
{
    return this->z;
}

void Vector3::setX(float x)
{
    this->x = x;
}

void Vector3::setY(float y)
{
    this->y = y;
}

void Vector3::setZ(float z)
{
    this->z = z;
}

float Vector4::lenght()
{
    return std::sqrt(this->x * this->x + this->y * this->y + this->z * this->z + this->w * this->w);
}

void Vector4::normalize()
{
    float len = this->lenght();

    this->x /= len;
    this->y /= len;
    this->z /= len;
    this->w /= len;
}

void Vector4::negate()
{
    this->x = -this->x;
    this->y = -this->y;
    this->z = -this->z;
    this->w = -this->w;
}

void Vector4::subtract(Vector4 vec)
{
    this->x -= vec.getX();
    this->y -= vec.getY();
    this->z -= vec.getZ();
    this->w -= vec.getW();
}

Vector4 Vector4::subtract(Vector4 a, Vector4 b)
{
    a.subtract(b);
    return a;
}

float Vector4::dot(Vector4 vec)
{
    return this->x * vec.getX() + this->y * vec.getY() + this->z * vec.getZ() + this->w * vec.getW();
}

Vector4 Vector4::dot(Vector4 a, Vector4 b)
{
    a.dot(b);
    return a;
}

uint32_t Vector4::toInt()
{
    uint32_t dwX = this->x >= 1.0f ? 0xff : this->x <= 0.0f ? 0x00
                                                            : (uint32_t)(this->x * 255.0f + 0.5f);
    uint32_t dwY = this->y >= 1.0f ? 0xff : this->y <= 0.0f ? 0x00
                                                            : (uint32_t)(this->y * 255.0f + 0.5f);
    uint32_t dwZ = this->z >= 1.0f ? 0xff : this->z <= 0.0f ? 0x00
                                                            : (uint32_t)(this->z * 255.0f + 0.5f);
    uint32_t dwW = this->w >= 1.0f ? 0xff : this->w <= 0.0f ? 0x00
                                                            : (uint32_t)(this->w * 255.0f + 0.5f);

    return (dwX << 24) | (dwY << 16) | (dwZ << 8) | (dwW << 0);
}

void Vector4::fromInt(uint32_t i)
{
    float f = 1.0f / 255.0f;

    this->x = f * (float)(unsigned char)(i >> 24);
    this->y = f * (float)(unsigned char)(i >> 16);
    this->z = f * (float)(unsigned char)(i >> 8);
    this->w = f * (float)(unsigned char)(i >> 0);
}

void Vector4::clear()
{
    memset(this, 0, sizeof(Vector4));
}

float Vector4::getX()
{
    return this->x;
}

float Vector4::getY()
{
    return this->y;
}

float Vector4::getZ()
{
    return this->z;
}

float Vector4::getW()
{
    return this->w;
}

void Vector4::setX(float x)
{
    this->x = x;
}

void Vector4::setY(float y)
{
    this->y = y;
}

void Vector4::setZ(float z)
{
    this->z = z;
}

void Vector4::setW(float w)
{
    this->w = w;
}

void Matrix4x4::identity()
{
    this->m[0] = 1;
    this->m[1] = 0;
    this->m[2] = 0;
    this->m[3] = 0;
    this->m[4] = 0;
    this->m[5] = 1;
    this->m[6] = 0;
    this->m[7] = 0;
    this->m[8] = 0;
    this->m[9] = 0;
    this->m[10] = 1;
    this->m[11] = 0;
    this->m[12] = 0;
    this->m[13] = 0;
    this->m[14] = 0;
    this->m[15] = 1;
}

#ifdef SB_COORDINATE_SYSTEM_LH
void Matrix4x4::perspective(float fovy, float aspect, float znear, float zfar)
{
    float f = 1 / std::tanf(fovy / 2),
          a = zfar / (zfar - znear),
          b = -znear * zfar / (zfar - znear);

    this->m[0] = f / aspect;
    this->m[1] = 0;
    this->m[2] = 0;
    this->m[3] = 0;
    this->m[4] = 0;
    this->m[5] = f;
    this->m[6] = 0;
    this->m[7] = 0;
    this->m[8] = 0;
    this->m[9] = 0;
    this->m[10] = a;
    this->m[11] = 1;
    this->m[12] = 0;
    this->m[13] = 0;
    this->m[14] = b;
    this->m[15] = 0;
}

void Matrix4x4::ortho(float width, float height, float znear, float zfar)
{
    float a = 1.0f / (zfar - znear),
          b = -znear / (zfar - znear);

    this->m[0] = 2.0f / width;
    this->m[1] = 0;
    this->m[2] = 0;
    this->m[3] = 0;
    this->m[4] = 0;
    this->m[5] = 2.0f / height;
    this->m[6] = 0;
    this->m[7] = 0;
    this->m[8] = 0;
    this->m[9] = 0;
    this->m[10] = a;
    this->m[11] = 0;
    this->m[12] = 0;
    this->m[13] = 0;
    this->m[14] = b;
    this->m[15] = 1;
}

void Matrix4x4::lookAt(Vector3 eye, Vector3 at, Vector3 up)
{
    Vector3 axisZ = Vector3::subtract(at, eye);
    axisZ.normalize();

    Vector3 axisX = Vector3::cross(up, axisZ);
    axisX.normalize();

    Vector3 axisY = Vector3::cross(axisZ, axisX);
    // axisY.normalize();
    //  TODO Normalize or not?... Hmmmm...

    this->m[0] = axisX.getX();
    this->m[1] = axisY.getX();
    this->m[2] = axisZ.getX();
    this->m[3] = 0;
    this->m[4] = axisX.getY();
    this->m[5] = axisY.getY();
    this->m[6] = axisZ.getY();
    this->m[7] = 0;
    this->m[8] = axisX.getZ();
    this->m[9] = axisY.getZ();
    this->m[10] = axisZ.getZ();
    this->m[11] = 0;
    this->m[12] = -axisX.dot(eye);
    this->m[13] = -axisY.dot(eye);
    this->m[14] = -axisZ.dot(eye);
    this->m[15] = 1;
}
#else
void Matrix4x4::perspective(float fovy, float aspect, float znear, float zfar)
{
    float f = 1 / tanf(fovy / 2),
          a = zfar / (znear - zfar),
          b = znear * zfar / (znear - zfar);

    this->m[0] = f / aspect;
    this->m[1] = 0;
    this->m[2] = 0;
    this->m[3] = 0;
    this->m[4] = 0;
    this->m[5] = f;
    this->m[6] = 0;
    this->m[7] = 0;
    this->m[8] = 0;
    this->m[9] = 0;
    this->m[10] = a;
    this->m[11] = -1;
    this->m[12] = 0;
    this->m[13] = 0;
    this->m[14] = b;
    this->m[15] = 0;
}

void Matrix4x4::ortho(float width, float height, float znear, float zfar)
{
    float a = 1.0f / (znear - zfar),
          b = znear / (znear - zfar);

    this->m[0] = 2.0f / width;
    this->m[1] = 0;
    this->m[2] = 0;
    this->m[3] = 0;
    this->m[4] = 0;
    this->m[5] = 2.0f / height;
    this->m[6] = 0;
    this->m[7] = 0;
    this->m[8] = 0;
    this->m[9] = 0;
    this->m[10] = a;
    this->m[11] = 0;
    this->m[12] = 0;
    this->m[13] = 0;
    this->m[14] = b;
    this->m[15] = 1;
}

void Matrix4x4::lookAt(Vector3 eye, Vector3 at, Vector3 up)
{
    Vector3 axisZ = Vector3::subtract(eye, at);
    axisZ.normalize();

    Vector3 axisX = Vector3::cross(up, axisZ);
    axisX.normalize();

    Vector3 axisY = Vector3::cross(axisZ, axisX);
    // axisY.normalize();
    //  TODO Normalize or not?... Hmmmm...

    // strange thing. in dx docs it's says +dot(axis, eye) but with that camera looks afterwards
    // with -dot(axis, eye) camera look in right direction. i dunno why
    this->m[0] = axisX.getX();
    this->m[1] = axisY.getX();
    this->m[2] = axisZ.getX();
    this->m[3] = 0;
    this->m[4] = axisX.getY();
    this->m[5] = axisY.getY();
    this->m[6] = axisZ.getY();
    this->m[7] = 0;
    this->m[8] = axisX.getZ();
    this->m[9] = axisY.getZ();
    this->m[10] = axisZ.getZ();
    this->m[11] = 0;
    this->m[12] = -axisX.dot(eye);
    this->m[13] = -axisY.dot(eye);
    this->m[14] = -axisZ.dot(eye);
    this->m[15] = 1;
}
#endif

void Matrix4x4::mul(Matrix4x4 m)
{
    float newM[16];
    float *a = this->getM();
    float *b = m.getM();

    newM[0] = a[0] * b[0] + a[1] * b[4] + a[2] * b[8] + a[3] * b[12];
    newM[1] = a[0] * b[1] + a[1] * b[5] + a[2] * b[9] + a[3] * b[13];
    newM[2] = a[0] * b[2] + a[1] * b[6] + a[2] * b[10] + a[3] * b[14];
    newM[3] = a[0] * b[3] + a[1] * b[7] + a[2] * b[11] + a[3] * b[15];
    newM[4] = a[4] * b[0] + a[5] * b[4] + a[6] * b[8] + a[7] * b[12];
    newM[5] = a[4] * b[1] + a[5] * b[5] + a[6] * b[9] + a[7] * b[13];
    newM[6] = a[4] * b[2] + a[5] * b[6] + a[6] * b[10] + a[7] * b[14];
    newM[7] = a[4] * b[3] + a[5] * b[7] + a[6] * b[11] + a[7] * b[15];
    newM[8] = a[8] * b[0] + a[9] * b[4] + a[10] * b[8] + a[11] * b[12];
    newM[9] = a[8] * b[1] + a[9] * b[5] + a[10] * b[9] + a[11] * b[13];
    newM[10] = a[8] * b[2] + a[9] * b[6] + a[10] * b[10] + a[11] * b[14];
    newM[11] = a[8] * b[3] + a[9] * b[7] + a[10] * b[11] + a[11] * b[15];
    newM[12] = a[12] * b[0] + a[13] * b[4] + a[14] * b[8] + a[15] * b[12];
    newM[13] = a[12] * b[1] + a[13] * b[5] + a[14] * b[9] + a[15] * b[13];
    newM[14] = a[12] * b[2] + a[13] * b[6] + a[14] * b[10] + a[15] * b[14];
    newM[15] = a[12] * b[3] + a[13] * b[7] + a[14] * b[11] + a[15] * b[15];

    memcpy(a, newM, sizeof(Matrix4x4));
}

Matrix4x4 Matrix4x4::mul(Matrix4x4 a, Matrix4x4 b)
{
    a.mul(b);
    return a;
}

void Matrix4x4::rotate(float x, float y, float z)
{
    Matrix4x4 oldM{this->getM()};

    // degrees into radian
    float ratio = float(std::numbers::pi) / 180;
    x *= ratio;
    y *= ratio;
    z *= ratio;

    float a = cosf(x), b = sinf(x), c = cosf(y),
          d = sinf(y), e = cosf(z), f = sinf(z);
    float ad = a * d, bd = b * d;

    this->m[0] = c * e;
    this->m[1] = -c * f;
    this->m[2] = d;
    this->m[3] = 0;
    this->m[4] = bd * e + a * f;
    this->m[5] = -bd * f + a * e;
    this->m[6] = -b * c;
    this->m[7] = 0;
    this->m[8] = -ad * e + b * f;
    this->m[9] = ad * f + b * e;
    this->m[10] = a * c;
    this->m[11] = 0;
    this->m[12] = 0;
    this->m[13] = 0;
    this->m[14] = 0;
    this->m[15] = 1;

    this->mul(oldM);
}

void Matrix4x4::rotate(Vector3 vec)
{
    this->rotate(vec.getX(), vec.getY(), vec.getZ());
}

void Matrix4x4::move(float x, float y, float z)
{
    Matrix4x4 oldM{this->getM()};

    this->m[0] = 1;
    this->m[1] = 0;
    this->m[2] = 0;
    this->m[3] = 0;
    this->m[4] = 0;
    this->m[5] = 1;
    this->m[6] = 0;
    this->m[7] = 0;
    this->m[8] = 0;
    this->m[9] = 0;
    this->m[10] = 1;
    this->m[11] = 0;
    this->m[12] = x;
    this->m[13] = y;
    this->m[14] = z;
    this->m[15] = 1;

    this->mul(oldM);
}

void Matrix4x4::move(Vector3 vec)
{
    this->move(vec.getX(), vec.getY(), vec.getZ());
}

void Matrix4x4::scale(float x, float y, float z)
{
    Matrix4x4 oldM{this->getM()};

    this->m[0] = x;
    this->m[1] = 0;
    this->m[2] = 0;
    this->m[3] = 0;
    this->m[4] = 0;
    this->m[5] = y;
    this->m[6] = 0;
    this->m[7] = 0;
    this->m[8] = 0;
    this->m[9] = 0;
    this->m[10] = z;
    this->m[11] = 0;
    this->m[12] = 0;
    this->m[13] = 0;
    this->m[14] = 0;
    this->m[15] = 1;

    this->mul(oldM);
}

void Matrix4x4::scale(Vector3 vec)
{
    this->scale(vec.getX(), vec.getY(), vec.getZ());
}

void Matrix4x4::clear()
{
    memset(this->m, 0, sizeof(Matrix4x4));
}

float *Matrix4x4::getM()
{
    return this->m;
}

void Matrix4x4::setM(float *m)
{
    std::memcpy(this->m, m, sizeof(Matrix4x4));
}