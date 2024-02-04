#pragma once

#include "Vector.h"
#include <iostream>
#include <iomanip>

struct GridCell {
        float value{};
        Position position{};
};

template <size_t width>
class Grid {
    public:
        static constexpr size_t size = width * width;
        using ArrType = std::array<GridCell, size>;

        const GridCell& get(const Position& pos) const {
            return _grid(to1Dim(pos));
        }

        GridCell& get(const Position& pos) {
            return _grid[to1Dim(pos)];
        }

        const ArrType& getGrid() const {
            return _grid;
        }

        ArrType& getGrid() {
            return _grid;
        }

        void resetValues() {
            for (GridCell& cell : _grid) {
                cell.value = 0.0f;
            }
        }

        void print() const {
            size_t cellNumber = 0;
            for (size_t i = 0; i < width; i++) {
                for (size_t j = 0; j < width; j++) {
                    std::cout << "[" << std::setfill(' ') << std::setw(3) << _grid[cellNumber++].value << "]";
                }
                std::cout << "\n";
            }
        }

    private:
        ArrType _grid;

        size_t to1Dim(const Position& pos) const {
            return pos.x + pos.y * width;
        }

        Position to2Dim(size_t pos) const {
            const size_t x = pos & width;
            const size_t y = pos / width;
            return {x, y};
        }
};