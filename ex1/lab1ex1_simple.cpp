#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>   

#ifdef _WIN32
#include <windows.h>
#endif

#define MAX_ITER 1000

// мн-во мандельброта
// 1 - принадлежит, 0 - не принадлежит
int mandelbrot(double cr, double ci) {
    double zr = 0.0, zi = 0.0;
    int iter = 0;
    while (iter < MAX_ITER && (zr * zr + zi * zi) < 4.0) {
        double tmp = zr * zr - zi * zi + cr;
        zi = 2.0 * zr * zi + ci;
        zr = tmp;
        iter++;
    }
    return (iter == MAX_ITER) ? 1 : 0;
}

// время в секундах (монотонные часы)
double now(void) {
#ifdef _WIN32
    
    LARGE_INTEGER freq, cnt;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&cnt);
    return (double)cnt.QuadPart / (double)freq.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
#endif
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s npoints\n", argv[0]);
        return 1;
    }
    int npoints = atoi(argv[1]);

    int n_side = (int)sqrt((double)npoints);
    int total = n_side * n_side;

    const double real_min = -2.0, real_max = 1.0;
    const double imag_min = -1.5, imag_max = 1.5;
    const double dr = (real_max - real_min) / (n_side - 1);
    const double di = (imag_max - imag_min) / (n_side - 1);

    int* results = (int*)malloc(total * sizeof(int));
    if (!results) { perror("malloc"); return 1; }

    // начало замера
    double start = now();

    for (int idx = 0; idx < total; idx++) {
        int i = idx / n_side;
        int j = idx % n_side;
        double cr = real_min + j * dr;
        double ci = imag_min + i * di;
        results[idx] = mandelbrot(cr, ci);
    }

    double end = now();
    // конец замера

    printf("Time: %.6f s (sequential, npoints=%d, n_side=%d)\n",
        end - start, npoints, n_side);

    // запись ответа, вне замера
    FILE* f = fopen("mandelbrot_simple.csv", "w");
    if (!f) { perror("fopen"); free(results); return 1; }
    fprintf(f, "real,imag\n");
    for (int idx = 0; idx < total; idx++) {
        if (results[idx]) {
            int i = idx / n_side;
            int j = idx % n_side;
            double cr = real_min + j * dr;
            double ci = imag_min + i * di;
            fprintf(f, "%.10f,%.10f\n", cr, ci);
        }
    }
    fclose(f);

    free(results);
    return 0;
}