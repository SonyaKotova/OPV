#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <atomic>

class ThreadPool {

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;

public:
    explicit ThreadPool(size_t num_threads) : stop(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [this] {
                            return stop || !tasks.empty();
                            });
                        if (stop && tasks.empty())
                            return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
                });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers) {
            if (worker.joinable())
                worker.join();
        }
    }

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> result = task->get_future();

        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace([task]() { (*task)(); });
        }
        condition.notify_one();
        return result;
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
};

long long factorial(int n) {
    long long result = 1;
    for (int i = 2; i <= n; ++i)
        result *= i;
    return result;
}

int main() {
    setlocale(LC_ALL, "rus");

    ThreadPool pool(4);

    std::vector<std::future<long long>> results;

    std::cout << "Вычисление факториалов чисел от 1 до 15:\n";
    for (int n = 1; n <= 15; ++n) {
        results.emplace_back(pool.enqueue(factorial, n));
    }

    for (size_t i = 0; i < results.size(); ++i) {
        int number = 1 + i;
        long long fact = results[i].get();
        std::cout << number << "! = " << fact << std::endl;
    }

    std::cout << "Все задачи выполнены. Пул завершает работу.\n";
    return 0;
}