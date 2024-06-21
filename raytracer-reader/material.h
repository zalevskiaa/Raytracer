#pragma once

#include <vector.h>
#include <string>

struct Material {
    Material()
        : ambient_color({0, 0, 0}),
          diffuse_color({0, 0, 0}),
          specular_color({0, 0, 0}),
          intensity({0, 0, 0}),
          albedo({1, 0, 0}) {
    }

    std::string name;
    Vector ambient_color;
    Vector diffuse_color;
    Vector specular_color;
    Vector intensity;  // ?
    double specular_exponent;
    double refraction_index;
    std::array<double, 3> albedo;
};
