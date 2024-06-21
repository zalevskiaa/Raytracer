#pragma once

#include <vector.h>
#include <triangle.h>

struct Light {
    Vector position;
    Vector intensity;

    Light(const Vector& position, const Vector& intensity)
        : position(position), intensity(intensity) {
    }
};
