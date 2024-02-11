#pragma once

#include "Particle.h"
#include "Vector.h"
#include "utils.h"

#include <array>
#include <exception>
#include <format>
#include <iostream>
#include <vector>

struct Node {
        Position from;
        Position to;
        Position accumulatedCenterOfMass;
        double mass = 0;

        std::array<size_t, 8> children{0};
        Particle* particle = nullptr;

        constexpr Node(const Position& from, const Position& to) :
                from(from),
                to(to) {
        }

        constexpr double influence(const Position& p) const {
            // return (to.x - from.x) / math::distance(centerOfMass(), p);
            return (to.x - from.x) * math::invsqrtQuake((centerOfMass() - p).lengthSquared()); // worth?
        }

        constexpr Position centerOfMass() const {
            return accumulatedCenterOfMass / mass;
        }

        constexpr bool isLeaf() const {
            return children.front() == 0;
        }

        std::string toString() const {
            return std::format("[From: {}, To: {}, CenterOfMass: {}, Mass: {}]", from.toString(), to.toString(), centerOfMass().toString(), mass);
        }

        constexpr bool isInCell(const Position& pos) const {
            const bool in_x = (from.x < pos.x) and (pos.x < to.x);
            const bool in_y = (from.y < pos.y) and (pos.y < to.y);
            const bool in_z = (from.z < pos.z) and (pos.z < to.z);
            return in_x and in_y and in_z;
        }

        size_t getChildIndex(const Position& pos) {
            const Position relativeParticlePos = pos - from;
            const Vector3d childCellSize = (to - from) / 2;
            const Position coords = (relativeParticlePos / childCellSize); // x, y, z [0;2[,[0;2[,[0;2[
            const int index = static_cast<int>(coords.x) + (static_cast<int>(coords.y) * 2) + (static_cast<int>(coords.z) * 4);
            const size_t childIndex = children[index];
#if defined(_DEBUG)
            if (!isInCell(pos)) {
                throw std::runtime_error("Particle not in Cell");
            }
#endif
            return childIndex;
        }
};

class BarnesHut {
        static constexpr double influenceThreshold = 0.5; // 0.5 is common value across multiple papers
        static constexpr bool withCollision = true;

    public:
        BarnesHut(const Position& from, const Position& to) {
            _nodes.emplace_back(from, to);
        }

        constexpr void insertParticles(std::vector<Particle>& particles) {
            if (_nodes.capacity() < (particles.size() * 2)) {
                _nodes.reserve(particles.size() * 2);
            }

            for (Particle& p : particles) {
#if defined(_DEBUG)
                if (!_nodes[0].isInCell(p.position)) {
                    throw std::runtime_error(std::format("Particle not in OctTree: {}", p.position.toString()));
                }
#endif
                insert(0, p);
            }
        }

        constexpr void calculateAcceleration(Particle& p) const {
            calculateAcceleration(0, p);
        }

        void resetCalculation() {
            _nodes.erase(_nodes.begin() + 1, _nodes.end());
            _nodes.front().accumulatedCenterOfMass = Position(0.0, 0.0, 0.0);
            _nodes.front().accumulatedCenterOfMass = Position(0.0, 0.0, 0.0);
            _nodes.front().mass = 0.0;
            _nodes.front().particle = nullptr;

            for (size_t& index : _nodes.front().children) {
                index = 0;
            }
        }

    private:
        std::vector<Node> _nodes;

        constexpr void insert(size_t index, Particle& p) {
            Node* currentNode = &_nodes[index];

#if defined(_DEBUG)
            if (!currentNode->isInCell(p.position)) {
                throw std::runtime_error("Particle not in Cell");
            }
#endif
            if (currentNode->isLeaf()) {
                if (currentNode->particle == nullptr) {
                    currentNode->particle = &p;
                } else {
                    initializeChildrenForNode(index);
                    currentNode = &_nodes[index];

                    const size_t firstChildIndex = currentNode->getChildIndex(currentNode->particle->position);
                    const size_t secondChildIndex = currentNode->getChildIndex(p.position);

                    insert(firstChildIndex, *_nodes[index].particle);
                    insert(secondChildIndex, p);
                    currentNode = &_nodes[index];

                    currentNode->mass = p.mass + currentNode->particle->mass;
                    currentNode->accumulatedCenterOfMass = p.toForce() + currentNode->particle->toForce();
                    currentNode->particle = nullptr;
                }
            } else {
                const size_t childIndex = currentNode->getChildIndex(p.position);
                currentNode->mass += p.mass;
                currentNode->accumulatedCenterOfMass += p.toForce();

                insert(childIndex, p);
            }
        }

        constexpr void calculateAcceleration(size_t index, Particle& p) const {
            const Node& currentNode = _nodes[index];

            if (currentNode.mass == 0.0) {
                return;
            }

            if (currentNode.isLeaf()) {
                if (&p != currentNode.particle) {
                    const double distance = math::distance(p.position, currentNode.particle->position);

                    if (distance > (p.mass + currentNode.particle->mass)) {
                        p.accelerate(currentNode.particle->position, currentNode.particle->mass);
                    } else {
                        if constexpr (withCollision) {
                            collide(p, *currentNode.particle);
                            // re-insert particle?
                        }
                    }
                }
            } else if (currentNode.influence(p.position) < influenceThreshold) {
                p.accelerate(currentNode.centerOfMass(), currentNode.mass);
            } else {
                for (size_t childIndex : currentNode.children) {
                    calculateAcceleration(childIndex, p);
                }
            }
        }

        constexpr void initializeChildrenForNode(size_t index) {
            const Position from = _nodes[index].from;
            const Position to = _nodes[index].to;

            const Position half_cell((to - from) / 2);
            const Position center(half_cell + from);

            const Position along_x(half_cell.x, 0.0, 0.0);
            const Position along_y(0.0, half_cell.y, 0.0);
            const Position along_z(0.0, 0.0, half_cell.z);

            // Order important, index to find child is calculated
            _nodes[index].children[0] = createNode(from, center);
            _nodes[index].children[1] = createNode(from + along_x, center + along_x);
            _nodes[index].children[2] = createNode(from + along_y, center + along_y);
            _nodes[index].children[3] = createNode(from + along_x + along_y, center + along_x + along_y);
            _nodes[index].children[4] = createNode(from + along_z, center + along_z);
            _nodes[index].children[5] = createNode(from + along_x + along_z, center + along_x + along_z);
            _nodes[index].children[6] = createNode(from + along_y + along_z, center + along_y + along_z);
            _nodes[index].children[7] = createNode(center, to);
        }

        constexpr size_t createNode(const Position& from, const Position& to) {
            const size_t childIndex = _nodes.size();
            _nodes.emplace_back(from, to);
            return childIndex;
        }
};