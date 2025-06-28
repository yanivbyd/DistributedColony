#ifndef UTILS_H
#define UTILS_H

#include <random>
#include <cstdint>

class Random {
private:
    static std::mt19937 gen;
    static bool initialized;

public:
    static void init() {
        if (!initialized) {
            std::random_device rd;
            gen.seed(rd());
            initialized = true;
        }
    }

    // Random integer between min and max (inclusive)
    static int range(int min, int max) {
        init();
        std::uniform_int_distribution<> dist(min, max);
        return dist(gen);
    }

    // Random float between min and max
    static float range(float min, float max) {
        init();
        std::uniform_real_distribution<float> dist(min, max);
        return dist(gen);
    }

    // Random byte (0-255)
    static uint8_t byte() {
        return static_cast<uint8_t>(range(0, 255));
    }

    // Random boolean
    static bool coin() {
        return range(0, 1) == 1;
    }

    // Random choice from array
    template<typename T>
    static T choice(const T* array, int size) {
        return array[range(0, size - 1)];
    }
};

#endif // UTILS_H 