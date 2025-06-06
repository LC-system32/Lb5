#include <iostream>
#include <random>
#include <climits>
#include <chrono>
#include <omp.h>

const int rows = 500;
const int cols = 500;
int data[rows][cols];

void generateArray(int minValue = 1, int maxValue = 100) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dist(minValue, maxValue);

#pragma omp parallel for collapse(2)
	for (int i = 0; i < rows; ++i)
		for (int j = 0; j < cols; ++j)
			data[i][j] = dist(gen);

	for (int j = 0; j < cols; ++j)
		data[55][j] = -1;

}

long long calculateArraySum() {
	long long totalSum = 0;

#pragma omp parallel for reduction(+:totalSum) collapse(2)
	for (int i = 0; i < rows; ++i)
		for (int j = 0; j < cols; ++j)
			totalSum += data[i][j];

	return totalSum;
}

std::pair<int, int> findMinRowSum() {
	int minIndex = -1;
	int minSum = INT_MAX;

#pragma omp parallel
	{
		int localMinSum = INT_MAX;
		int localIndex = -1;

#pragma omp for nowait
		for (int i = 0; i < rows; ++i) {
			int sum = 0;
			for (int j = 0; j < cols; ++j)
				sum += data[i][j];

			if (sum < localMinSum) {
				localMinSum = sum;
				localIndex = i;
			}
		}

#pragma omp critical
		{
			if (localMinSum < minSum) {
				minSum = localMinSum;
				minIndex = localIndex;
			}
		}
	}

	return { minIndex, minSum };
}

int main() {
	omp_set_num_threads(omp_get_max_threads());

	generateArray();

	long long totalSum = 0;
	int minRow = -1;
	int minRowSum = 0;

	auto totalStart = std::chrono::high_resolution_clock::now();

#pragma omp parallel sections
	{
#pragma omp section
		{
			auto start = std::chrono::high_resolution_clock::now();
			totalSum = calculateArraySum();
			auto end = std::chrono::high_resolution_clock::now();
			std::cout << "Total sum of elements: " << totalSum
				<< " (Time: " << std::chrono::duration<double>(end - start).count() << " s)\n";
		}

#pragma omp section
		{
			auto start = std::chrono::high_resolution_clock::now();
			auto result = findMinRowSum();
			minRow = result.first;
			minRowSum = result.second;
			auto end = std::chrono::high_resolution_clock::now();
			std::cout << "Row with minimum sum: #" << minRow
				<< ", sum = " << minRowSum
				<< " (Time: " << std::chrono::duration<double>(end - start).count() << " s)\n";
		}
	}

	auto totalEnd = std::chrono::high_resolution_clock::now();
	std::cout << "Total execution time: "
		<< std::chrono::duration<double>(totalEnd - totalStart).count() << " s\n";

	return 0;
}
