#pragma once

#include "SFML/Graphics.hpp"

#include "Vector.h"

class Picture {
    public:
        Camera camera;

        Picture(Vector2u size, const Camera& camera) :
                _size(size),
                _pixelsSize(_size.x * _size.y * 4),
                _pixelBuffer(std::make_unique<sf::Uint8[]>(_pixelsSize)),
                _particleBuffer(std::make_unique<uint16_t[]>(_size.x * _size.y)),
                camera(camera) {
            _tex.create(_size.x, _size.y);
            _font.loadFromFile("ariblk.ttf");
            _textField.setFont(_font);
            _textField.setCharacterSize(20);
            _textField.setFillColor(sf::Color::White);
        }

        void fill(Color color) {
            for (size_t i = 0; i < _pixelsSize; i += 4) {
                _pixelBuffer[i + 0] = color.x;
                _pixelBuffer[i + 1] = color.y;
                _pixelBuffer[i + 2] = color.z;
                _pixelBuffer[i + 3] = 255;
            }
        }

        void reset() const {
            for (size_t i = 0; i < _pixelsSize; i++) {
                _pixelBuffer[i] = 0;
            }

            for (size_t i = 0; i < _size.x * _size.y; i++) {
                _particleBuffer[i] = 0;
            }
        }

        double calculateRadius(const Position& turnedPosition, double radius) const {
            const double screenParticleDelta = turnedPosition.z - camera.getDisplaySurface().z;
            double out = radius * (500 / screenParticleDelta) * std::tan(camera.getFov() / 2);
            return out;
        }

        void setParticle(const Particle& particle) const {
            const Position turnedPosition = camera.turn(particle.position());
            if (turnedPosition.z < camera.getDisplaySurface().z) {
                return;
            }

            const Vector2d particleProjectedPosition = camera.project(turnedPosition);

            double projectedRadius = calculateRadius(turnedPosition, particle.radius());

            if (projectedRadius <= 1) {
                if (isOutOfBounds(particleProjectedPosition)) {
                    return;
                }

                const size_t index = to1dim(particleProjectedPosition);
                if (_particleBuffer[index] < std::numeric_limits<uint8_t>::max()) {
                    _particleBuffer[index]++;
                }
            } else {
                for (int y = -projectedRadius; y < projectedRadius; y++) {
                    const int to_x = (int)std::sqrt(projectedRadius * projectedRadius - y * y);
                    for (int x = -to_x; x < to_x; x++) {
                        const Vector2d coord = Vector2d(x, y) + particleProjectedPosition;
                        if (isOutOfBounds(coord)) {
                            continue;
                        }

                        const size_t index = to1dim(coord);
                        if (_particleBuffer[index] < std::numeric_limits<uint8_t>::max()) {
                            _particleBuffer[index]++;
                        }
                    }
                }
            }
        }

        void setPixel(const Vector2d& coord, const Color& color) const {
            if (isOutOfBounds(coord)) {
                return;
            }

            _pixelBuffer[to1dim(coord) * 4 + 0] = color.x;
            _pixelBuffer[to1dim(coord) * 4 + 1] = color.y;
            _pixelBuffer[to1dim(coord) * 4 + 2] = color.z;
            _pixelBuffer[to1dim(coord) * 4 + 3] = 255;
        }

        void setPixelLine(const Vector2d& from, const Vector2d& to, const Color& color) const {
            for (int x = from.x; x < to.x; x++) {
                for (int y = from.y; y < to.y; y++) {
                    setPixel(Vector2d(x, y), color);
                }
            }
        }

        void render(sf::RenderTarget& target) {
            for (int y = 0; y < _size.y; y++) {
                for (int x = 0; x < _size.x; x++) {
                    uint16_t value = _particleBuffer[to1dim(Vector2d(x, y))];
                    if (value == 0) {
                        continue;
                    }

                    const uint8_t c = value * (105.0 / 255.0) + 150;

                    uint8_t rgb[4] = {c, c, c, 255};

                    const Vector2d coords(x, y);
                    setColor(coords, rgb);
                    // addBlurr(coords, rgb);
                }
            }

            _tex.update(_pixelBuffer.get());
            _sprite.setTexture(_tex);
            target.draw(_sprite);
            target.draw(_textField);
        }

        void addBlurr(const Vector2d& coord, uint8_t color[4]) {
            for (int i = 0; i < 3; i++) {
                color[i] *= 0.1;
            }

            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    if ((i == 0) and (j == 0)) {
                        continue;
                    }

                    setColor(coord + Vector2d(i, j), color);
                }
            }
        }

        void setColor(const Vector2d& coord, uint8_t color[4]) {
            if (isOutOfBounds(coord)) {
                return;
            }

            const size_t wideCoord = to1dim(coord) * 4;
            for (int i = 0; i < 4; i++) {
                const uint16_t c = _pixelBuffer[wideCoord + i] + color[i];
                if (c <= 255) {
                    _pixelBuffer[wideCoord + i] = c;
                }
            }
        }

        void setText(const std::string& text) {
            _textField.setString(text);
        }

        bool isOutOfBounds(const Vector2d& pixel) const {
            return (pixel.x < 0) or (pixel.x >= _size.x) or (pixel.y < 0) or (pixel.y >= _size.y);
        }

        size_t to1dim(const Vector2d& coord) const {
            return static_cast<size_t>(coord.x) + static_cast<size_t>(coord.y) * _size.x;
        }

    private:
        static constexpr uint8_t _minParticleBrightness = 150;

        Vector2u _size;
        size_t _pixelsSize;

        sf::Font _font;
        sf::Text _textField;
        sf::Texture _tex;
        sf::Sprite _sprite;
        std::unique_ptr<sf::Uint8[]> _pixelBuffer;
        std::unique_ptr<uint16_t[]> _particleBuffer;
};