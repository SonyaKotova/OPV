#include <omp.h>
#include <iostream>
#include <complex>
#include <chrono>
#include <opencv2/opencv.hpp>

int mandelbrot(double x, double y, int max_iter) {
    std::complex<double> c(x, y);
    std::complex<double> z(0, 0);
    int iter = 0;

    while (std::abs(z) < 2.0 && iter < max_iter) {
        z = z * z + c;
        iter++;
    }
    return iter;
}

cv::Vec3b getColor(int iter, int max_iter) {
    if (iter == max_iter) {
        return cv::Vec3b(0, 0, 0);
    }

    double t = (double)iter / max_iter;

    int r = (int)(9 * (1 - t) * t * t * t * 255);
    int g = (int)(15 * (1 - t) * (1 - t) * t * t * 255);
    int b = (int)(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);

    return cv::Vec3b(b, g, r);
}

int main() {
    setlocale(LC_ALL, "rus");
    const int W = 1000, H = 1000;
    const int MAX_ITER = 256;
    const double X_MIN = -2.5, X_MAX = 1.0;
    const double Y_MIN = -1.5, Y_MAX = 1.5;

    std::cout << "МНОЖЕСТВО МАНДЕЛЬБРОТА" << std::endl;
    std::cout << "Размер: " << W << "x" << H << std::endl;
    std::cout << "Макс. итераций: " << MAX_ITER << std::endl;

    std::cout << "\nПоследовательная версия..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    cv::Mat img_seq(H, W, CV_8UC3);
    for (int y = 0; y < H; y++) {
        double cy = Y_MIN + y * (Y_MAX - Y_MIN) / H;
        for (int x = 0; x < W; x++) {
            double cx = X_MIN + x * (X_MAX - X_MIN) / W;
            int iter = mandelbrot(cx, cy, MAX_ITER);
            img_seq.at<cv::Vec3b>(y, x) = getColor(iter, MAX_ITER);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    long long time_seq = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Время: " << time_seq << " мс" << std::endl;

    std::cout << "\nПараллельная версия..." << std::endl;

    std::vector<int> threads = { 2, 4, 6 };

    for (int t : threads) {
        cv::Mat img_par(H, W, CV_8UC3);
        omp_set_num_threads(t);

        auto start_par = std::chrono::high_resolution_clock::now();

#pragma omp parallel for collapse(2)
        for (int y = 0; y < H; y++) {
            double cy = Y_MIN + y * (Y_MAX - Y_MIN) / H;
            for (int x = 0; x < W; x++) {
                double cx = X_MIN + x * (X_MAX - X_MIN) / W;
                int iter = mandelbrot(cx, cy, MAX_ITER);
                img_par.at<cv::Vec3b>(y, x) = getColor(iter, MAX_ITER);
            }
        }

        auto end_par = std::chrono::high_resolution_clock::now();
        long long time_par = std::chrono::duration_cast<std::chrono::milliseconds>(end_par - start_par).count();
        double speedup = (double)time_seq / time_par;

        std::cout << "Потоков: " << t << " | Время: " << time_par
            << " мс | Ускорение: " << speedup << "x" << std::endl;
    }

    cv::Mat final_img(H, W, CV_8UC3);
    omp_set_num_threads(4);

#pragma omp parallel for collapse(2)
    for (int y = 0; y < H; y++) {
        double cy = Y_MIN + y * (Y_MAX - Y_MIN) / H;
        for (int x = 0; x < W; x++) {
            double cx = X_MIN + x * (X_MAX - X_MIN) / W;
            int iter = mandelbrot(cx, cy, MAX_ITER);
            final_img.at<cv::Vec3b>(y, x) = getColor(iter, MAX_ITER);
        }
    }

    cv::imshow("Множество Мандельброта", final_img);
    cv::imwrite("mandelbrot_result.jpg", final_img);

    std::cout << "\nИзображение сохранено" << std::endl;
    std::cout << "Нажмите любую клавишу..." << std::endl;
    cv::waitKey(0);

    return 0;
}