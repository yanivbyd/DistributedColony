#ifndef BE_BACKGROUND_THREAD_H
#define BE_BACKGROUND_THREAD_H

#include <thread>
#include <atomic>

class BackgroundThread {
public:
    BackgroundThread();
    ~BackgroundThread();
    
    void start();
    
private:
    std::thread thread;
    bool is_running;
    
    void thread_loop();
};

#endif // BE_BACKGROUND_THREAD_H 