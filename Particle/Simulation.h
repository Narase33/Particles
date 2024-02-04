#pragma once

#include "Particle.h"

#include <execution>


class Simulation {
    public:
        Simulation(Vector2u windowSize) :
                _camera(Position(windowSize.x, windowSize.y, 600), Vector2d(0.0, 0.0), 90),
                _window(sf::VideoMode(windowSize.x, windowSize.y), "Particle"),
                _pic(Vector2u(windowSize.x, windowSize.y)),
                _octTree(Position(-windowSize.x * 2, -windowSize.x * 2, -windowSize.x * 2), Position(windowSize.x * 2, windowSize.x * 2, windowSize.x * 2), 8) {
            _window.setPosition({200, 5});
        }

        void step_bruteForce() {
            handleEvents();

            _pic.fill(Color(0, 0, 0));

            std::for_each(std::execution::par_unseq, _particles.begin(), _particles.end(), [this](Particle& p) {
                for (const Particle& other : _particles) {
                    calculateAcceleration(p, other);
                }
                p.step();
            });

            for (const Particle& p : _particles) {
                _pic.set(_camera.project(p.position));
            }

            _window.clear();
            _pic.renderTo(_window);
            _window.display();
        }

        void step_barnesHut() {
            handleEvents();

            _pic.fill(Color(0, 0, 0));

            _octTree.resetCalculation();
            _octTree.insertParticles(_particles);

            std::for_each(std::execution::par_unseq, _particles.begin(), _particles.end(), [this](Particle& p) {
                _octTree.calculateAcceleration(p);
                p.step();
            });

            for (const Particle& p : _particles) {
                _pic.set(_camera.project(p.position));
            }

            _window.clear();
            _pic.renderTo(_window);
            _window.display();
        }

        void placeParticle(const Position& pos, const Vector3d& acceleration) {
            _particles.emplace_back(pos, acceleration);
        }

    private:
        std::vector<Particle> _particles;
        OctTree _octTree;

        Camera _camera;
        sf::RenderWindow _window;
        Picture _pic;

        bool _inMouseMove = false;
        bool _inMouseRotation = false;

        static void calculateAcceleration(Particle& p, const Particle& other) {
            constexpr double G = 0.001;

            const Vector3d delta = other.position - p.position;
            const double force = (G * p.mass * other.mass) / (delta.x * delta.x + delta.y * delta.y + delta.z * delta.z + 1);
            const Vector3d acceleration = delta * force;

            p.acceleration += acceleration;

#if defined(_DEBUG)
            if (std::isnan(p.acceleration.x) or std::isnan(p.acceleration.y) or std::isnan(p.acceleration.z)) {
                throw std::runtime_error("Is NAN");
            }
#endif
        }

        void handleEvents() {
            static Vector2d oldMousePosition;

            sf::Event ev;
            while (_window.pollEvent(ev)) {
                switch (ev.type) {
                    case sf::Event::Closed:
                        _window.close();
                        std::exit(0);
                        return;
                    case sf::Event::MouseButtonPressed:
                        if (ev.mouseButton.button == sf::Mouse::Right) {
                            const auto& mousePos = sf::Mouse::getPosition(_window);
                            oldMousePosition = Vector2d(mousePos.x, mousePos.y);
                            _inMouseMove = true;
                        } else if (ev.mouseButton.button == sf::Mouse::Left) {
                            const auto& mousePos = sf::Mouse::getPosition(_window);
                            oldMousePosition = Vector2d(mousePos.x, mousePos.y);
                            _inMouseRotation = true;
                        }
                        break;
                    case sf::Event::MouseButtonReleased:
                        if (ev.mouseButton.button == sf::Mouse::Right) {
                            _inMouseMove = false;
                        } else if (ev.mouseButton.button == sf::Mouse::Left) {
                            _inMouseRotation = false;
                        }
                        break;
                    case sf::Event::MouseMoved:
                        if (_inMouseMove) {
                            const auto& mousePos = sf::Mouse::getPosition(_window);
                            const Vector2d newMousePosition(mousePos.x, mousePos.y);
                            const Vector2d deltaPos = newMousePosition - oldMousePosition;
                            _camera.move(deltaPos);
                            oldMousePosition = newMousePosition;
                        } else if (_inMouseRotation) {
                            const auto& mousePos = sf::Mouse::getPosition(_window);
                            const Vector2d newMousePosition(mousePos.x, mousePos.y);
                            const Vector2d deltaPos = newMousePosition - oldMousePosition;
                            _camera.turn(Vector2d(deltaPos.y / -2, deltaPos.x / 2));
                            oldMousePosition = newMousePosition;
                        }
                        break;
                    case sf::Event::MouseWheelScrolled:
                        const int factor = (ev.mouseWheel.x > 0) ? -1 : 1;
                        _camera.zoom(100 * factor);
                        return;
                }
            }
        }
};