#include <iostream>
#include <thread>
#include <chrono>

void thread_function(int thread_num, int delay_ms) {
    for (int i = 1; i <= 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        std::cout << "Поток " << thread_num << ": " << i << std::endl;
    }
    std::cout << "Поток " << thread_num << " завершился." << std::endl;
}

int main() {
    std::setlocale(LC_ALL, "");
    std::cout << "Главный поток: запуск." << std::endl;

    std::thread t1(thread_function, 1, 300); 
    std::thread t2(thread_function, 2, 200);

    t1.detach();

    for (int i = 0; i < 3; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        std::cout << "Главный поток: работает..." << std::endl;
    }

    std::cout << "Главный поток: ожидание второго потока" << std::endl;
    t2.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "Главный поток: завершение." << std::endl;
    return 0;
}
