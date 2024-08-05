#pragma once

#include "BarnesHut.h"
#include "Camera.h"
#include "Picture.h"

#include <execution>

struct Pixel {
        Position pos;
        Color color;
};

struct PixelLine {
        Position from;
        Position to;
        Color color;
};

class sfLine : public sf::Drawable {
    public:
        sfLine(const Vector2d& from, const Vector2d& to, const Color& color) :
                color(color) {
            Vector2d direction = to - from;
            Vector2d unitDirection = direction / std::sqrt(direction.x * direction.x + direction.y * direction.y);
            Vector2d unitPerpendicular(-unitDirection.y, unitDirection.x);

            Vector2d offset = (thickness / 2.0) * unitPerpendicular;

            vertices[0].position = toSF(from + offset);
            vertices[1].position = toSF(to + offset);
            vertices[2].position = toSF(to - offset);
            vertices[3].position = toSF(from - offset);

            for (int i = 0; i < 4; ++i) {
                vertices[i].color = toSF(color);
            }
        }

        void draw(sf::RenderTarget& target, sf::RenderStates) const override {
            target.draw(vertices, 4, sf::Quads);
        }

        sf::Vector2f toSF(const Vector2d& v) const {
            return sf::Vector2f(v.x, v.y);
        }

        sf::Color toSF(const Color& v) const {
            return sf::Color(v.x, v.y, v.z);
        }

    private:
        sf::Vertex vertices[4];
        double thickness = 0.5f;
        Color color;
};

class BlueWorld {
    public:
        explicit BlueWorld(Vector2u windowSize) :
                _window(sf::VideoMode(windowSize.x, windowSize.y), "Particle - BlueWorld"),
                _pic(Vector2u(windowSize.x, windowSize.y), Camera(Position(windowSize.x, windowSize.y, 600), Vector2d(0.0, 0.0), 90)) {
            _window.setPosition({200, 5});
        }

        void show() {
            handleEvents();

            _window.clear();

            for (const Pixel& p : _pixels) {
                _pic.setPixel(_pic.camera.project(p.pos), p.color);
            }

            for (const PixelLine& p : _pixelLines) {
                sfLine line(_pic.camera.project(p.from * Vector3d{1.0, -1.0, -1.0}), _pic.camera.project(p.to * Vector3d{1.0, -1.0, -1.0}), p.color);

                _window.draw(line);
            }

            _window.display();
        }

        void add(const Pixel& p) {
            _pixels.push_back(p);
        }

        void add(const PixelLine& p) {
            _pixelLines.push_back(p);
        }

        void addShort(const Vector3d& vec, const Color& color) {
            add(PixelLine({0.0, 0.0, 0.0}, vec, color));
        }

        void add(const Vector3d& vec) {
            const Position zero{};

            add(PixelLine(zero, Position(vec.x, 0.0, 0.0), Color(255, 0, 0)));
            add(PixelLine(Position(vec.x, 0.0, 0.0), vec, Color(0, 255, 255)));

            add(PixelLine(zero, Position(0.0, vec.y, 0.0), Color(0, 255, 0)));
            add(PixelLine(Position(0.0, vec.y, 0.0), vec, Color(255, 0, 255)));

            add(PixelLine(zero, Position(0.0, 0.0, vec.z), Color(0, 0, 255)));
            add(PixelLine(Position(0.0, 0.0, vec.z), vec, Color(255, 255, 0)));

            add(PixelLine(zero, vec, Color(255, 255, 255)));
        }

        void setText(const std::string& text) {
            _pic.setText(text);
        }

    private:
        std::vector<Pixel> _pixels;
        std::vector<PixelLine> _pixelLines;

        sf::RenderWindow _window;
        Picture _pic;

        bool _inMouseMove = false;
        bool _inMouseRotation = false;

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
                    case sf::Event::MouseWheelScrolled:
                        const int factor = (ev.mouseWheel.x > 0) ? 1 : -1;
                        _pic.camera.zoom(100 * factor);
                        return;
                }
            }
        }
};