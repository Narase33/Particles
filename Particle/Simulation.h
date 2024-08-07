#pragma once

#include "BarnesHut.h"
#include "Camera.h"
#include "Picture.h"

#include <execution>

class Simulation {
    public:
        explicit Simulation(Vector2u windowSize) :
                _barnesHut(Position(-windowSize.x * 4, -windowSize.x * 4, -windowSize.x * 4), Position(windowSize.x * 4, windowSize.x * 4, windowSize.x * 4)),
                _window(sf::VideoMode(windowSize.x, windowSize.y), "Particle"),
                _pic(Vector2u(windowSize.x, windowSize.y), Camera(Position(windowSize.x, windowSize.y, -600), Vector2d(0.0, 0.0), 90)) {
            _window.setPosition({200, 5});
        }

        void step_bruteForce() {
            handleEvents();

            _pic.reset();

            if (!_simPaused) {
                std::ranges::for_each(_particles, [this](Particle& p) {
                    for (Particle& other : _particles) {
                        if (&p == &other) {
                            continue;
                        }

                        p.accelerate(other.position(), other.mass());

                        if (math::distance(p.position(), other.position()) < (p.radius() + other.radius())) {
                            p.collide(other);
                        }
                    }
                });

                std::ranges::for_each(_particles, [this](Particle& p) {
                    p.step();
                });
            }

            for (const Particle& p : _particles) {
                _pic.setParticle(p);
            }

            _window.clear();
            _pic.render(_window);
            _window.display();
        }

        void step_barnesHut() {
            handleEvents();

            _pic.reset();

            std::erase_if(_particles, [](const Particle& p) {
                return !p.isEnabled();
            });

            _barnesHut.resetCalculation();
            _barnesHut.insertParticles(_particles);

            if (!_simPaused) {
                std::for_each(std::execution::seq, _particles.begin(), _particles.end(), [this](Particle& p) {
                    if (p.isEnabled()) {
                        _barnesHut.calculateAcceleration(p);
                    }
                });

                std::for_each(std::execution::seq, _particles.begin(), _particles.end(), [](Particle& p) {
                    p.step();
                });
            }

            for (const Particle& p : _particles) {
                if (p.isEnabled()) {
                    _pic.setParticle(p);
                }
            }

            _window.clear();
            _pic.render(_window);
            _window.display();
        }

        void placeParticle(const Position& pos, const Vector3d& acceleration) {
            _particles.emplace_back(pos, acceleration);
        }

        void placeParticle(const Particle& p) {
            _particles.push_back(p);
        }

        void setText(const std::string& text) {
            _pic.setText(text);
        }

    private:
        std::vector<Particle> _particles;
        BarnesHut _barnesHut;

        sf::RenderWindow _window;
        Picture _pic;

        bool _inMouseMove = false;
        bool _inMouseRotation = false;
        bool _simPaused = false;

        void handleEvents() {
            static Vector2d oldMousePosition;

            sf::Event ev;
            while (_window.pollEvent(ev)) {
                switch (ev.type) {
                    case sf::Event::Closed:
                        _window.close();
                        std::exit(0);
                        return;
                    case sf::Event::KeyPressed:
                        if (ev.key.code == sf::Keyboard::Space) {
                            _simPaused = !_simPaused;
                        }
                        if (ev.key.code == sf::Keyboard::BackSpace) {
                            _pic.camera.reset();
                        }
                        break;
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
                            _pic.camera.move(deltaPos);
                            oldMousePosition = newMousePosition;
                        } else if (_inMouseRotation) {
                            const auto& mousePos = sf::Mouse::getPosition(_window);
                            const Vector2d newMousePosition(mousePos.x, mousePos.y);
                            const Vector2d deltaPos = newMousePosition - oldMousePosition;
                            _pic.camera.turn(Vector2d(deltaPos.y / -2, deltaPos.x / 2));
                            oldMousePosition = newMousePosition;
                        }
                        break;
                    case sf::Event::MouseWheelScrolled: {
                        const int factor = (ev.mouseWheel.x > 0) ? 1 : -1;
                        _pic.camera.zoom(100 * factor);
                        return;
                    }
                    default:
                        break;
                }
            }
        }
};