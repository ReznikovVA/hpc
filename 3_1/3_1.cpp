#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <atomic>

const int NUM_THREADS = 4;

std::mutex mtx;
std::vector<int> byte_count(256, 0); //Вектор с количеством единиц каждого байта
std::atomic<long> bytes_read(0); //Атомарная перемнная счетчик общего числа байтов

//Выполняется каждым потоком:
void count_bytes(std::ifstream& file, std::streampos start, std::streampos end) {
    file.seekg(start);
    char byte;
    while (file.tellg() < end && file.get(byte)) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            ++byte_count[static_cast<unsigned char>(byte)]; //Инкрементим счетчик текущего байтиа
        }
        ++bytes_read; //Инкрементим общее число байт
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1], std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << argv[1] << std::endl;
        return 1;
    }

    file.seekg(0, std::ios::end);
    std::streampos file_size = file.tellg();

    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        std::streampos start = i * (file_size / NUM_THREADS); //Разделение файла для последующего чтения потоками
        std::streampos end = (i == NUM_THREADS - 1) ? file_size : static_cast<std::streampos>((i + 1) * (file_size / NUM_THREADS));
        threads.emplace_back(count_bytes, std::ref(file), start, end); //Создание треда
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "Total bytes read: " << bytes_read << std::endl;

    //Рисуем "гистограмму" с инфо по байтам :D
    for (int i = 0; i < 256; ++i) {
        if (byte_count[i] > 0) {
            std::cout << static_cast<char>(i) << " : ";
            for (int j = 0; j < byte_count[i]; ++j) {
                std::cout << "#";
            }
            std::cout << std::endl;
        }
    }

    return 0;
}
