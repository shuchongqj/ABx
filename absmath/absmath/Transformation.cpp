#include "stdafx.h"
#include "Transformation.h"
#include "Matrix4.h"

namespace Math {

XMath::XMMATRIX Transformation::GetMatrix() const
{
    return GetMatrix(oriention_);
}

XMath::XMMATRIX Transformation::GetMatrix(const Quaternion& rot) const
{
    static const XMath::XMVECTOR vZero = { 0 };
    static const XMath::XMVECTOR qId = { 0.0f, 0.0f, 0.0f, 1.0f };
    XMath::XMVECTOR scale = XMath::XMVectorSet(scale_.x_, scale_.y_, scale_.z_, 0.0f);
    XMath::XMVECTOR rotation = XMath::XMVectorSet(rot.x_, rot.y_, rot.z_, rot.w_);
    XMath::XMVECTOR position = XMath::XMVectorSet(position_.x_, position_.y_, position_.z_, 0.0f);
    return XMath::XMMatrixTransformation(vZero, qId, scale, vZero, rotation, position);
}

void Transformation::Move(float speed, const Vector3& amount)
{
    // new position = position + direction * speed (where speed = amount * speed)

    // It's as easy as:
    // 1. Create a matrix from the rotation,
    // 2. multiply this matrix with the moving vector and
    // 3. add the resulting vector to the current position
#if defined(HAVE_DIRECTX_MATH) || defined(HAVE_X_MATH)
    XMath::XMMATRIX m = XMath::XMMatrixRotationAxis(Math::Vector3::UnitY, -GetYRotation());
    Vector3 a = amount * speed;
    XMath::XMVECTOR v = XMath::XMVector3Transform(a, m);
    position_.x_ += XMath::XMVectorGetX(v);
    position_.y_ += XMath::XMVectorGetY(v);
    position_.z_ += XMath::XMVectorGetZ(v);
#else
    Matrix4 m = Math::Matrix4::FromQuaternion(oriention_.Inverse());
    Vector3 a = amount * speed;
    Vector3 v = m * a;
    position_ += v;
#endif
}

void Transformation::Turn(float yAngle)
{
    float ang = GetYRotation();
    ang += yAngle;
    NormalizeAngle(ang);
    SetYRotation(ang);
}

float Transformation::GetYRotation() const
{
    return oriention_.EulerAngles().y_;
}

void Transformation::SetYRotation(float rad)
{
    oriention_ = Quaternion(oriention_.x_, rad, oriention_.z_);
}

void Transformation::LookAt(const Vector3& lookAt, const Vector3& up)
{
    Matrix4 mat = Matrix4::FromLookAt(lookAt, position_, up);
    oriention_ = mat.Rotation();
}

}
