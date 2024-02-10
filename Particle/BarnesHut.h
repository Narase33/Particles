#pragma once

#include "Particle.h"
#include "Vector.h"
#include "utils.h"

#include <array>
#include <exception>
#include <format>
#include <iostream>
#include <vector>

class Cell {
    public:
        static std::vector<Cell> _cellList;

        constexpr Cell(const Position& from, const Position& to) :
                _from(from),
                _to(to) {
        }

        constexpr void initializeChildren() {
            if (_cellList.capacity() < (_cellList.size() + 8)) {
                throw std::length_error("");
            }

            const Position half_cell((_to - _from) / 2);
            const Position center(half_cell + _from);
            const Position along_x(half_cell.x, 0.0, 0.0);
            const Position along_y(0.0, half_cell.y, 0.0);
            const Position along_z(0.0, 0.0, half_cell.z);

            // Order important, index to find child is calculated
            addChild<0>(_from, center);
            addChild<1>(_from + along_x, center + along_x);
            addChild<2>(_from + along_y, center + along_y);
            addChild<3>(_from + along_x + along_y, center + along_x + along_y);
            addChild<4>(_from + along_z, center + along_z);
            addChild<5>(_from + along_x + along_z, center + along_x + along_z);
            addChild<6>(_from + along_y + along_z, center + along_y + along_z);
            addChild<7>(center, _to);
        }

        template <size_t index>
        constexpr void addChild(const Position& from, const Position& to) {
            _children[index] = _cellList.size();
            _cellList.emplace_back(from, to);
        }

        Cell& getChild(const Position& pos) {
            const Position relativeParticlePos = pos - _from;
            const Vector3d childCellSize = (_to - _from) / 2;
            const Position coords = (relativeParticlePos / childCellSize); // x, y, z [0;2[,[0;2[,[0;2[
            const int index = static_cast<int>(coords.x) + (static_cast<int>(coords.y) * 2) + (static_cast<int>(coords.z) * 4);
            Cell& c = _cellList[_children[index]];
#if defined(_DEBUG)
            if (!c.isInCell(pos)) {
                throw std::runtime_error("Particle not in Cell");
            }
#endif
            return c;
        }

        constexpr Position centerOfMass() const {
            return _accumulatedCenterOfMass / _mass;
        }

        constexpr bool isLeaf() const {
            return _children.front() == 0;
        }

        constexpr void insert(Particle& p) {
#if defined(_DEBUG)
            if (!isInCell(p.position)) {
                throw std::runtime_error("Particle not in Cell");
            }
#endif
            if (isLeaf()) {
                if (_particle == nullptr) {
                    _particle = &p;
                } else {
                    initializeChildren();

                    getChild(_particle->position).insert(*_particle);
                    getChild(p.position).insert(p);

                    _mass = p.mass + _particle->mass;
                    _accumulatedCenterOfMass = p.toForce() + _particle->toForce();
                    _particle = nullptr;
                }
            } else {
                getChild(p.position).insert(p);

                _mass += p.mass;
                _accumulatedCenterOfMass += p.toForce();
            }
        }

        constexpr double influence(const Position& p) const {
            // return (_to.x - _from.x) / math::distance(centerOfMass(), p);
            return (_to.x - _from.x) * math::invsqrtQuake((centerOfMass() - p).lengthSquared()); // worth?
        }

        constexpr void calculateAcceleration(Particle& p) const {
            if (_mass == 0.0) {
                return;
            }

            if (isLeaf()) {
                if (&p != _particle) {
                    const double distance = math::distance(p.position, _particle->position);

                    if (distance > (p.mass + _particle->mass)) {
                        p.accelerate(_particle->position, _particle->mass);
                    } else {
                        collide(p, *_particle);
                        // re-insert particle?
                    }
                }
            } else if (influence(p.position) < _influenceThreshold) {
                p.accelerate(centerOfMass(), _mass);
            } else {
                for (size_t index : _children) {
                    _cellList[index].calculateAcceleration(p);
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

        void resetCalculation() {
            _accumulatedCenterOfMass = Position(0.0, 0.0, 0.0);
            _mass = 0.0;
            _particle = nullptr;

            for (size_t& index : _children) {
                index = 0;
            }
        }

    private:
        static constexpr double _influenceThreshold = 0.5; // 0.5 is common value across multiple papers

        Position _from;
        Position _to;
        Position _accumulatedCenterOfMass;
        double _mass = 0;

        std::array<size_t, 8> _children{0};
        Particle* _particle = nullptr;
};

class BarnesHut {
    public:
        BarnesHut(const Position& from, const Position& to) :
                _cell(from, to) {
        }

        constexpr void insertParticles(std::vector<Particle>& particles) {
            if (Cell::_cellList.capacity() < (particles.size() * 2)) {
                Cell::_cellList.reserve(particles.size() * 2);
            }

            for (Particle& p : particles) {
                if (Cell::_cellList.capacity() < (Cell::_cellList.size() + 8)) {
                    reserveCellSpace();
                }

#if defined(_DEBUG)
                if (!_cell.isInCell(p.position)) {
                    throw std::runtime_error(std::format("Particle not in OctTree: {}", p.position.toString()));
                }
#endif
                try {
                    _cell.insert(p);
                } catch (std::length_error&) {
                    reserveCellSpace();
                    _cell.insert(p);
                }
            }
        }

        constexpr void calculateAcceleration(Particle& p) const {
            _cell.calculateAcceleration(p);
        }

        void resetCalculation() {
            Cell::_cellList.clear();
            _cell.resetCalculation();
        }

        void reserveCellSpace() {
            Cell::_cellList.reserve(Cell::_cellList.capacity() * 1.5);
        }

    private:
        Cell _cell;
};