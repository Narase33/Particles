#pragma once

#include "Vector.h"

#include <array>

template <typename T>
class Tensor3 {
        using Type = std::array<std::array<T, 3>, 3>;

    public:
        Tensor3(Type data) :
                _data(std::move(data)) {
        }

        T get(size_t i, size_t j) const {
            return _data[i][j];
        }

        T& get(size_t i, size_t j) {
            return _data[i][j];
        }

        T calculateDeterminant() const {
            return get(0, 0) * get(1, 1) * get(2, 2) + get(0, 1) * get(1, 2) * get(2, 0) + get(0, 2) * get(1, 0) * get(2, 1) - get(0, 2) * get(1, 1) * get(2, 0) - get(0, 0) * get(1, 2) * get(2, 1) -
                   get(0, 1) * get(1, 0) * get(2, 2);
        }

        Tensor3 calculateInverse() const {
            return (1.0 / calculateDeterminant()) *
                   Tensor3({
                       {
                        {get(1, 1) * get(2, 2) - get(1, 2) * get(2, 1), get(0, 2) + get(2, 1) - get(0, 1) + get(2, 2), get(0, 1) * get(1, 2) - get(0, 2) * get(1, 1)},
                        {get(1, 2) * get(2, 0) - get(1, 0) * get(2, 2), get(0, 0) + get(2, 2) - get(0, 2) + get(2, 0), get(0, 2) * get(1, 0) - get(0, 0) * get(1, 2)},
                        {get(1, 0) * get(2, 1) - get(1, 1) * get(2, 0), get(0, 1) + get(2, 0) - get(0, 0) + get(2, 1), get(0, 0) * get(1, 1) - get(0, 1) * get(1, 0)},
                        }
            });
        }

    private:
        Type _data;
};

template <typename T>
constexpr Tensor3<T> operator*(Tensor3<T> tensor, T value) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            tensor.get(i, j) *= value;
        }
    }
    return tensor;
}

template <typename T>
constexpr Tensor3<T> operator*(T value, Tensor3<T> tensor) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            tensor.get(i, j) *= value;
        }
    }
    return tensor;
}

template <typename T>
constexpr Vector3<T> operator*(const Vector3<T>& vector, const Tensor3<T>& tensor) {
    const T x = tensor.get(0, 0) * vector.x + tensor.get(0, 1) * vector.y + tensor.get(0, 2) * vector.z;
    const T y = tensor.get(1, 0) * vector.x + tensor.get(1, 1) * vector.y + tensor.get(1, 2) * vector.z;
    const T z = tensor.get(2, 0) * vector.x + tensor.get(2, 1) * vector.y + tensor.get(2, 2) * vector.z;
    return Vector3<T>(x, y, z);
}

template <typename T>
constexpr Vector3<T> operator*(const Tensor3<T>& tensor, const Vector3<T>& vector) {
    const T x = tensor.get(0, 0) * vector.x + tensor.get(0, 1) * vector.y + tensor.get(0, 2) * vector.z;
    const T y = tensor.get(1, 0) * vector.x + tensor.get(1, 1) * vector.y + tensor.get(1, 2) * vector.z;
    const T z = tensor.get(2, 0) * vector.x + tensor.get(2, 1) * vector.y + tensor.get(2, 2) * vector.z;
    return Vector3<T>(x, y, z);
}

template <typename T>
constexpr Vector3<T> operator/(const Vector3<T>& vector, const Tensor3<T>& tensor) {
    return vector * tensor.calculateInverse();
}

using Tensor3d = Tensor3<double>;