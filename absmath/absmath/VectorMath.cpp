/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "VectorMath.h"
#include "MathDefs.h"
#include "MathUtils.h"
#include "Matrix4.h"
#include <sa/Compiler.h>

namespace Math {

bool IsPointInTriangle(const Vector3& point, const Vector3& pa, const Vector3& pb, const Vector3& pc)
{
#if 1
    const Vector3 e10 = pb - pa;
    const Vector3 e20 = pc - pa;

    float a = e10.DotProduct(e10);
    float b = e10.DotProduct(e20);
    float c = e20.DotProduct(e20);
    float ac_bb = (a*c) - (b*b);

    const Vector3 vp(point.x_ - pa.x_, point.y_ - pa.y_, point.z_ - pa.z_);

    float d = vp.DotProduct(e10);
    float e = vp.DotProduct(e20);
    float x = (d*c) - (e*b);
    float y = (e*a) - (d*b);
    float z = x + y - ac_bb;

#define in(a) ((uint32_t&) a)
PRAGMA_WARNING_PUSH
PRAGMA_WARNING_DISABLE_GCC("-Wstrict-aliasing")
    return ((in(z) & ~(in(x) | in(y)))) & 0x80000000;
PRAGMA_WARNING_POP
#undef in
#else
    const Vector3 v1 = (point - pa).Normal();
    const Vector3 v2 = (point - pb).Normal();
    const Vector3 v3 = (point - pc).Normal();

    // If the point is in the triangle the sum of the angles can not be
    // greater than 360 Deg, because the sum the of the angles of a triangle
    // is 360 Deg
    const float totalAngles =
        acosf(v1.DotProduct(v2)) +
        acosf(v2.DotProduct(v3)) +
        acosf(v3.DotProduct(v1));

    if (fabsf(totalAngles - M_TWOPI) <= 0.005f)
        return true;
    return false;
#endif
}

Vector3 GetClosestPointOnLine(const Vector3& a, const Vector3& b, const Vector3& p)
{
    const Vector3 c = p - a;
    Vector3 V = b - a;
    float d = V.Length();

    V.Normalize();
    float t = V.DotProduct(c);
    if (t < 0.0f)
        return a;
    if (t > d)
        return b;

    V *= t;

    return a + V;
}

Vector3 GetClosestPointOnTriangle(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& p)
{
    Vector3 Rab = GetClosestPointOnLine(a, b, p);
    Vector3 Rbc = GetClosestPointOnLine(b, c, p);
    Vector3 Rca = GetClosestPointOnLine(c, a, p);

    float dAB = (p - Rab).Length();
    float dBC = (p - Rbc).Length();
    float dCA = (p - Rca).Length();

    Vector3 result = Rab;

    float m = dAB;
    if (dBC < m)
    {
        m = dBC;
        result = Rbc;
    }
    if (dCA < m)
        result = Rca;

    return result;
}

PointClass GetPointClass(const Vector3& point, const Vector3& origin, const Vector3& normal)
{
    const Vector3 dir = origin - point;
    float d = dir.DotProduct(normal);
    if (d < -0.001f)
        return PointClass::PlaneFront;
    if (d > 0.001f)
        return PointClass::PlaneBack;

    return PointClass::OnPlane;
}

Vector3 GetTriangleNormal(const Vector3& p1, const Vector3& p2, const Vector3& p3)
{
    return (p1 - p2).CrossProduct(p1 - p3).Normal();
}

void ReverseOrder(std::array<Vector3, 3>& triangle)
{
    std::swap(triangle[0], triangle[2]);
}

Vector3 GetPosFromDirectionDistance(const Vector3& position, const Quaternion& direction, float distance)
{
    Math::Matrix4 m = Math::Matrix4::FromQuaternion(direction.Inverse());
    Math::Vector3 v = m * (Math::Vector3::One * distance);
    return position + v;
}

}
