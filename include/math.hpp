#pragma once
#include <vector>
#include <iostream>
#include <array>
#include "common.hpp"

struct Vector {
    f32 x, y;

    Vector(f32 x, f32 y) : x(x), y(y) {}
    Vector() : x(0), y(0) {}

    Vector operator-(const Vector& rhs) const {
        return Vector(x - rhs.x, y - rhs.y);
    }

    Vector operator+(const Vector& rhs) const {
        return Vector(x + rhs.x, y + rhs.y);
    }

    Vector operator/(f32 rhs) const {
        return Vector(x / rhs, y / rhs);
    }

    Vector operator*(f32 rhs) const {
        return Vector(x * rhs, y * rhs);
    }

    void operator*=(f32 rhs) {
        x *= rhs;
        y *= rhs;
    }
};

// If slope of AB is greater than slope of AC, then this three points are in clockwise order.
inline bool clockwise(Vector a, Vector b, Vector c) {
    return (b.y - a.y) * (c.x - a.x) > (c.y - a.y) * (b.x - a.x);
}

inline bool intersect(const Vector& a, const Vector& b, const Vector& c, const Vector& d) {
    // If two edges are intersecting, at least one triple is swapped. Algorithm
    // taken from this wise guy:
    // https://bryceboe.com/2006/10/23/line-segment-intersection-algorithm
    return clockwise(a, c, d) != clockwise(b, c, d) && clockwise(a, b, c) != clockwise(a, b, d);
}

// Class for storing approximated objects in a form of polygons (convex or not,
// doesn't matter) for the purpose of collision detection.
struct ObjectContour {
    std::vector<Vector> vertices;

    // A size of the half of "outer rectangle" of the object
    Vector half_of_sides;

    void update_for_dpi(f32 scale_factor) {
        for (auto& vertex : vertices) {
            vertex *= scale_factor;
        }

        half_of_sides *= scale_factor;
    }
};

// Check if the collision may occur by comparing distance of the objects to
// their size. If so, then check every edge pair for collision.
//
// The main advantage of this algorithms in comparison to SAT is it's
// simplicity, so it is easy to implement it at 2 AM.
inline bool intersect(const ObjectContour& lhs, const ObjectContour& rhs,
                      const Vector& lhs_center, const Vector& rhs_center) {
    Vector distance = rhs_center - lhs_center;

    bool safe_distance_on_x = fabsf(distance.x) > lhs.half_of_sides.x + rhs.half_of_sides.x;

    if (safe_distance_on_x)
        return false;

    bool safe_distance_on_y = fabsf(distance.y) > lhs.half_of_sides.y + rhs.half_of_sides.y;

    if (safe_distance_on_y)
        return false;

    // Maybe there is an intersection (maybe not). Check every edge pair.
    for (size_t i = 0; i < lhs.vertices.size(); ++i) {
        const Vector& a = lhs.vertices[i] - distance;
        const Vector& b = lhs.vertices[i + 1 < lhs.vertices.size() ? i + 1 : 0] - distance;

        for (size_t j = 0; j < rhs.vertices.size(); ++j) {
            const Vector& c = rhs.vertices[j];
            const Vector& d = rhs.vertices[j + 1 < rhs.vertices.size() ? j + 1 : 0];

            if (intersect(a, b, c, d))
                return true;
        }
    }

    return false;
}
