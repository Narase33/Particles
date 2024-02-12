#pragma once

#include "Vector.h"
#include <iostream>

class Particle {
    public:
        constexpr Particle(Position position, Vector3d speed) noexcept :
                position(position),
                speed(speed),
                newSpeed(speed),
                enabled(true),
                mass(1.0) {
        }

        constexpr Particle() noexcept :
                Particle(Position(0.0, 0.0, 0.0), Vector3d(0.0, 0.0, 0.0)) {
        }

        constexpr void step() {
            speed = newSpeed;
            position += speed;
        }

        void accelerate(const Position& pos_other, double mass_other) {
            constexpr double G = 0.001;

            const Vector3d delta = pos_other - position;
            const double force = (G * mass * mass_other) / (delta.x * delta.x + delta.y * delta.y + delta.z * delta.z + 1);
            const Vector3d acceleration = delta * force;

            newSpeed = newSpeed + acceleration;

#if defined(_DEBUG)
            if (std::isnan(newSpeed.x) or std::isnan(newSpeed.y) or std::isnan(newSpeed.z)) {
                throw std::runtime_error("Is NAN");
            }
#endif
        }

        void collide(const Particle& b) {
            // https://exploratoria.github.io/exhibits/mechanics/elastic-collisions-in-3d/
            const Vector3d normal = math::unitVector(position - b.position);
            const Vector3d relativeVelocity = speed - b.speed;
            const double dot = math::dot(relativeVelocity, normal);
            const Vector3d normal2 = normal * dot;

            newSpeed = newSpeed - normal2;
        }

        constexpr Position toForce() const {
            return position * mass;
        }

        Position position;
        Vector3d newSpeed;
        Vector3d spin;
        double mass;
        bool enabled;

    private:
        Vector3d speed;
};