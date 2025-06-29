#include "be_background_thread.h"
#include "../shared/utils.h"
#include "be_colony.h"
#include <chrono>
#include <iostream>
using std::this_thread::sleep_for;

BackgroundThread::BackgroundThread() {
    is_running = false;
}

BackgroundThread::~BackgroundThread() {
    is_running = false;
    if (thread.joinable()) {
        thread.join();
    }
}

void BackgroundThread::start() {
    is_running = true;
    thread = std::thread(&BackgroundThread::thread_loop, this);
}

void BackgroundThread::thread_loop() {
    while (is_running) {
        // Sleep for 200 milliseconds
        sleep_for(std::chrono::milliseconds(200));
        
        // Get actual grid dimensions from ColonyBackend
        ColonyBackend& colony = ColonyBackend::instance();
        
        // Generate random coordinates and radius
        const int x = Random::range(0, colony.get_width() - 1);
        const int y = Random::range(0, colony.get_height() - 1);
        const int radius = Random::range(3, 30);
        
        colony.blast(x, y, radius);
    }
} 