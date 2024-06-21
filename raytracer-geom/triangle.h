#pragma once

#include <vector.h>

class Triangle {
public:
    Triangle(std::initializer_list<Vector> list) {
        int i = 0;
        for (const Vector* pointer = list.begin(); i < 3 && pointer != list.end(); ++i, ++pointer) {
            vertices_[i] = *pointer;
        }
        if (i < 3) {
            throw "bad initializer_list size";
        }
    }
    double Area() const {
        return Length(CrossProduct(vertices_[1] - vertices_[0], vertices_[2] - vertices_[0])) / 2;
    }

    const Vector& GetVertex(size_t ind) const {
        return vertices_[ind];
    }

private:
    std::array<Vector, 3> vertices_;
};