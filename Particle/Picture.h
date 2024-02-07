#pragma once

#include "SFML/Graphics.hpp"

#include "Vector.h"

class Picture {
    public:
        Picture(Vector2u size) :
                _size(size),
                _pixelsSize(_size.x * _size.y * 4),
                _pixelBuffer(new sf::Uint8[_pixelsSize]),
                _particleBuffer(new uint16_t[_size.x * _size.y]) {
            _tex.create(_size.x, _size.y);
        }

        void fill(Color color) {
            for (size_t i = 0; i < _pixelsSize; i += 4) {
                _pixelBuffer[i + 0] = color.x;
                _pixelBuffer[i + 1] = color.y;
                _pixelBuffer[i + 2] = color.z;
                _pixelBuffer[i + 3] = 255;
            }
        }

        void reset() {
            for (size_t i = 0; i < _pixelsSize; i++) {
                _pixelBuffer[i] = 0;
            }

            for (size_t i = 0; i < _size.x * _size.y; i++) {
                _particleBuffer[i] = 0;
            }
        }

        void setParticle(const Vector2d& coord) {
            if (isOutOfBounds(coord)) {
                return;
            }

            const size_t index = to1dim(coord);
            if (_particleBuffer[index] < std::numeric_limits<uint8_t>::max()) {
                _particleBuffer[index]++;
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

        sf::Texture _tex;
        sf::Sprite _sprite;
        std::unique_ptr<sf::Uint8[]> _pixelBuffer;
        std::unique_ptr<uint16_t[]> _particleBuffer;
};