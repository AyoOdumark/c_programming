#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define LOWER_BOUND 1
#define UPPER_BOUND 9

typedef void (*MatmulFunc)(int dim, int block_size, double *A, double *B, double *C);

void naive(int dim, int block_size, double *A, double *B, double *C)
{
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            for (int k = 0; k < dim; k++) {
                C[i * dim + j] += A[i * dim + k] * B[k * dim + j];
            }
        }
    }
}

void loopReorder(int dim, int block_size, double *A, double *B, double *C)
{
    double r;
    
    for (int i = 0; i < dim; i++) {
        for (int k = 0; k < dim; k++) {
            r = A[i * dim + k];
            for (int j = 0; j < dim; j++) {
                C[i * dim + j] += r * B[k * dim + j];
            }
        }
    }
}

void tiling(int dim, int block_size, double *A, double *B, double *C)
{
    for (int ii = 0; ii < dim; ii += block_size) {
        for (int jj = 0; jj < dim; jj += block_size) {
            for (int kk = 0; kk < dim; kk += block_size) {
                for (int i = ii; i < ii+block_size; i++) {
                    for (int j = jj; j < jj+block_size; j++) {
                        for (int k = kk; k < kk+block_size; k++) {
                            C[i * dim + j] += A[i * dim + k] * B[k * dim + j];
                        }
                    }
                }
            }
        }
    }
}

uint64_t nanos() {
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    return (uint64_t)start.tv_sec*1000000000 + start.tv_nsec;
}

void benchmark_algorithm(MatmulFunc func, const char *name, FILE *output_file, int N, int runs, int block_size) 
{
    
    double *A = (double *)malloc(N * N * sizeof(double));
    double *B = (double *)malloc(N * N * sizeof(double));
    double *C = (double *)malloc(N * N * sizeof(double));
    
    // Initialize matrices
    for (int i = 0; i < N*N; i++) {
        A[i] = LOWER_BOUND + rand() % (UPPER_BOUND - LOWER_BOUND + 1);
        B[i] = LOWER_BOUND + rand() % (UPPER_BOUND - LOWER_BOUND + 1);
        C[i] = 0.0;
    }

    double total_time = 0.0;

    for (int r = 0; r < runs; r++) {
        memset(C, 0, N * N * sizeof(double));
        uint64_t start = nanos();
        func(N, block_size, A, B, C);
        uint64_t end = nanos();

        double time = (end - start) * 1e-9;
        total_time += time;
    }

    double avg_time = total_time / runs;
    double gflops = (2.0 * N * N * N) / (avg_time * 1e9);

    fprintf(output_file, "%s, %d, %d, %.6f, %.6f\n", name, N, block_size, avg_time, gflops);

    free(A);
    free(B);
    free(C);

}

void benchmark_all_algorithms(FILE *output_file, int *sizes, int num_sizes, int runs, int *block_sizes, int num_blocks)
{
    fprintf(output_file, "Algorithm, Matrix Size, Block Size, Avg Time (s), Avg GFLOPS\n");

    MatmulFunc matmul_algorithms[] = {naive, loopReorder, tiling};
    const char *algorithm_names[] = {"Naive", "LoopReorder", "Tiling"};
    int num_algorithms = sizeof(matmul_algorithms) / sizeof(matmul_algorithms[0]);

    for (int i = 0; i < num_algorithms; i++) {
        for (int j = 0; j < num_sizes; j++) {
            int N = sizes[j];

            for (int k = 0; k < (strcmp(algorithm_names[i], "Tiling") == 0 ? num_blocks : 1); k++) {
                int block_size = (strcmp(algorithm_names[i], "Tiling") == 0) ? block_sizes[k] : 0;
                benchmark_algorithm(matmul_algorithms[i], algorithm_names[i], output_file, N, runs, block_size);
            }
        }
    }
}

int main() 
{
    srand(time(NULL));

    int sizes[] = {64, 128, 256, 512, 1024, 2048};           // Matrix sizes
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);        
    int num_runs = 5;                                        // Number of runs per algorithm and size 
    
    // Block sizes for tiled algorithms
    int block_sizes[] = {32, 64, 128, 256};
    int num_blocks = sizeof(block_sizes) / sizeof(block_sizes[0]);

    FILE *output_file = fopen("benchmark_results.csv", "w");
    if (!output_file) {
        fprintf(stderr, "Failed to open output file\n");
        return 1;
    }

    benchmark_all_algorithms(output_file, sizes, num_sizes, num_runs, block_sizes, num_blocks);
    
    fclose(output_file);

    printf("Benchmarking complete. Results saved to benchmark_results.csv\n");
    return 0;
}


    
    

