#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <atomic>
#include <sstream>
#include <iomanip>

const int NUM_THREADS = 4;
const int NUM_BINS = 256;

std::atomic<long> bytes_read(0);
std::atomic<int> byte_count[NUM_BINS] = {0};

//Выполняется каждым потоком:
void count_bytes(const std::string &filename, std::streampos start, std::streampos end) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file for thread: " << filename << std::endl;
        return;
    }

    file.seekg(start);
    std::streamsize chunk_size = 1024;
    std::vector<char> buffer(chunk_size);

    while (file.tellg() < end) {
        std::streamsize bytes_to_read = std::min(chunk_size, end - file.tellg());
        file.read(buffer.data(), bytes_to_read);
        std::streamsize bytes_read_chunk = file.gcount();

        for (std::streamsize i = 0; i < bytes_read_chunk; ++i) {
            ++byte_count[static_cast<unsigned char>(buffer[i])];
            ++bytes_read;
        }
    }

    file.close();
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];

    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return 1;
    }

    file.seekg(0, std::ios::end);
    std::streampos file_size = file.tellg();
    file.close();

    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        std::streampos start = i * (file_size / NUM_THREADS); //Разделение файла для последующего чтения потоками
        std::streampos end = (i == NUM_THREADS - 1) ? file_size : static_cast<std::streampos>((i + 1) * (file_size / NUM_THREADS));
        threads.emplace_back(count_bytes, filename, start, end); //Создание треда
    }

    for (auto &thread : threads) {
        thread.join();
    }

    std::cout << "Total bytes read: " << bytes_read << std::endl;

    int max_bin_count = 0;
    for (int i = 0; i < NUM_BINS; ++i) {
        max_bin_count = std::max(max_bin_count, byte_count[i].load());
    }

    //Выбираем нужную ширину отображения
    const int display_width = 20;

    std::cout << std::endl;

    //Визуализация
    for (int i = 0; i < NUM_BINS; ++i) {
        if (byte_count[i] > 0) {
            //Нормированная длина столбцов гистограммы
            double bar_length = static_cast<double>(byte_count[i].load()) / max_bin_count * display_width;

            std::cout << std::hex << std::setw(2) << std::setfill('0') << i << ": ";

            for (int j = 0; j < static_cast<int>(bar_length); ++j) {
                std::cout << '#';
            }
            std::cout << std::endl;
        }
    }

    return 0;
}
