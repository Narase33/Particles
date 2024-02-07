#pragma once

#include "Vector.h"

class Particle {
    public:
        constexpr Particle(Position position, Vector3d speed) noexcept :
                position(position),
                speed(speed),
                mass(1.0) {
        }

        constexpr Particle() noexcept :
                Particle(Position(0.0, 0.0, 0.0), Vector3d(0.0, 0.0, 0.0)) {
        }

        constexpr void step() {
            position += speed;
        }

        void accelerate(const Position& pos_other, double mass_other) {
            constexpr double G = 0.001;

            const Vector3d delta = pos_other - position;
            const double force = (G * mass * mass_other) / (delta.x * delta.x + delta.y * delta.y + delta.z * delta.z + 1);
            const Vector3d acceleration = delta * force;

            speed += acceleration;

#if defined(_DEBUG)
            if (std::isnan(speed.x) or std::isnan(speed.y) or std::isnan(speed.z)) {
                throw std::runtime_error("Is NAN");
            }
#endif
        }

        constexpr Position toForce() const {
            return position * mass;
        }

        Position position;
        Vector3d speed;
        Vector3d spin;
        double mass;
};

inline void collide(Particle&, Particle&) {
    // TODO
}