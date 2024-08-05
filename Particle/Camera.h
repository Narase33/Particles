#pragma once

#include "Vector.h"
#include "utils.h"

#include <limits>

class Camera {
    public:
        Camera(const Vector3d& displaySurface, const Vector2d offset, double fov) :
                _displaySurface_org(displaySurface),
                _offset_org(offset),
                _displaySurface(displaySurface),
                _offset(offset),
                _fov(math::degreesToRadians(fov)) {
        }

        Vector3d turn(const Position& particle) const {
            return rotate_x_axis(_turn.x, rotate_y_axis(_turn.y, particle));
        }

        Vector2d project(const Position& turnedPosition) const {
            const double screenParticleDelta = _displaySurface.z - turnedPosition.z;
            const double f = 1 / std::tan(_fov / 2);
            const double aspectRatio = _displaySurface.x / _displaySurface.y;

            Vector2d out;
            out.x = (aspectRatio * f * turnedPosition.x) / screenParticleDelta;
            out.y = (f * turnedPosition.y) / screenParticleDelta;

            out += 1.0;
            out *= 0.5;

            out.x *= _displaySurface.x;
            out.y *= _displaySurface.y;

            out.x += _offset.x;
            out.y += _offset.y;

            return out;
        }

        void move(const Vector2d& direction) {
            _offset += direction;
        }

        void zoom(double amount) {
            double newZ = _displaySurface.z + amount;
            if (newZ < 0) {
                _displaySurface.z = newZ;
            }
        }

        Vector3d rotate_x_axis(double degree_x, Vector3d particle) const {
            const double radians = math::degreesToRadians(degree_x);
            const double sinTheta = std::sin(radians);
            const double cosTheta = std::cos(radians);

            const double new_y = cosTheta * particle.y - sinTheta * particle.z;
            const double new_z = cosTheta * particle.z + sinTheta * particle.y;
            return Vector3d(particle.x, new_y, new_z);
        }

        Vector3d rotate_y_axis(double degree_y, Vector3d particle) const {
            const double radians = math::degreesToRadians(degree_y);
            const double sinTheta = std::sin(radians);
            const double cosTheta = std::cos(radians);

            const double new_x = cosTheta * particle.x + sinTheta * particle.z;
            const double new_z = cosTheta * particle.z - sinTheta * particle.x;
            return Vector3d(new_x, particle.y, new_z);
        }

        Vector3d rotate_z_axis(double degree_x, Vector3d particle) const {
            const double radians = math::degreesToRadians(degree_x);
            const double sinTheta = std::sin(radians);
            const double cosTheta = std::cos(radians);

            const double new_x = cosTheta * particle.x - sinTheta * particle.y;
            const double new_y = cosTheta * particle.y + sinTheta * particle.x;
            return Vector3d(new_x, new_y, particle.z);
        }

        void turn(Vector2d direction) {
            _turn += direction;
        }

        void reset() {
            _displaySurface = _displaySurface_org;
            _offset = _offset_org;
            _turn = {};
        }

        const Vector3d& getDisplaySurface() const {
            return _displaySurface;
        }

        double getFov() const {
            return _fov;
        }

    private:
        const Vector3d _displaySurface_org;
        const Vector2d _offset_org;

        Vector3d _displaySurface;
        Vector2d _offset;
        Vector2d _turn;
        double _fov;
};