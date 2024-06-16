#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <mpi.h>

constexpr size_t VECTOR_SIZE_MB = 100;
constexpr size_t VECTOR_SIZE = VECTOR_SIZE_MB * 1024 * 1024 / sizeof(int);

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

void check_sorted(const std::vector<int>& v) {
    for (size_t i = 1; i < v.size(); i++) {
        if (v[i] < v[i-1]) {
            std::cerr << "Wrong order in position " << i - 1 << ": " << v[i-1] << ", " << v[i] << std::endl; 
            exit(EXIT_FAILURE);
        }
    }   
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    std::vector<int> vec;
    if (world_rank == 0) {
        vec = generateRandomVector(VECTOR_SIZE);
    }
    
    size_t local_size = VECTOR_SIZE / world_size;
    std::vector<int> local_vec(local_size);
    
    MPI_Scatter(vec.data(), local_size, MPI_INT, local_vec.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);
    
    auto start = std::chrono::steady_clock::now();
    std::sort(local_vec.begin(), local_vec.end());
    auto end = std::chrono::steady_clock::now();
    
    std::vector<int> sorted_vec;
    if (world_rank == 0) {
        sorted_vec.resize(VECTOR_SIZE);
    }
    
    MPI_Gather(local_vec.data(), local_size, MPI_INT, sorted_vec.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (world_rank == 0) {
        auto start_merge = std::chrono::steady_clock::now();
        
        for (int i = 1; i < world_size; ++i) {
            std::inplace_merge(sorted_vec.begin(), sorted_vec.begin() + i * local_size, sorted_vec.begin() + (i + 1) * local_size);
        }
        
        auto end_merge = std::chrono::steady_clock::now();
        
        std::chrono::duration<double> duration_merge = end_merge - start_merge;
        std::cout << "Мёрж занял " << duration_merge.count() << " секунд" << std::endl;
        
        check_sorted(sorted_vec);
    }
    
    auto duration_local_sort = end - start;
    std::cout << "Процесс " << world_rank << " завершил за " << duration_local_sort.count() << " секунд" << std::endl;
    

    MPI_Finalize();
    return 0;
}
