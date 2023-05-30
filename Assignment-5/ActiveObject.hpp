#ifndef ACTIVEOBJECT_HPP
#define ACTIVEOBJECT_HPP

#include <thread>
#include <functional>
#include "Queue.hpp"

class ActiveObject
{
public:
    using TaskFunction = std::function<void()>;

    ActiveObject(Queue<TaskFunction> *queue) : queue_(queue) {}

    void bindQueue(Queue<TaskFunction> *queue)
    {
        queue_ = queue;
    }

    void createActiveObject()
    {
        active_ = true;
        thread_ = std::thread([this]
                              {
            while (active_)
            {
                TaskFunction task = queue_->dequeue();
                task();
                if (stopRequested_)
                {
                    // Notify the main thread that this thread is exiting
                    std::lock_guard<std::mutex> lock(mutex_);
                    active_ = false;
                    condition_.notify_one();
                    break; // Stop requested, break out of the loop
                }
            } });
    }

    void stop()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopRequested_ = true;
        }
        condition_.notify_all();
        active_ = false;
    }

    void join()
    {
        if (thread_.joinable())
        {
            thread_.join();
        }
    }

    bool isActive() { return active_; }

    Queue<TaskFunction> *getQueue()
    {
        return queue_;
    }

private:
    Queue<TaskFunction> *queue_;
    std::thread thread_;
    bool active_ = false;
    bool stopRequested_ = false;
    std::mutex mutex_;
    std::condition_variable condition_;
};

#endif // ACTIVEOBJECT_HPP
