#pragma once

#include "Vector.h"
#include "utils.h"

#include <cassert>
#include <iostream>
#include <mutex>

class Particle {
    public:
        struct Attributes {
                Position position;
                Vector3d speed;
                Vector3d spin;
                double mass;
        };

        explicit Particle(const Attributes& attributes) noexcept :
                nextAttributes(attributes),
                currentAttributes(attributes) {
        }

        Particle(const Position& position, const Vector3d& speed) noexcept :
                Particle(Attributes{position, speed, Vector3d(0.0, 0.0, 0.0), 1.0}) {
        }

        Particle() noexcept :
                Particle(Position(0.0, 0.0, 0.0), Vector3d(0.0, 0.0, 0.0)) {
        }

        Particle(const Particle& other) :
                nextAttributes(other.nextAttributes),
                currentAttributes(other.currentAttributes),
                enabled(other.enabled) {
        }

        Particle& operator=(const Particle& other) {
            nextAttributes = other.nextAttributes;
            currentAttributes = other.currentAttributes;
            enabled = other.enabled;
            return *this;
        }

        constexpr void step() {
            nextAttributes.position += nextAttributes.speed;
            currentAttributes = nextAttributes;
        }

        void accelerate(const Position& pos_other, double mass_other) {
            constexpr double G = 0.001;

            const Vector3d delta = pos_other - position();
            const double force = (G * mass() * mass_other) / (delta.lengthSquared() + 1);
            const Vector3d acceleration = delta * force;

            nextAttributes.speed += acceleration;

            assert(!std::isnan(acceleration.x));
            assert(!std::isnan(acceleration.y));
            assert(!std::isnan(acceleration.z));
        }

        void collide_(const Particle& b) {
            // https://exploratoria.github.io/exhibits/mechanics/elastic-collisions-in-3d/
            const Vector3d normal = math::normal(position(), b.position());
            const Vector3d relativeVelocity = speed() - b.speed();
            const double dot = math::dot(relativeVelocity, normal);
            const Vector3d work = normal * dot;

            nextAttributes.speed = nextAttributes.speed - work;
        }

        void collide(Particle& b) {
            const double angleBetween = math::radiansToDegrees(math::angleBetween(speed(), b.speed()));
            if (angleBetween > 90) {
                bounce(b);
            } else {
                merge(b);
            }
        }

        void bounce(Particle& b) {
            std::lock(_criticalSection, b._criticalSection);
            const std::lock_guard lock(_criticalSection, std::adopt_lock);
            const std::lock_guard lock_b(b._criticalSection, std::adopt_lock);

            if (!isEnabled() or !b.isEnabled()) {
                return;
            }

            // https://exploratoria.github.io/exhibits/mechanics/elastic-collisions-in-3d/
            const Vector3d normal = math::normal(position(), b.position());
            const Vector3d relativeVelocity = speed() - b.speed();
            const double dot = math::dot(relativeVelocity, normal);
            const Vector3d work = normal * dot;

            nextAttributes.speed = nextAttributes.speed - work;
        }

        void merge(Particle& b) {
            std::lock(_criticalSection, b._criticalSection);
            const std::lock_guard lock(_criticalSection, std::adopt_lock);
            const std::lock_guard lock_b(b._criticalSection, std::adopt_lock);

            if (!isEnabled() or !b.isEnabled()) {
                return;
            }

            nextAttributes.position = (b.position() - position()) / 2.0 + position();
            nextAttributes.speed = (speed() * mass() + b.speed() * b.mass()) / (mass() + b.mass());
            nextAttributes.mass = mass() + b.mass();
            b.enabled = false;
        }

        constexpr Position toForce() const {
            return position() * mass();
        }

        const Position& position() const {
            return currentAttributes.position;
        }

        const Vector3d& speed() const {
            return currentAttributes.speed;
        }
        const Vector3d& spin() const {
            return currentAttributes.spin;
        }

        double mass() const {
            return currentAttributes.mass;
        }

        bool isEnabled() const {
            return enabled;
        }

    private:
        Attributes nextAttributes;
        Attributes currentAttributes;
        bool enabled = true;
        std::mutex _criticalSection;
};