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

        constexpr void addNextDepth(std::vector<Cell>& cellList, uint16_t depth) {
            if (!isLeaf()) {
                for (Cell* c : _children) {
                    c->addNextDepth(cellList, depth);
                }
            } else {
                const Position half_cell((_to - _from) / 2);
                const Position center(half_cell + _from);
                const Position along_x(half_cell.x, 0.0, 0.0);
                const Position along_y(0.0, half_cell.y, 0.0);
                const Position along_z(0.0, 0.0, half_cell.z);

                _children[0] = &cellList.emplace_back(_from, center);
                _children[1] = &cellList.emplace_back(_from + along_x, center + along_x);
                _children[2] = &cellList.emplace_back(_from + along_y, center + along_y);
                _children[3] = &cellList.emplace_back(_from + along_z, center + along_z);
                _children[4] = &cellList.emplace_back(_from + along_x + along_y, center + along_x + along_y);
                _children[5] = &cellList.emplace_back(_from + along_x + along_z, center + along_x + along_z);
                _children[6] = &cellList.emplace_back(_from + along_y + along_z, center + along_y + along_z);
                _children[7] = &cellList.emplace_back(center, _to);
            }
        }

        constexpr Position centerOfMass() const {
            return _accumulatedCenterOfMass / _mass;
        }

        constexpr bool isLeaf() const {
            return _children[0] == nullptr;
        }

        constexpr void insertParticle(const Particle& p) {
            _mass += p.mass;
            _accumulatedCenterOfMass += p.position * p.mass;

            if (!isLeaf()) {
                for (Cell* cell : _children) {
                    if (cell->isInCell(p.position)) {
                        cell->insertParticle(p);
                        break;
                    }
                }
            }
        }

        void computeAttraction(Particle& p) const {
            constexpr double G = 0.001;

            const Vector3d delta = centerOfMass() - p.position;
            const double force = (G * p.mass * _mass) / (delta.x * delta.x + delta.y * delta.y + delta.z * delta.z + 1);
            const Vector3d acceleration = delta * force;

            p.acceleration += acceleration;

#if defined(_DEBUG)
            if (std::isnan(p.acceleration.x) or std::isnan(p.acceleration.y) or std::isnan(p.acceleration.z)) {
                throw std::runtime_error("Is NAN");
            }
#endif
        }

        constexpr void resetCalculations() {
            _mass = 0.0;
            _accumulatedCenterOfMass = Position(0.0, 0.0, 0.0);
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
                computeAttraction(p);
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
        OctTree(const Position& from, const Position& to, int depth) {
            _cellList.reserve(numberOfNodes(depth));
            std::print(std::cout, "Start creating OctTree with {} nodes\n", _cellList.capacity());
            _cellList.emplace_back(from, to);
            for (int i = 1; i < depth; i++) {
                _cellList.front().addNextDepth(_cellList, i);
            }
            std::print(std::cout, "Finished creating OctTree\nSmallest cell is {} wide\n", std::abs((from - to).x) / std::pow(2, depth - 1));
        }

        constexpr void insertParticles(const std::vector<Particle>& particles) {
            for (const Particle& p : particles) {
#if defined(_DEBUG)
                if (!_cellList.front().isInCell(p.position)) {
                    throw std::runtime_error(std::format("Particle not in OctTree: {}", p.position.toString()));
                }
#endif
                _cellList.front().insertParticle(p);
            }
        }

        constexpr void calculateAcceleration(Particle& p) const {
            _cellList.front().calculateAcceleration(p);
        }

        constexpr void resetCalculation() {
            for (Cell& c : _cellList) {
                c.resetCalculations();
            }
        }

    private:
        std::vector<Cell> _cellList;

        constexpr size_t numberOfNodes(int depth) const {
            size_t sum = 0;
            for (int i = 0; i < depth; i++) {
                sum += std::pow(8, i);
            }
            return sum;
        }
};