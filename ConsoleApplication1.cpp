#include <iostream>
#include <random>
#include <climits>
#include <chrono>
#include <omp.h>

const int rows = 500;
const int cols = 500;
const int arraySize = rows * cols;
int data[arraySize];

void generateArray(int minValue = 1, int maxValue = 100) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(minValue, maxValue);

    for (int i = 0; i < arraySize; ++i) {
        data[i] = dist(gen);
    }

    for (int j = 0; j < cols; ++j) {
        data[55 * cols + j] = -1;
    }
}

long long calculateArraySum() {
    long long totalSum = 0;
#pragma omp parallel for reduction(+:totalSum)
    for (int i = 0; i < arraySize; ++i) {
        totalSum += data[i];
    }
    return totalSum;
}

std::pair<int, int> findMinRowSum() {
    int minRowIndex = -1;
    int minRowSum = INT_MAX;

#pragma omp parallel
    {
        int localMinSum = INT_MAX;
        int localRowIndex = -1;

#pragma omp for nowait
        for (int i = 0; i < rows; ++i) {
            int rowSum = 0;
            for (int j = 0; j < cols; ++j) {
                rowSum += data[i * cols + j];
            }
            if (rowSum < localMinSum) {
                localMinSum = rowSum;
                localRowIndex = i;
            }
        }

#pragma omp critical
        {
            if (localMinSum < minRowSum) {
                minRowSum = localMinSum;
                minRowIndex = localRowIndex;
            }
        }
    }

    return { minRowIndex, minRowSum };
}

int main() {
    omp_set_num_threads(2);

    generateArray();

    long long totalSum = 0;
    int minRowIndex = -1;
    int minRowSum = 0;

    auto totalStart = std::chrono::high_resolution_clock::now();

#pragma omp parallel sections
    {
#pragma omp section
        {
            auto start = std::chrono::high_resolution_clock::now();
            totalSum = calculateArraySum();
            auto end = std::chrono::high_resolution_clock::now();
            std::cout << "Total array sum: " << totalSum
                << " (Time: " << std::chrono::duration<double>(end - start).count() << " s)\n";
        }

#pragma omp section
        {
            auto start = std::chrono::high_resolution_clock::now();
            auto result = findMinRowSum();
            minRowIndex = result.first;
            minRowSum = result.second;
            auto end = std::chrono::high_resolution_clock::now();
            std::cout << "Row with min sum: #" << minRowIndex
                << ", sum = " << minRowSum
                << " (Time: " << std::chrono::duration<double>(end - start).count() << " s)\n";
        }
    }

    auto totalEnd = std::chrono::high_resolution_clock::now();
    std::cout << "Total execution time: "
        << std::chrono::duration<double>(totalEnd - totalStart).count() << " s\n";

    return 0;
}
