#include <omp.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>

int main() {
    setlocale(LC_ALL, "rus");

    const long long N = 10000000;
    const int NUM_THREADS = 7;

    std::vector<double> a(N);

    std::cout << "Вычисление суммы элементов" << std::endl;
    std::cout << "Количество элементов: " << N << std::endl;
    std::cout << "Количество потоков: " << NUM_THREADS << std::endl;

    for (long long i = 0; i < N; ++i) {
        a[i] = i * 0.5;
    }

    double sum_sequential = 0.0;
    double sum_parallel = 0.0;

    std::cout << "Последовательная версия:" << std::endl;

    auto start_seq = std::chrono::high_resolution_clock::now();

    for (long long i = 0; i < N; ++i) {
        sum_sequential += a[i];
    }

    auto end_seq = std::chrono::high_resolution_clock::now();
    auto duration_seq = std::chrono::duration_cast<std::chrono::milliseconds>(end_seq - start_seq);

    std::cout << "Сумма элементов = " << std::fixed << std::setprecision(2) << sum_sequential << std::endl;
    std::cout << "Время выполнения: " << duration_seq.count() << " мс" << std::endl;
    std::cout << std::endl;

    std::cout << "Параллельная версия (OpenMp)" << std::endl;

    omp_set_num_threads(NUM_THREADS);
    omp_set_dynamic(0);

    auto start_par = std::chrono::high_resolution_clock::now();

#pragma omp parallel for reduction(+:sum_parallel)
    for (long long i = 0; i < N; ++i) {
        sum_parallel += a[i];
    }

    auto end_par = std::chrono::high_resolution_clock::now();
    auto duration_par = std::chrono::duration_cast<std::chrono::milliseconds>(end_par - start_par);

    std::cout << "Сумма элементов = " << std::fixed << std::setprecision(2) << sum_parallel << std::endl;
    std::cout << "Время выполнения: " << duration_par.count() << " мс" << std::endl;
    std::cout << std::endl;

    std::cout << "Сравнение результатов" << std::endl;
    std::cout << "Последовательная версия: " << duration_seq.count() << " мс" << std::endl;
    std::cout << "Параллельная версия:     " << duration_par.count() << " мс" << std::endl;

    if (duration_par.count() > 0) {
        double speedup = (double)duration_seq.count() / duration_par.count();
        std::cout << "Ускорение:               " << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;
    }

    std::cout << "\nПроверка корректности:" << std::endl;
    if (sum_sequential == sum_parallel) {
        std::cout << " Результаты совпадают!" << std::endl;
        std::cout << "  Сумма (посл): " << std::fixed << std::setprecision(2) << sum_sequential << std::endl;
        std::cout << "  Сумма (пар):  " << std::fixed << std::setprecision(2) << sum_parallel << std::endl;
    }
    else {
        std::cout << " Ошибка: результаты не совпадают!" << std::endl;
        std::cout << "  Разница: " << (sum_sequential - sum_parallel) << std::endl;
    }

    return 0;
}