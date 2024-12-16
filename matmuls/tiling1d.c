#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <omp.h>

#define DIM 1024
#define TILESIZE 64
#define NUM_THREADS 2
#define MIN(a, b) ((a) < (b) ? (a) : (b))

void matmul(float *, float *, float *);

uint64_t nanos() {
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    return (uint64_t)start.tv_sec*1000000000 + start.tv_nsec;
}


int main()
{
    int lb = 1, ub = 9;

    srand(time(NULL));

    float *A = (float *)malloc(DIM * DIM * sizeof(float));
    float *B = (float *)malloc(DIM * DIM * sizeof(float));
    float *C = (float *)malloc(DIM * DIM * sizeof(float));

    // initialize matrices
    for (int i = 0; i < DIM*DIM; i++) {
        A[i] = lb + rand() % (ub - lb + 1);
        B[i] = lb + rand() % (ub - lb + 1);
        C[i] = 0;
    }

    uint64_t start = nanos();
    matmul(A, B, C);
    uint64_t end = nanos();

    double gflop = (2.0 * DIM *DIM * DIM) * 1e-9;
    double time = (end - start) * 1e-9;

    printf("%d-dim matrix, gflops/sec %f, time-taken %f\n", DIM, gflop/time, time);

    free(A);
    free(B);
    free(C);

    return 0;
}


void matmul (float *A, float *B, float *C)
{
    #pragma omp parallel for shared(A, B, C) collapse(2) default(none) num_threads(NUM_THREADS)
    for (int ii = 0; ii < DIM; ii += 512) {
        for (int jj = 0; jj < DIM; jj += 512) {
            for (int kk = 0; kk < DIM; kk += TILESIZE) {
                for (int i = ii; i < ii+512; i++) {
                    int kTileLimit = MIN(DIM, (kk + TILESIZE));
                    for (int k = kk; k < kTileLimit; k++) {
                        for (int j = jj; j < jj+512; j +=4) {
                            C[i * DIM + j] += A[i * DIM + k] * B[k * DIM + j];
                        }
                    }
                }
            }
        }
    }
}


