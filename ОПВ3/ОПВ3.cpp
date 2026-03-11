#include <iostream>
#include <vector>
#include <thread>
#include <future>
#include <atomic>
#include <chrono>
#include <cmath>
#include <random>

bool is_prime(int n) {
    if (n <= 1) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    for (int i = 3; i <= std::sqrt(n); i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

int main() {
    setlocale(LC_ALL, "ru");

    const size_t N = 10;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, 100);

    std::vector<int> numbers(N);
    for (size_t i = 0; i < N; ++i) {
        numbers[i] = dist(gen);
    }

    std::atomic<int> completed_tasks(0);

    std::promise<void> all_done_promise;
    std::future<void> all_done_future = all_done_promise.get_future();

    std::vector<std::future<bool>> futures;
    std::vector<std::thread> task_threads;

    for (int num : numbers) {
        std::packaged_task<bool()> task([num, &completed_tasks]() {
            bool result = is_prime(num);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            ++completed_tasks;
            return result;
            });

        futures.push_back(task.get_future());
        task_threads.emplace_back(std::move(task));
    }

    std::thread monitor([&completed_tasks, N, p = std::move(all_done_promise)]() mutable {
        while (completed_tasks.load() < N) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        p.set_value();
        });

    std::cout << "Основной поток: ожидание завершения всех задач...\n";
    all_done_future.wait();
    std::cout << "Основной поток: все задачи выполнены. Получение результатов...\n";

    for (size_t i = 0; i < N; ++i) {
        bool prime = futures[i].get();
        std::cout << "Число " << numbers[i] << (prime ? " простое" : " не простое") << std::endl;
    }

    for (auto& t : task_threads) {
        t.join();
    }
    monitor.join();

    return 0;
}