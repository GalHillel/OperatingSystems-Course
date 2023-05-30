#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class Queue
{
public:
    void enqueue(const T &item)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(item);
        lock.unlock();
        condition_.notify_one();
    }

    T dequeue()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this]
                        { return !queue_.empty(); });
        T item = queue_.front();
        queue_.pop();
        return item;
    }

    bool isEmpty() const
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.empty();
    }

private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
};

#endif // QUEUE_HPP
