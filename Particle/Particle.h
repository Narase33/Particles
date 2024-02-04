#pragma once

#include "Vector.h"

class Particle {
    public:
        Particle(Position position, Vector3d acceleration) noexcept :
                position(position),
                acceleration(acceleration),
                mass(1.0) {
        }

        Particle() noexcept :
                Particle(Position(0.0, 0.0, 0.0), Vector3d(0.0, 0.0, 0.0)) {
        }

        void step() {
            position += acceleration;
        }

        Position position;
        Vector3d acceleration;
        double mass;
};