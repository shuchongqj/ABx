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


#include "CollisionShape.h"
#include "VectorMath.h"
#include <algorithm>
#include "Plane.h"
#include "Ray.h"
#include "Sphere.h"
#include <iostream>
#include "BoundingBox.h"

namespace Math {

AbstractCollisionShape::~AbstractCollisionShape() = default;

bool AbstractCollisionShape::GetManifold(CollisionManifold& manifold, const Matrix4& transformation) const
{
    // We know we are colliding with this object. this simplifies stuff.

    bool result = false;
    // Transform to world coordinates
    const Shape shape = GetShape().Transformed(transformation);

    // World coordinates
    const Vector3& source = manifold.position;
    manifold.normalizedVelocity = manifold.velocity.Normal();

    float distance = manifold.velocity.Length();

    // Radius is 1 because the points are in our ellipsoid space
    const Sphere sphere(source, 1.0f);
    const BoundingBox bb(source - 1.0f, source + 1.0f);

    for (unsigned i = 0; i < shape.GetCount(); i += 3)
    {
        // One face world coordinates
        const Vector3 A = shape.GetVertex(i);
        const Vector3 B = shape.GetVertex(i + 1);
        const Vector3 C = shape.GetVertex(i + 2);

        // Scale the triangle to ellipsoid space
        const Vector3 p1 = A / manifold.radius;
        const Vector3 p2 = B / manifold.radius;
        const Vector3 p3 = C / manifold.radius;

        Vector3 planeOrigin = p1;
        Vector3 v1 = p2 - p1;
        Vector3 v2 = p3 - p1;

        if (v1.Equals(Vector3::Zero) || v2.Equals(Vector3::Zero))
            continue;

        const Vector3 planeNormal = v1.CrossProduct(v2).Normal();

        Vector3 sphereIntersectionPoint = source - planeNormal;

        float distToIntersection;
        Vector3 intersectionPoint;
        PointClass pointClass = GetPointClass(sphereIntersectionPoint, planeOrigin, planeNormal);
        if (pointClass == PointClass::PlaneBack)
        {
            const Ray ray(sphereIntersectionPoint, planeNormal);
            const Plane plane(planeNormal, planeOrigin);
            distToIntersection = ray.HitDistance(plane);
            intersectionPoint = sphereIntersectionPoint + (distToIntersection * planeNormal);
        }
        else
        {
            const Ray ray(sphereIntersectionPoint, manifold.normalizedVelocity);
            const Plane plane(planeNormal, planeOrigin);
            distToIntersection = ray.HitDistance(plane);
            intersectionPoint = sphereIntersectionPoint + (distToIntersection * manifold.normalizedVelocity);
        }

        if (!IsPointInTriangle(intersectionPoint, p1, p2, p3))
        {
            const Vector3 polyPoint = GetClosestPointOnTriangle(p1, p2, p3, intersectionPoint);
            // PolyPoint -> colliding object
            const Ray ray(polyPoint, -manifold.normalizedVelocity);
            float dist = ray.HitDistance(sphere);
            if (!IsInfinite(dist))
            {
                distToIntersection = dist;
                intersectionPoint = intersectionPoint + (distToIntersection * -manifold.normalizedVelocity);
            }
        }

        if (sphere.IsInside(intersectionPoint) == Intersection::Inside)
            manifold.stuck = true;

        // Update collision data if we hit something
        if (distToIntersection <= distance)
        {
            if (distToIntersection < manifold.distance)
            {
                manifold.distance = distToIntersection;
                manifold.nearestSphereIntersectionPoint = sphereIntersectionPoint;
                manifold.intersectionPoint = intersectionPoint;
                manifold.normal = planeNormal;
                result = true;
            }
        }
    }
    return result;
}

}
