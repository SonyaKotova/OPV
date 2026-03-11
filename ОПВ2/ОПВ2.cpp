#include <iostream>
#include <thread>
#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>

std::queue<int> tasks;
std::mutex mtx;
std::condition_variable cv;
bool done = false;

// std::recursive_mutex log_mtx;
std::mutex log_mtx;

//void log_recursive(const std::string& msg, int depth = 0) {
  //  std::lock_guard<std::recursive_mutex> lock(log_mtx);
    //if (depth < 2) {
      //  log_recursive("вложенный вызов", depth + 1);
    //}
    //std::cout << "[LOG] " << msg << " (глубина " << depth << ")\n";
//}

void log_recursive(const std::string& msg, int depth = 0) {
    std::lock_guard<std::mutex> lock(log_mtx);
    if (depth < 2) {
        log_recursive("вложенный вызов", depth + 1);
    }
    std::cout << "[LOG] " << msg << " (глубина " << depth << ")\n";
}

void producer() {
    for (int i = 1; i <= 20; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        {
            std::lock_guard<std::mutex> lock(mtx);
            tasks.push(i);
            std::cout << "Производитель: добавил задачу " << i << '\n';
        }
        cv.notify_one();
    }
    {
        std::lock_guard<std::mutex> lock(mtx);
        done = true;
        std::cout << "Производитель: завершил генерацию задач.\n";
    }
    cv.notify_all();
}

void consumer(int id) {
    thread_local int processed = 0;
    while (true) {
        int task = -1;
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return !tasks.empty() || done; });

        if (!tasks.empty()) {
            task = tasks.front();
            tasks.pop();
            std::cout << "Потребитель " << id << " (ID " << std::this_thread::get_id()
                << ") взял задачу " << task << '\n';
            lock.unlock();

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << "Потребитель " << id << " обработал задачу " << task
                << ", квадрат = " << task * task << '\n';
            ++processed;
        }
        else if (done) {
            break;
        }
    }
    std::cout << "Потребитель " << id << " (ID " << std::this_thread::get_id()
        << ") завершил работу. Обработано задач: " << processed << '\n';
}

int main() {
    setlocale(LC_ALL, "");
    std::cout << "=== Демонстрация recursive_mutex ===\n";
    log_recursive("первый вызов");
    std::cout << "Рекурсивная блокировка выполнена успешно (deadlock не возник).\n";

    std::cout << "Главный поток: запуск производителя и потребителей.\n";
    std::thread prod(producer);
    std::vector<std::thread> consumers;
    for (int i = 0; i < 2; ++i)
        consumers.emplace_back(consumer, i + 1);

    prod.join();
    for (auto& c : consumers) c.join();
    std::cout << "Главный поток: все потоки завершены.\n";
    return 0;
}