#pragma once

#include "Tensor.h"
#include "Vector.h"
#include "utils.h"

#include <cassert>
#include <iostream>
#include <mutex>

class Particle {
    public:
        Particle(const Position& position, const Vector3d& velocity, const Vector3d& spin, double mass) {
            _position[0] = position;
            _position[1] = position;

            _velocity[0] = velocity;
            _velocity[1] = velocity;

            _spin[0] = spin;
            _spin[1] = spin;

            _mass[0] = mass;
            _mass[1] = mass;
        }

        Particle(const Position& position, const Vector3d& velocity) noexcept :
                Particle(position, velocity, Vector3d(0.0, 0.0, 0.0), 1.0) {
        }

        Particle() noexcept :
                Particle(Position(0.0, 0.0, 0.0), Vector3d(0.0, 0.0, 0.0)) {
        }

        Particle(const Particle& other) = default;
        Particle& operator=(const Particle& other) = default;

        constexpr void step() {
            _velocity[1] += _addedAcceleration[1];
            _addedAcceleration[1] = Vector3d(0.0, 0.0, 0.0);
            _position[1] += _velocity[1];

            _position[0] = _position[1];
            _velocity[0] = _velocity[1];
            _spin[0] = _spin[1];
            _mass[0] = _mass[1];
        }

        void accelerate(const Position& pos_other, double mass_other) {
            constexpr double G = 0.001;

            const Vector3d delta = pos_other - position();
            const double force = (G * mass() * mass_other) / (delta.lengthSquared() + 1);
            const Vector3d acceleration = (delta * force) / mass() * mass_other;

            // assert(!std::isnan(acceleration.x));
            // assert(!std::isnan(acceleration.y));
            // assert(!std::isnan(acceleration.z));

            _addedAcceleration[1] += acceleration;
        }

        void collide_(const Particle& b) {
            // https://exploratoria.github.io/exhibits/mechanics/elastic-collisions-in-3d/
            const Vector3d normal = math::normal(position(), b.position());
            const Vector3d relativeVelocity = velocity() - b.velocity();
            const double dot = math::dot(relativeVelocity, normal);
            const Vector3d work = normal * dot;

            _velocity[1] = _velocity[0] - work;
        }

        void collide(Particle& b) {
            if (!isEnabled() or !b.isEnabled()) {
                return;
            }

            const double angleBetween = math::radiansToDegrees(math::angleBetween(velocity(), b.velocity()));
            if (angleBetween > 90) {
                bounce(b);
            } else {
                merge(b);
            }
        }

        void bounce_(const Particle& b) {
            constexpr double mu = 0.5;

            const double r1 = radius();
            const double r2 = b.radius();

            const Vector3d normalizedPositionDelta = math::unitVector(position() - b.position());

            const Vector3d v_rel = velocity() - b.velocity();
            const Vector3d w_rel = math::cross(spin(), Vector3d{r1, r1, r1}) - math::cross(b.spin(), Vector3d{r2, r2, r2});
            const Vector3d v_contact = v_rel + w_rel;

            const double v_rel_n = math::dot(v_contact, normalizedPositionDelta);

            if (v_rel_n > 0) {
                return; // No collision if velocities are separating
            }

            const double impulse = (2 * v_rel_n) / (mass() + b.mass());
            const Vector3d impulseVec = normalizedPositionDelta * impulse;

            // Update linear velocities
            const Vector3d newVelocity = velocity() - impulseVec * b.mass();

            // Calculate friction impulse
            Vector3d t = v_contact - (normalizedPositionDelta * v_rel_n);
            if (t.x != 0 || t.y != 0 || t.z != 0) {
                t = math::unitVector(t);
                const double jt = math::dot(t, v_contact) * mu;

                const Vector3d frictionImpulse = t * jt;
                _velocity[1] = newVelocity - frictionImpulse * b.mass();

                // Update angular velocities
                _spin[1] = spin() + math::cross(frictionImpulse, Vector3d{r1, r1, r1}) * (5 / (2 * mass() * r1 * r1));
            } else {
                _velocity[1] = newVelocity;
            }
        }

        void bounce(Particle& b) {
            const Vector3d Vrel = b.velocity() - velocity(); // Org from site?
            const double e = 1;
            const Vector3d n = math::normal(position(), b.position());
            const Vector3d r1 = radius() * n; // ChatGPT: d=pos2-pos1; |d|=d.lenghth(); ^d=d/|d|; r1=radius1*^d
            const Vector3d r2 = b.radius() * n;
            const Tensor3d I1 = createInertiaTensor();
            const Tensor3d I2 = b.createInertiaTensor();
            const Vector3d J = (-Vrel * (1 + e)) / (1 / mass() + 1 / b.mass() + n.dot((r1.cross(n) / I1).cross(r1)) + n.dot((r2.cross(n) / I2).cross(r2)));

            _velocity[0] = velocity() + (J * n) / mass();
            _spin[0] = spin() + (r1.cross(J * n)) / I1;
        }

        void merge(Particle& b) {
            _position[1] = (b.position() - position()) / 2.0 + position();
            _velocity[1] = (velocity() * mass() + b.velocity() * b.mass()) / (mass() + b.mass());
            _mass[1] = mass() + b.mass();
            b._enabled = false;
        }

        constexpr Position toForce() const {
            return position() * mass();
        }

        const Position& position() const {
            return _position[0];
        }

        const Vector3d& velocity() const {
            return _velocity[0];
        }
        const Vector3d& spin() const {
            return _spin[0];
        }

        double radius() const {
            return std::cbrt((3 * mass()) / (4 * std::numbers::pi));
        }

        double mass() const {
            return _mass[0];
        }

        double invMass() const {
            return 1.0 / mass();
        }

        constexpr double elasticity() const {
            return 0.5;
        }

        Tensor3d createInertiaTensor() const {
            const double inertia = (2.0 / 5.0) * mass() * radius() * radius();
            return inertia * Tensor3d({
                                 {
                                  {1.0, 0.0, 0.0},
                                  {0.0, 1.0, 0.0},
                                  {0.0, 0.0, 1.0},
                                  }
            });
        }

        bool isEnabled() const {
            return _enabled;
        }

    private:
        Position _position[2];
        Vector3d _addedAcceleration[2]{};
        Vector3d _velocity[2];
        Vector3d _spin[2]; // rad/s ; x,y,z-axis
        double _mass[2];

        bool _enabled = true;
};