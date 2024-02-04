#pragma once

#include <cmath>
#include <cstdint>
#include <format>

template <typename T>
struct Vector2 {
        T x, y;

        constexpr Vector2() :
                Vector2({}, {}) {
        }

        constexpr Vector2(T x, T y) :
                x(x),
                y(y) {
        }

        template <typename T0>
        constexpr Vector2<T>& operator+=(const Vector2<T0>& other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        template <typename T0>
        constexpr Vector2<T>& operator+=(T0 other) {
            x += other;
            y += other;
            return *this;
        }

        template <typename T0>
        constexpr Vector2<T>& operator*=(T0 other) {
            x *= other;
            y *= other;
            return *this;
        }

        constexpr Vector2<T>& operator-=(const Vector2<T>& other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }
};

template <typename T>
constexpr Vector2<T> operator+(const Vector2<T>& a, const Vector2<T>& b) {
    return {a.x + b.x, a.y + b.y};
}

template <typename T>
constexpr Vector2<T> operator-(const Vector2<T>& a, const Vector2<T>& b) {
    return {a.x - b.x, a.y - b.y};
}

template <typename T>
constexpr Vector2<T> operator*(const Vector2<T>& a, T b) {
    return {a.x * b, a.y * b};
}

template <typename T0, typename T1>
constexpr Vector2<T0> operator/(const Vector2<T0>& a, T1 b) {
    return {a.x / b, a.y / b};
}

using Vector2f = Vector2<float>;
using Vector2d = Vector2<double>;
using Vector2u = Vector2<uint16_t>;

template <typename T>
struct Vector3 {
        T x, y, z;

        constexpr Vector3() :
                Vector3({}, {}, {}) {
        }

        constexpr Vector3(T x, T y, T z) :
                x(x),
                y(y),
                z(z) {
        }

        template <typename T0>
        constexpr Vector3<T>& operator+=(const Vector3<T0>& other) {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }

        template <typename T0>
        constexpr Vector3<T>& operator+=(T0 other) {
            x += other;
            y += other;
            z += other;
            return *this;
        }

        template <typename T0>
        constexpr Vector3<T>& operator*=(T0 other) {
            x *= other;
            y *= other;
            z *= other;
            return *this;
        }

        constexpr Vector3<T>& operator-=(const Vector3<T>& other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
        }

        constexpr double lengthSquared() const noexcept {
            return x * x + y * y + z * z;
        }

        constexpr double length() const noexcept {
            return std::sqrt(lengthSquared());
        }

        std::string toString() const {
            return std::format("[{},{},{}]", x, y, z);
        }
};

template <typename T>
constexpr Vector3<T> operator+(const Vector3<T>& a, const Vector3<T>& b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

template <typename T>
constexpr Vector3<T> operator-(const Vector3<T>& a, const Vector3<T>& b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

template <typename T>
constexpr Vector3<T> operator*(const Vector3<T>& a, T b) {
    return {a.x * b, a.y * b, a.z * b};
}

template <typename T>
constexpr Vector3<T> operator*(T a, const Vector3<T>& b) {
    return {a * b.x, a * b.y, a * b.z};
}

template <typename T0, typename T1>
constexpr Vector3<T0> operator/(const Vector3<T0>& a, T1 b) {
    return {a.x / b, a.y / b, a.z / b};
}

using Vector3i = Vector3<int>;
using Vector3d = Vector3<double>;
using Position = Vector3d;
using Color = Vector3<uint8_t>;