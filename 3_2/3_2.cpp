#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include <execution>
#include <random>
#include <chrono>

constexpr size_t VECTOR_SIZE_MB = 100;
constexpr size_t VECTOR_SIZE = VECTOR_SIZE_MB * 1024 * 1024 / sizeof(int);
constexpr size_t NUM_THREADS = 4;

std::vector<int> generateRandomVector(size_t size) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000000);

    std::vector<int> vec(size);
    for (size_t i = 0; i < size; ++i) {
        vec[i] = dis(gen);
    }
    return vec;
}

void sortVectorInRange(std::vector<int>& vec, size_t start, size_t end) {
    std::sort(vec.begin() + start, vec.begin() + end);
}

int main() {
    std::vector<int> vec = generateRandomVector(VECTOR_SIZE);

    //Однопоточный сорт
    auto start = std::chrono::high_resolution_clock::now();
    std::sort(vec.begin(), vec.end());
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_single = end - start;
    std::cout << "Single-threaded sort took " << duration_single.count() << " seconds." << std::endl;

    //Многопоточный сорт
    std::vector<std::thread> threads;
    start = std::chrono::high_resolution_clock::now();
    size_t chunkSize = VECTOR_SIZE / NUM_THREADS;
    for (size_t i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(sortVectorInRange, std::ref(vec), i * chunkSize, (i + 1) * chunkSize);
    }
    for (auto& thread : threads) {
        thread.join();
    }
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_multi = end - start;
    std::cout << "Multi-threaded sort took " << duration_multi.count() << " seconds." << std::endl;

    //Паралелльная сортировка через std::execution::par
    start = std::chrono::high_resolution_clock::now();
    std::sort(std::execution::par, vec.begin(), vec.end());
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_parallel = end - start;
    std::cout << "Parallel std::execution sort took " << duration_parallel.count() << " seconds." << std::endl;

    return 0;
}
