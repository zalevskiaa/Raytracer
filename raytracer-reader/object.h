#pragma once

#include <triangle.h>
#include <material.h>
#include <sphere.h>

struct Object {
    const Vector* GetNormal(size_t index) const {
        return &normal.GetVertex(index);
    }

    const Material* material = nullptr;
    Triangle polygon;
    Triangle texture;
    Triangle normal;
};

struct SphereObject {
    const Material* material = nullptr;
    Sphere sphere;
};
