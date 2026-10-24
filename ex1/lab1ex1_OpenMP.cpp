#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

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

int main(int argc, char** argv) {
    // подготовка, нет смысла распаралеливать
    if (argc != 3) {
        fprintf(stderr, "Usage: %s nthreads npoints\n", argv[0]);
        return 1;
    }
    int nthreads = atoi(argv[1]);
    int npoints = atoi(argv[2]);

    int n_side = (int)sqrt((double)npoints);   
    int total = n_side * n_side;              

    const double real_min = -2.0, real_max = 1.0;
    const double imag_min = -1.5, imag_max = 1.5;
    const double dr = (real_max - real_min) / (n_side - 1);
    const double di = (imag_max - imag_min) / (n_side - 1);

    int* results = (int*)malloc(total * sizeof(int));
    if (!results) { perror("malloc"); return 1; }

    // начало замера
    double start = omp_get_wtime();

#pragma omp parallel for num_threads(nthreads) schedule(dynamic) \
        default(none) shared(results, n_side, dr, di, real_min, imag_min, total)
    for (int idx = 0; idx < total; idx++) {
        int i = idx / n_side;          
        int j = idx % n_side;          
        double cr = real_min + j * dr;
        double ci = imag_min + i * di;
        results[idx] = mandelbrot(cr, ci);
    }

    double end = omp_get_wtime();
    // конец замера

    printf("Time: %.6f s (nthreads=%d, npoints=%d, n_side=%d)\n",
        end - start, nthreads, npoints, n_side);

    // запись ответа, вне замера
    FILE* f = fopen("mandelbrot.csv", "w");
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
