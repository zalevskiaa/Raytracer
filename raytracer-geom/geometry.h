#include <vector.h>
#include <ray.h>
#include <sphere.h>
#include <intersection.h>
#include <triangle.h>

#include <optional>

std::optional<Intersection> GetIntersection(const Ray& ray, const Sphere& sphere) {
    // solve for tc
    Vector l = sphere.GetCenter() - ray.GetOrigin();
    double tc = DotProduct(l, ray.GetDirection());
    // double tc = (DotProduct(ray.GetDirection(), sphere.GetCenter()) -
    //              DotProduct(ray.GetDirection(), ray.GetOrigin())) /
    //             DotProduct(ray.GetDirection(), ray.GetDirection());
    if (tc < 0) {
        return std::nullopt;
    }
    double l2 = DotProduct(l, l);
    double d2 = l2 - (tc * tc);

    double radius2 = sphere.GetRadius() * sphere.GetRadius();
    if (d2 > radius2) {
        return std::nullopt;
    }

    // solve for t1c
    float t1c = sqrt(radius2 - d2);

    // solve for intersection points
    double t1 = tc - t1c;
    double t2 = tc + t1c;

    if (t1 < 0) {
        std::swap(t1, t2);
    }
    if (t1 < 0) {
        throw "wtf (both t1 and t2 i guess can't be < 0)";
    }
    Vector p1(ray.GetOrigin() + t1 * ray.GetDirection());
    Vector n1(p1 - sphere.GetCenter());
    if (l2 < radius2) {
        // case ray origin is inside the sphere
        n1 = -1 * n1;
    }

    return std::optional<Intersection>({p1, n1, Length(ray.GetOrigin() - p1)});
}

const double kEps = 1e-9;

std::optional<Intersection> GetIntersection(const Ray& ray, const Triangle& triangle) {
    Vector vertex0 = triangle.GetVertex(0);
    Vector vertex1 = triangle.GetVertex(1);
    Vector vertex2 = triangle.GetVertex(2);
    Vector edge1 = vertex1 - vertex0;
    Vector edge2 = vertex2 - vertex0;

    Vector h = CrossProduct(ray.GetDirection(), edge2);
    double a = DotProduct(edge1, h);

    if (a > -kEps && a < kEps) {
        return std::nullopt;  // This ray is parallel to this triangle.
    }

    double f = 1 / a;
    Vector s = ray.GetOrigin() - vertex0;
    double u = f * DotProduct(s, h);
    if (u < 0 || u > 1) {
        return std::nullopt;
    }

    Vector q = CrossProduct(s, edge1);
    double v = f * DotProduct(ray.GetDirection(), q);
    if (v < 0 || u + v > 1) {
        return std::nullopt;
    }
    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = f * DotProduct(edge2, q);
    if (t < 0) {
        // This means that there is a line intersection but not a ray intersection.
        return std::nullopt;
    }
    // ray intersection
    auto position = ray.GetOrigin() + t * ray.GetDirection();
    Vector normal1 = CrossProduct(edge1, edge2);
    Vector normal2 = -1 * normal1;
    // normal1.Normalize();
    // normal2.Normalize();

    if (Length(position + normal1 - ray.GetOrigin()) <
        Length(position + normal2 - ray.GetOrigin())) {
        return Intersection(position, normal1, Length(ray.GetOrigin() - position));
    }
    return Intersection(position, normal2, Length(ray.GetOrigin() - position));
}

std::optional<Vector> Refract(const Vector& ray, const Vector& normal, double eta) {
    double cosine = DotProduct(-1 * normal, ray);
    Vector refract =
        eta * ray + (eta * cosine - sqrt(1 - eta * eta * (1 - cosine * cosine))) * normal;
    if (DotProduct(normal, refract) > 0) {
        // refract back to self (not through).
        // I guess we stop in this case
        return std::nullopt;
    }
    return refract;
}
Vector Reflect(const Vector& ray, const Vector& normal) {
    double cosine = DotProduct(-1 * normal, ray);
    return ray + 2 * cosine * normal;
}
Vector GetBarycentricCoords(const Triangle& triangle, const Vector& point) {
    double bcx = Triangle({triangle.GetVertex(1), triangle.GetVertex(2), point}).Area();
    double cax = Triangle({triangle.GetVertex(2), triangle.GetVertex(0), point}).Area();
    double abx = Triangle({triangle.GetVertex(0), triangle.GetVertex(1), point}).Area();
    double sum = bcx + cax + abx;
    bcx /= sum;
    cax /= sum;
    abx /= sum;
    return {bcx, cax, abx};
}
