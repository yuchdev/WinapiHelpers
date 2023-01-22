#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <algorithm>


namespace helpers {

/// @brief Thread-pool class. Uses std::hardware_concurrency() to define a number of pools
/// Tasks are stored in a lock-based std::queue
class thread_pool {
public:

    /// @brief Create thread pool with passed size
    /// @param threads: number of threads that could be executed concurrently
    /// the constructor just launches some amount of workers waiting for tasks
    explicit thread_pool(size_t threads = 0)
    {
        cores_number_ = std::thread::hardware_concurrency();
        if (0 == threads) {
            if (0 == cores_number_) {
                threads_number_ = 2;
            }
            else {
                threads_number_ = cores_number_;
            }
        }
        else {
            threads_number_ = threads;
        }

        workers_.clear();
        for (size_t i = 0; i < threads_number_; ++i) {
            workers_.emplace_back(
                [this] {

                while (true) {
                    std::unique_lock<std::mutex> lock(this->queue_mutex_);
                    while (!this->stop_work_ && this->tasks_.empty())
                        this->queue_condition_.wait(lock);
                    if (this->stop_work_)
                        return;
                    std::function<void()> task(this->tasks_.front());
                    this->tasks_.pop();
                    lock.unlock();
                    task();
                }
            });
        }
        
        stop_work_.store(false);
    }

    thread_pool(const thread_pool&) = delete;
    thread_pool& operator=(const thread_pool&) = delete;
    
    /// @brief: Place task to the queue
    /// @param f: function or method to execute in the thread
    /// @param args: function arguments
    /// @throw: std::runtime_error exception if try to add new task to stopped pool
    /// @return: std::future<result_type> NOTE! Future should not overlive the pool!
    /// Use enque_promise(std::promise<result_type>) is you need results to be overlived
    /// Always catch exceptions under enqueue() call! and accessing the returned future!
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)->std::future<typename std::result_of<F(Args...)>::type>;
    
    /// @brief destroy pool with joining all executed threads

    ~thread_pool()
    {
        stop();
    }

    /// @brief Cleanup tasks queue
    void clear()
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        std::queue<std::function<void()>> zero;
        std::swap(tasks_, zero);
    }

    /// @brief Start the queue (takes effect if was stopped before)
    inline void start()
    {
        work_stopped_.store(false);
        stop_work_.store(false);
    }

    /// @brief Stop the queue, unable to accept new tasks
    inline void stop()
    {
        if (stop_work_)
            return;

        stop_work_.store(true);
        queue_condition_.notify_all();

        // gently wait for all workers to finish
        std::for_each(workers_.begin(), workers_.end(), [](std::thread& w) {w.join(); });
        work_stopped_.store(true);
    }

    /// @brief Check whether task queue is empty
    bool empty() const
    {
        return tasks_.empty();
    }

    /// @brief Check whether all tasks are finished after stop
    bool stopped() const
    {
        return work_stopped_;
    }

    /// @brief CPU cores as reported by the system
    size_t cores_number() const
    {
        return cores_number_;
    }

    /// @brief Thread workers in the pool
    size_t threads_number() const
    {
        return threads_number_;
    }

private:

    // CPU cores as reported by the system
    size_t cores_number_{};

    // Thread workers in the pool
    size_t threads_number_{};

    // need to keep track of threads so we can join them
    std::vector<std::thread> workers_;

    // the task queue
    std::queue<std::function<void()>> tasks_;

    // synchronization
    std::mutex queue_mutex_;
    std::condition_variable queue_condition_;

    // flag to stop
#if defined(_MSC_VER) && (_MSC_VER <= 1900)
    std::atomic<bool> stop_work_ = false;
    std::atomic<bool> work_stopped_ = false;
#else
    std::atomic<bool> stop_work_{false};
    std::atomic<bool> work_stopped_{false};
#endif
};

template<class F, class... Args>
auto thread_pool::enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> 
{
    typedef typename std::result_of<F(Args...)>::type return_type;

    // don't allow enqueue after stopping the pool
    if (stop_work_){
        // just return empty future, we already stopped
        return std::future<return_type>{};
    }

    auto task = std::make_shared< std::packaged_task<return_type()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();

    /* wrap queue lock */{
        std::unique_lock<std::mutex> lock(queue_mutex_);
        tasks_.push([task](){ (*task)(); });
    }
    queue_condition_.notify_one();
    return std::move(res);
}

} // namespace helpers
