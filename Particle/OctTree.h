#pragma once

#include "Particle.h"
#include "Vector.h"
#include "utils.h"

#include <array>
#include <format>
#include <iostream>
#include <vector>

class Cell {
    public:
        constexpr Cell(const Position& from, const Position& to) :
                _from(from),
                _to(to) {
        }

        constexpr void initializeChildren() {
            const Position half_cell((_to - _from) / 2);
            const Position center(half_cell + _from);
            const Position along_x(half_cell.x, 0.0, 0.0);
            const Position along_y(0.0, half_cell.y, 0.0);
            const Position along_z(0.0, 0.0, half_cell.z);

            // Order important, index to find child is calculated
            _children[0] = new Cell(_from, center);
            _children[1] = new Cell(_from + along_x, center + along_x);
            _children[2] = new Cell(_from + along_y, center + along_y);
            _children[3] = new Cell(_from + along_x + along_y, center + along_x + along_y);
            _children[4] = new Cell(_from + along_z, center + along_z);
            _children[5] = new Cell(_from + along_x + along_z, center + along_x + along_z);
            _children[6] = new Cell(_from + along_y + along_z, center + along_y + along_z);
            _children[7] = new Cell(center, _to);
        }

        Cell* getChild(const Position& pos) {
            const Position relativeParticlePos = pos - _from;
            const Position childCellSize = (_to - _from) / 2;
            const Position coords = relativeParticlePos / childCellSize; // x, y, z [0;2[,[0;2[,[0;2[
            const int index = static_cast<int>(coords.x) + (static_cast<int>(coords.y) * 2) + (static_cast<int>(coords.z) * 4);
            return _children[index];
        }

        constexpr Position centerOfMass() const {
            return _accumulatedCenterOfMass / _mass;
        }

        constexpr bool isLeaf() const {
            return _children.front() == nullptr;
        }

        constexpr void insert(const Position& pos, double mass) {
            if (isLeaf()) {
                if (_mass != 0.0) {
                    initializeChildren();

                    getChild(pos)->insert(pos, mass);
                    getChild(centerOfMass())->insert(centerOfMass(), mass);
                }
            } else {
                getChild(pos)->insert(pos, mass);
            }

            _mass += mass;
            _accumulatedCenterOfMass += pos * mass;
        }

        void computeAcceleration(Particle& p) const {
            constexpr double G = 0.001;

            const Position _centerOfMass = centerOfMass();
            const Vector3d delta = _centerOfMass - p.position;
            const double force = (G * p.mass * _mass) / (delta.x * delta.x + delta.y * delta.y + delta.z * delta.z + 1);
            const Vector3d acceleration = delta * force;

            p.acceleration += acceleration;

#if defined(_DEBUG)
            if (std::isnan(p.acceleration.x) or std::isnan(p.acceleration.y) or std::isnan(p.acceleration.z)) {
                throw std::runtime_error("Is NAN");
            }
#endif
        }

        constexpr double influence(const Position& p) const {
            // return (_to.x - _from.x) / math::distance(centerOfMass(), p);
            return (_to.x - _from.x) * math::invsqrtQuake((centerOfMass() - p).lengthSquared()); // worth?
        }

        constexpr void calculateAcceleration(Particle& p) const {
            if (_mass == 0.0) {
                return;
            }

            if (isLeaf() or (influence(p.position) < _influenceThreshold)) {
                computeAcceleration(p);
            } else {
                for (Cell* c : _children) {
                    c->calculateAcceleration(p);
                }
            }
        }

        constexpr bool isInCell(const Position& pos) const {
            const bool in_x = (_from.x < pos.x) and (pos.x < _to.x);
            const bool in_y = (_from.y < pos.y) and (pos.y < _to.y);
            const bool in_z = (_from.z < pos.z) and (pos.z < _to.z);
            return in_x and in_y and in_z;
        }

        std::string toString() const {
            return std::format("[From: {}, To: {}, CenterOfMass: {}, Mass: {}]", _from.toString(), _to.toString(), centerOfMass().toString(), _mass);
        }

        void removeChildren() {
            for (int i = 0; i < 8; i++) {
                delete _children[i];
                _children[i] = nullptr;
            }
        }

        ~Cell() {
            removeChildren();
        }

    private:
        static constexpr double _influenceThreshold = 0.5; // 0.5 is common value across multiple papers

        Position _from;
        Position _to;
        Position _accumulatedCenterOfMass;
        double _mass = 0;

        std::array<Cell*, 8> _children{nullptr};
};

class OctTree {
    public:
        OctTree(const Position& from, const Position& to) :
                _cell(from, to) {
        }

        constexpr void insertParticles(const std::vector<Particle>& particles) {
            for (const Particle& p : particles) {
#if defined(_DEBUG)
                if (!_cell.isInCell(p.position)) {
                    throw std::runtime_error(std::format("Particle not in OctTree: {}", p.position.toString()));
                }
#endif
                _cell.insert(p.position, p.mass);
            }
        }

        constexpr void calculateAcceleration(Particle& p) const {
            _cell.calculateAcceleration(p);
        }

        void resetCalculation() {
            _cell.removeChildren();
        }

    private:
        Cell _cell;
};