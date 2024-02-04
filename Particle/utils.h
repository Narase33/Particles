#pragma once

#include "Vector.h"

namespace math {
    constexpr double pi = 3.1415926535897932385;

    inline constexpr double degreesToRadians(double degrees) {
        return degrees * pi / 180.0;
    }

    template <typename T>
    constexpr double dot(const Vector3<T>& a, const Vector3<T>& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    template <typename T>
    constexpr Vector3<T> cross(const Vector3<T>& a, const Vector3<T>& b) noexcept {
        const T x = a.y * b.z - a.z * b.y;
        const T y = a.z * b.x - a.x * b.z;
        const T z = a.x * b.y - a.y * b.x;
        return Vector3<T>(x, y, z);
    }

    template <typename T>
    constexpr Vector3<T> unitVector(const Vector3<T>& vec) noexcept {
        return vec / vec.length();
    }

    template <typename T>
    constexpr T distance(const Vector3<T>& a, const Vector3<T>& b) {
        return (a - b).length();
    }

    constexpr double invsqrtQuake(double y) {
        double x2 = y * 0.5;
        std::int64_t i = *(std::int64_t*)&y;
        // The magic number is for doubles is from https://cs.uwaterloo.ca/~m32rober/rsqrt.pdf
        i = 0x5fe6eb50c7b537a9 - (i >> 1);
        y = *(double*)&i;
        y = y * (1.5 - (x2 * y * y)); // 1st iteration
        //      y  = y * ( 1.5 - ( x2 * y * y ) );   // 2nd iteration, this can be removed
        return y;
    }
} // namespace math