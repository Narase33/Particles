// Particle.cpp : Defines the entry point for the application.
//

#include "Camera.h"
#include "Grid.h"
#include "OctTree.h"
#include "Particle.h"
#include "Picture.h"
#include "Simulation.h"

#include <chrono>
#include <iostream>
#include <random>

void run_showcase() {
    constexpr size_t particleCount = 10'000;
    constexpr uint16_t windowWidth = 1'000;
    constexpr uint16_t spawnWidth = windowWidth / 1;

    Simulation sim(Vector2u(windowWidth, windowWidth));

    std::mt19937 mt;
    std::uniform_real_distribution<double> dice_window_x(-spawnWidth, spawnWidth);
    std::uniform_real_distribution<double> dice_window_y(-spawnWidth, spawnWidth);
    std::uniform_real_distribution<double> dice_window_z(-spawnWidth, spawnWidth);

    for (int i = 0; i < particleCount; i++) {
        Position pos(dice_window_x(mt), dice_window_y(mt), dice_window_z(mt));
        while (std::hypot(pos.x, pos.y, pos.z) > (spawnWidth / 2.0)) {
            pos = Position(dice_window_x(mt), dice_window_y(mt), dice_window_z(mt));
        }

        sim.placeParticle(pos, Vector3d(0.0, 0.0, 0.0));
    }
    std::print(std::cout, "Placed {} particles\n", particleCount);

    std::print(std::cout, "Start sinulation [press enter]");
    std::cin.get();
    while (true) {
        const auto startTime = std::chrono::high_resolution_clock::now();
        // sim.step_bruteForce();
        sim.step_barnesHut();
        const auto endTime = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        std::cout << duration.count() << "ms\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(20) - duration);
    }
}

void run_benchmark() {
    constexpr size_t particleCount = 100'000;
    constexpr uint16_t windowWidth = 1000;
    constexpr uint16_t spawnWidth = windowWidth / 2;
    constexpr int benchmarkRounds = 1'000;

    Simulation sim(Vector2u(windowWidth, windowWidth));

    std::mt19937 mt;
    std::uniform_real_distribution<double> dice_window_x(-spawnWidth, spawnWidth);
    std::uniform_real_distribution<double> dice_window_y(-spawnWidth, spawnWidth);
    std::uniform_real_distribution<double> dice_window_z(-spawnWidth / 10, spawnWidth / 10);

    for (int i = 0; i < particleCount; i++) {
        Position pos(dice_window_x(mt), dice_window_y(mt), dice_window_z(mt));
        while (std::hypot(pos.x, pos.y) > (spawnWidth / 2.0)) {
            pos = Position(dice_window_x(mt), dice_window_y(mt), dice_window_z(mt));
        }

        sim.placeParticle(pos, Vector3d(0.0, 0.0, 0.0));
    }

    std::chrono::milliseconds sum(0), min(std::chrono::milliseconds::max()), max(std::chrono::milliseconds::min());
    for (int i = 0; i < benchmarkRounds; i++) {
        const auto startTime = std::chrono::high_resolution_clock::now();
        // sim.step_bruteForce();
        sim.step_barnesHut();
        const auto endTime = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        sum += duration;
        max = std::max(max, duration);
        min = std::min(min, duration);

        std::print(std::cout, "{}/{} {}\n", i, benchmarkRounds, duration);
    }

    std::cout << "Finished!\n";
    std::print(std::cout, "Total time: {}\n", sum);
    std::print(std::cout, "Average time: {}\n", sum / benchmarkRounds);
    std::print(std::cout, "Min time: {}\n", min);
    std::print(std::cout, "Max time: {}\n", max);
    std::print(std::cout, "{}, {}, {}, {}\n", sum, sum / benchmarkRounds, min, max);
}

int main() {
    run_benchmark();
    // run_showcase();
}

/*
 * Benchmarks:
 *                  | Average time | Min round | Max round
 * Single thread    |      30948ms |   30603ms |   31551ms
 * Full multithread |       4383ms |    4214ms |    4719ms
 * Barnes-Hut       |        574ms |     413ms |     728ms    // First naive implementation
 *                  |        532ms |     369ms |     660ms    // Calculated child indices
 */