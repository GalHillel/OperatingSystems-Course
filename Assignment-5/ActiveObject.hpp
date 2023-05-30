#ifndef ACTIVEOBJECT_HPP
#define ACTIVEOBJECT_HPP

#include <thread>
#include <functional>
#include <atomic>
#include "Queue.hpp"

class ActiveObject
{
public:
    using TaskFunction = std::function<void()>;

    ActiveObject(Queue<TaskFunction> *queue)
        : queue_(queue), active_(false), stopRequested_(false)
    {
    }

    void bindQueue(Queue<TaskFunction> *queue)
    {
        queue_ = queue;
    }

    void createActiveObject()
    {
        active_ = true;
        thread_ = std::thread([this]
                              {
            while (true) {
                TaskFunction task = queue_->dequeue();
                task();

                if (stopRequested_.load()) {
                    break;
                }
            } });
    }

    void stop()
    {
        stopRequested_.store(true);
        queue_->enqueue([] {}); // Enqueue a dummy task to wake up the thread
    }

    void join()
    {
        if (thread_.joinable())
        {
            thread_.join();
        }
    }

    Queue<TaskFunction> *getQueue()
    {
        return queue_;
    }

private:
    Queue<TaskFunction> *queue_;
    std::thread thread_;
    std::atomic<bool> active_;
    std::atomic<bool> stopRequested_;
};

#endif // ACTIVEOBJECT_HPP
