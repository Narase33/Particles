// Particle.cpp : Defines the entry point for the application.
//

#include "BlueWorld.h"
#include "Camera.h"
#include "Simulation.h"

#include <chrono>
#include <iostream>
#include <random>

void pixelTest() {
    constexpr uint16_t windowWidth = 1'000;
    const Color red(255.0, 0.0, 0.0);
    const Color green(0.0, 255.0, 0.0);
    const Color blue(0.0, 0.0, 255.0);

    BlueWorld world(Vector2u(windowWidth, windowWidth));
    Vector3d vec0(30.0, 40.0, 50.0);
    Vector3d vec1(50.0, 40.0, 30.0);
    world.addShort(vec0, red);
    world.addShort(vec1, green);
    world.addShort(math::normal(vec0, vec1), blue);

    while (true) {
        world.show();
    }
}

void special_test_collide() {
    constexpr uint16_t windowWidth = 1'000;

    Simulation sim(Vector2u(windowWidth, windowWidth));

    Particle a(Position(10.0, 1.0, 0.0), Vector3d(-0.5, 0.0, 0.0), Vector3d(0, 0, 0), 10);
    Particle b(Position(-10.0, -1.0, 0.0), Vector3d(0.5, 0.0, 0.0), Vector3d(), 10);

    sim.placeParticle(a);
    sim.placeParticle(b);

    std::string text;
    while (true) {
        const auto startTime = std::chrono::high_resolution_clock::now();
        sim.step_bruteForce();
        const auto endTime = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        sim.setText(std::format("{} FPS", 1000 / duration.count()));
        std::this_thread::sleep_for(std::chrono::milliseconds(200) - duration);
    }
}

void special_test_merge() {
    constexpr uint16_t windowWidth = 1'000;

    Simulation sim(Vector2u(windowWidth, windowWidth));

    Particle a(Position(20.0, 1.0, 0.0), Vector3d(0.1, 0.0, 0.0), Vector3d(), 5);
    Particle b(Position(-10.0, -1.0, 0.0), Vector3d(0.5, 0.0, 0.0), Vector3d(), 10);

    sim.placeParticle(a);
    sim.placeParticle(b);

    std::string text;
    while (true) {
        const auto startTime = std::chrono::high_resolution_clock::now();
        sim.step_bruteForce();
        const auto endTime = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        sim.setText(std::format("{} FPS", 1000 / duration.count()));
        std::this_thread::sleep_for(std::chrono::milliseconds(200) - duration);
    }
}

void special_test_spin() {
    constexpr uint16_t windowWidth = 1'000;

    Simulation sim(Vector2u(windowWidth, windowWidth));

    Particle a(Position(5.0, 0.0, 0.0), Vector3d(0.0, 0.0, 0.0), Vector3d(0, 0, 1), 10);
    Particle b(Position(-5.0, 0.0, 0.0), Vector3d(0.0, 0.0, 0.0), Vector3d(), 10);

    sim.placeParticle(a);
    sim.placeParticle(b);

    std::string text;
    while (true) {
        const auto startTime = std::chrono::high_resolution_clock::now();
        sim.step_bruteForce();
        const auto endTime = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        sim.setText(std::format("{} FPS", 1000 / duration.count()));
        std::this_thread::sleep_for(std::chrono::milliseconds(200) - duration);
    }
}

void run_showcase3() {
    constexpr size_t particleCount = 100;
    constexpr uint16_t windowWidth = 1'000;
    constexpr uint16_t spawnWidth = windowWidth / 1;

    Simulation sim(Vector2u(windowWidth, windowWidth));

    std::mt19937 mt;
    std::uniform_real_distribution<double> dice_window_x(-spawnWidth, spawnWidth);
    std::uniform_real_distribution<double> dice_window_y(-spawnWidth, spawnWidth);

    auto spawner = [&]() {
        Position pos(dice_window_x(mt), dice_window_y(mt), 0);
        while (std::hypot(pos.x, pos.y, pos.z) > (spawnWidth / 2.0)) {
            pos = Position(dice_window_x(mt), dice_window_y(mt), 0);
        }

        sim.placeParticle(pos, Vector3d(0.0, 0.0, 0.0));
    };

    std::string text;
    constexpr size_t particlesPerRound = 10;
    for (int i = 0; i < particleCount;) {
        const auto startTime = std::chrono::high_resolution_clock::now();
        for (int j = 0; j < particlesPerRound; j++, i++) {
            spawner();
        }
        sim.step_bruteForce();
        const auto endTime = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        sim.setText(std::format("{} particle, {}ms", i, duration.count()));
        std::this_thread::sleep_for(std::chrono::milliseconds(20) - duration);
    }

    while (true) {
        const auto startTime = std::chrono::high_resolution_clock::now();
        sim.step_bruteForce();
        const auto endTime = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        sim.setText(std::format("{} particle, {}ms", particleCount, duration.count()));
        std::this_thread::sleep_for(std::chrono::milliseconds(20) - duration);
    }
}

void run_showcase2() {
    constexpr size_t particleCount = 1'000;
    constexpr uint16_t windowWidth = 1'000;
    constexpr uint16_t spawnWidth = windowWidth / 1;

    Simulation sim(Vector2u(windowWidth, windowWidth));

    std::mt19937 mt;
    std::uniform_real_distribution<double> dice_window_x(-spawnWidth, spawnWidth);
    std::uniform_real_distribution<double> dice_window_y(-spawnWidth, spawnWidth);
    std::uniform_real_distribution<double> dice_window_z(-spawnWidth, spawnWidth);

    auto spawner = [&]() {
        Position pos(dice_window_x(mt), dice_window_y(mt), dice_window_z(mt));
        while (std::hypot(pos.x, pos.y, pos.z) > (spawnWidth / 2.0)) {
            pos = Position(dice_window_x(mt), dice_window_y(mt), dice_window_z(mt));
        }

        sim.placeParticle(pos, Vector3d(0.0, 0.0, 0.0));
    };

    std::string text;
    constexpr size_t particlesPerRound = 10;
    for (int i = 0; i < particleCount;) {
        const auto startTime = std::chrono::high_resolution_clock::now();
        for (int j = 0; j < particlesPerRound; j++, i++) {
            spawner();
        }
        sim.step_bruteForce();
        const auto endTime = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        sim.setText(std::format("{} particle, {}ms", i, duration.count()));
        std::this_thread::sleep_for(std::chrono::milliseconds(20) - duration);
    }

    while (true) {
        const auto startTime = std::chrono::high_resolution_clock::now();
        sim.step_bruteForce();
        const auto endTime = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        sim.setText(std::format("{} particle, {}ms", particleCount, duration.count()));
        std::this_thread::sleep_for(std::chrono::milliseconds(20) - duration);
    }
}

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
    std::string text;
    while (true) {
        const auto startTime = std::chrono::high_resolution_clock::now();
        sim.step_bruteForce();
        const auto endTime = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        sim.setText(std::format("{} FPS", 1000 / duration.count()));
        std::this_thread::sleep_for(std::chrono::milliseconds(20) - duration);
    }
}

void run_benchmark() {
    constexpr size_t particleCount = 100'000;
    constexpr uint16_t windowWidth = 1'000;
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

    std::chrono::milliseconds sum(0);
    std::chrono::milliseconds min(std::chrono::milliseconds::max());
    std::chrono::milliseconds max(std::chrono::milliseconds::min());
    for (int i = 0; i < benchmarkRounds; i++) {
        const auto startTime = std::chrono::high_resolution_clock::now();
        sim.step_bruteForce();
        const auto endTime = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        sum += duration;
        max = std::max(max, duration);
        min = std::min(min, duration);

        sim.setText(std::format("{}/{} {}\n", i, benchmarkRounds, duration));
    }

    std::cout << "Finished!\n";
    std::print(std::cout, "Total time: {}\n", sum);
    std::print(std::cout, "Average time: {}\n", sum / benchmarkRounds);
    std::print(std::cout, "Min time: {}\n", min);
    std::print(std::cout, "Max time: {}\n", max);
    std::print(std::cout, "{} | {} | {} | {}\n", sum, sum / benchmarkRounds, min, max);
}

int main() {
    // pixelTest();
    // special_test_merge();
    special_test_collide();
    // special_test_spin();
    // run_benchmark();
    // run_showcase2();
    // run_showcase3();
}

/*
 * Benchmarks:
 *                  | Average time | Min round | Max round
 * Single thread    |      30948ms |   30603ms |   31551ms
 * Full multithread |       4383ms |    4214ms |    4719ms
 * Barnes-Hut       |        574ms |     413ms |     728ms     First naive implementation
 *                  |        532ms |     369ms |     660ms     Calculated child indices
 *                  |        491ms |     346ms |     633ms
 *                  |        390ms |     256ms |     612ms     Cells put into global vector
 */