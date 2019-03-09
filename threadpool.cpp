#include <unistd.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

class thread_pool {
   private:
    class lock_mutex_thread {
       public:
        std::thread thread;
        std::mutex mutex;
        std::unique_lock<std::mutex> lock;
        int id;
        lock_mutex_thread() {}
        lock_mutex_thread(const lock_mutex_thread &l) { this->id = l.id; }
    };
    std::mutex queue_mutex;
    bool ready = false;
    int size;
    std::queue<std::function<void(void)> > jobs;
    std::condition_variable cv;
    thread_pool(const thread_pool &tp) = delete;
    std::vector<lock_mutex_thread> threads;

   public:
    thread_pool(int size) {
        this->size = size;
        threads = std::vector<lock_mutex_thread>(size);
        for (int i = 0; i < size; i++) {
            threads[i].id = i;
            threads[i].thread =
                std::thread([this, i] { this->WorkerFunction(threads[i]); });
            threads[i].thread.detach();
        }
        ready = true;
    }
    void WorkerFunction(lock_mutex_thread locker) {
        locker.lock = std::unique_lock<std::mutex>(locker.mutex);
        while (true) {
            std::cout << "Executing on Thread " << locker.id;
            cv.wait(locker.lock,
                    [this] { return ((ready) && (jobs.size() > 0)); });
            std::unique_lock<std::mutex> queueLock(queue_mutex);
            std::function<void(void)> job = jobs.front();
            jobs.pop();
            queueLock.unlock();
            job();
        }
    }
    void AddToQueue(std::function<void(void)> fun) {
        jobs.push(fun);
        cv.notify_one();
    }
};

int main() {
    thread_pool pool(4);
    pool.AddToQueue([]() { std::cout << "works1\n"; });
    pool.AddToQueue([]() { std::cout << "works2\n"; });
    pool.AddToQueue([]() { std::cout << "works3\n"; });
    pool.AddToQueue([]() { std::cout << "works4\n"; });
    pool.AddToQueue([]() { std::cout << "works5\n"; });
    pool.AddToQueue([]() { std::cout << "works6\n"; });
    pool.AddToQueue([]() { std::cout << "works7\n"; });
    pool.AddToQueue([]() { std::cout << "works8\n"; });
    pool.AddToQueue([]() { std::cout << std::endl; });
    while (true)
        ;
    return 0;
}