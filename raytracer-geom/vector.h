#pragma once

#include <array>
#include <cmath>
#include <iostream>
#include <initializer_list>
#include <algorithm>

class Vector {
public:
    Vector() : data_({0, 0, 0}) {
    }
    Vector(std::initializer_list<double> list) {
        int i = 0;
        for (const double* pointer = list.begin(); i < 3 && pointer != list.end(); ++i, ++pointer) {
            data_[i] = *pointer;
        }
        if (i < 3) {
            throw "bad initializer_list size";
        }
    }
    Vector(std::array<double, 3> data) : data_(data) {
    }

    double& operator[](size_t ind) {
        return data_[ind];
    }
    double operator[](size_t ind) const {
        return data_[ind];
    }

    void Normalize() {
        double length = sqrt(data_[0] * data_[0] + data_[1] * data_[1] + data_[2] * data_[2]);
        data_[0] /= length;
        data_[1] /= length;
        data_[2] /= length;
    }

private:
    std::array<double, 3> data_;
};

inline double DotProduct(const Vector& lhs, const Vector& rhs) {
    return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2];
}
inline Vector CrossProduct(const Vector& a, const Vector& b) {
    return Vector(
        {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]});
}
inline double Length(const Vector& vec) {
    return sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

inline Vector operator+(const Vector& a, const Vector& b) {
    return Vector({a[0] + b[0], a[1] + b[1], a[2] + b[2]});
}
inline Vector operator-(const Vector& a, const Vector& b) {
    return Vector({a[0] - b[0], a[1] - b[1], a[2] - b[2]});
}
inline Vector operator*(double a, const Vector& b) {
    return Vector({a * b[0], a * b[1], a * b[2]});
}

inline Vector operator*(const Vector& a, const Vector& b) {
    return {a[0] * b[0], a[1] * b[1], a[2] * b[2]};
}
inline Vector operator/(const Vector& a, const Vector& b) {
    return {a[0] / b[0], a[1] / b[1], a[2] / b[2]};
}