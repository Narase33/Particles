#pragma once

#include "SFML/Graphics.hpp"

#include "Vector.h"

class Picture {
    public:
        Picture(Vector2u size) :
                _size(size),
                _pixelsSize(_size.x * _size.y * 4),
                _pixelBuffer(new sf::Uint8[_pixelsSize]) {
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

        void set(Vector2d pixel) {
            constexpr uint8_t particleBrightness = 10;

            if ((pixel.x < 0) or (pixel.x > _size.x) or (pixel.y < 0) or (pixel.y > _size.y)) {
                return;
            }

            const size_t pos = (static_cast<size_t>(pixel.x) + (static_cast<size_t>(pixel.y) * _size.x)) * 4;
            if (_pixelBuffer[pos] < (255 - particleBrightness)) {
                if (_pixelBuffer[pos] == 0) {
                    _pixelBuffer[pos + 0] = 150; // r
                    _pixelBuffer[pos + 1] = 150; // g
                    _pixelBuffer[pos + 2] = 150; // b
                    _pixelBuffer[pos + 3] = 255;
                } else {
                    _pixelBuffer[pos + 0] += particleBrightness; // r
                    _pixelBuffer[pos + 1] += particleBrightness; // g
                    _pixelBuffer[pos + 2] += particleBrightness; // b
                }
            }

            constexpr int blurrWidth = 1;
            for (int i = -blurrWidth; i <= blurrWidth; i++) {
                for (int j = -blurrWidth; j <= blurrWidth; j++) {
                    addBlurr(pixel + Vector2d(i, j), 5);
                }
            }
        }

        void addBlurr(Vector2d pixel, uint8_t particleBrightness) {
            if ((pixel.x < 0) or (pixel.x > _size.x) or (pixel.y < 0) or (pixel.y > _size.y)) {
                return;
            }

            const size_t pos = (static_cast<size_t>(pixel.x) + (static_cast<size_t>(pixel.y) * _size.x)) * 4;
            if (_pixelBuffer[pos] < (255 - particleBrightness)) {
                _pixelBuffer[pos + 0] += particleBrightness; // r
                _pixelBuffer[pos + 1] += particleBrightness; // g
                _pixelBuffer[pos + 2] += particleBrightness; // b
            }
        }

        void set(Position pixel, Color color) {
            if ((pixel.x < 0) or (pixel.x > _size.x) or (pixel.y < 0) or (pixel.y > _size.y)) {
                return;
            }

            const size_t pos = (static_cast<size_t>(pixel.x) + (static_cast<size_t>(pixel.y) * _size.x)) * 4;
            _pixelBuffer[pos + 0] = color.x; // r
            _pixelBuffer[pos + 1] = color.y; // g
            _pixelBuffer[pos + 2] = color.z; // b
            _pixelBuffer[pos + 3] = 255;
        }

        void renderTo(sf::RenderTarget& target) {
            _tex.update(_pixelBuffer.get());
            _sprite.setTexture(_tex);
            target.draw(_sprite);
        }

    private:
        Vector2u _size;
        size_t _pixelsSize;

        sf::Texture _tex;
        sf::Sprite _sprite;
        std::unique_ptr<sf::Uint8[]> _pixelBuffer;
};