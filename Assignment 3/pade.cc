#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <chrono>
#include <omp.h>

struct MyMax
{
    double max;
    int index;
};

static double sqrarg;
#define SQR(a) ((sqrarg = (a)) == 0.0 ? 0.0 : sqrarg * sqrarg)

#define TINY 1e-20
#define BIG 1.0e30

double **dmatrix(int nrl, int nrh, int ncl, int nch)
{
    double **m;
    m = (double **)malloc((nrh - nrl + 1) * sizeof(double *));
    for (int i = 0; i <= nrh - nrl; i++)
    {
        m[i] = (double *)malloc((nch - ncl + 1) * sizeof(double));
    }
    return m;
}

double *dvector(int nl, int nh)
{
    return (double *)malloc((nh - nl + 1) * sizeof(double));
}

int *ivector(int nl, int nh)
{
    return (int *)malloc((nh - nl + 1) * sizeof(int));
}

void free_dmatrix(double **m, int nrl, int nrh, int ncl, int nch)
{
    for (int i = 0; i <= nrh - nrl; i++)
    {
        free(m[i]);
    }
    free(m);
}

void free_dvector(double *v, int nl, int nh)
{
    free(v);
}

void free_ivector(int *v, int nl, int nh)
{
    free(v);
}

// LU decomposition
void ludcmp(double **a, int n, int *indx, double *d)
{
    int i, imax, j, k;
    double big, dum, sum, temp;
    double *vv;

    vv = dvector(0, n - 1);
    *d = 1.0;
    for (i = 0; i < n; i++)
    {
        big = 0.0;
        for (j = 0; j < n; j++)
        {
            if ((temp = fabs(a[i][j])) > big)
                big = temp;
        }
        if (big == 0.0)
        {
            printf("Singular matrix in routine ludcmp\n");
            exit(1);
        }
        vv[i] = 1.0 / big;
    }
    for (j = 0; j < n; j++)
    {
        for (i = 0; i < j; i++)
        {
            sum = a[i][j];
            for (k = 0; k < i; k++)
                sum -= a[i][k] * a[k][j];
            a[i][j] = sum;
        }
        big = 0.0;
        for (i = j; i < n; i++)
        {
            sum = a[i][j];
            for (k = 0; k < j; k++)
                sum -= a[i][k] * a[k][j];
            a[i][j] = sum;
            if ((dum = vv[i] * fabs(sum)) >= big)
            {
                big = dum;
                imax = i;
            }
        }
        if (j != imax)
        {
            for (k = 0; k < n; k++)
            {
                dum = a[imax][k];
                a[imax][k] = a[j][k];
                a[j][k] = dum;
            }
            *d = -(*d);
            vv[imax] = vv[j];
        }
        indx[j] = imax;
        if (fabs(a[j][j]) < 1e-6)
        {
            printf("Zero pivot in ludcmp. Regularizing...\n");
            a[j][j] = 1e-6;
        }
    }
    free_dvector(vv, 0, n - 1);
}

void ludcmp_parallel(double **a, int n, int *indx, double *d)
{
    int i, imax, j, k;
    double big, dum, sum, temp;
    double *vv = dvector(0, n-1);
    *d = 1.0;

#pragma omp parallel for private(i, j, big, temp)
    for (i = 0; i < n; i++)
    {
        big = 0.0;
        for (j = 0; j < n; j++)
        {
            if ((temp = fabs(a[i][j])) > big)
                big = temp;
        }
        if (big == 0.0)
        {
            printf("Singular matrix in routine ludcmp\n");
            exit(1);
        }
        vv[i] = 1.0 / big;
    }

    for (j = 0; j < n; j++)
    {
#pragma omp parallel for private(i, sum, k)
        for (i = 0; i < j; i++)
        {
            sum = a[i][j];
#pragma omp simd reduction(- : sum)
            for (k = 0; k < i; k++)
            {
                sum -= a[i][k] * a[k][j];
            }
            a[i][j] = sum;
        }

        big = 0.0;

#pragma omp parallel for private(i, sum, k, dum) reduction(max : big)
        for (i = j; i < n; i++)
        {
            sum = a[i][j];
#pragma omp simd reduction(- : sum)
            for (k = 0; k < j; k++)
            {
                sum -= a[i][k] * a[k][j];
            }
            a[i][j] = sum;
            if ((dum = vv[i] * fabs(sum)) >= big)
            {
                big = dum;
                imax = i;
            }
        }

        if (j != imax)
        {
#pragma omp parallel for private(k, dum)
            for (k = 0; k < n; k++)
            {
                dum = a[imax][k];
                a[imax][k] = a[j][k];
                a[j][k] = dum;
            }
            *d = -(*d);
            vv[imax] = vv[j];
        }

        indx[j] = imax;

        if (fabs(a[j][j]) < 1e-6)
        {
            printf("Zero pivot in ludcmp. Regularizing...\n");
            a[j][j] = 1e-6;
        }
    }

    free_dvector(vv, 0, n-1);
}

// backsubstitution
void lubksb(double **a, int n, int *indx, double b[])
{
    int i, ii = 0, ip, j;
    double sum;

    for (i = 0; i < n; i++)
    {
        ip = indx[i];
        sum = b[ip];
        b[ip] = b[i];
        if (ii)
            for (j = ii; j < i; j++)
                sum -= a[i][j] * b[j];
        else if (sum)
            ii = i;
        b[i] = sum;
    }
    for (i = n - 1; i >= 0; i--)
    {
        sum = b[i];
        for (j = i + 1; j < n; j++)
            sum -= a[i][j] * b[j];
        b[i] = sum / a[i][i];
    }
}

void lubksb_parallel(double **a, int n, int *indx, double b[])
{
    int i, ii = -1, ip, j;
    double sum;

#pragma omp parallel for private(i, ip, sum)
    for (i = 0; i < n; i++)
    {
        ip = indx[i];
        sum = b[ip];
        b[ip] = b[i];
        b[i] = sum;
    }

#pragma omp parallel
    {
        int local_ii = -1;

#pragma omp for private(i, j, sum)
        for (i = 0; i < n; i++)
        {
            sum = b[i];
            if (local_ii >= 0)
            {
#pragma omp simd reduction(- : sum)
                for (j = local_ii; j < i; j++)
                {
                    sum -= a[i][j] * b[j];
                }
            }
            else if (sum != 0.0)
            {
                local_ii = i;
            }
            b[i] = sum;
        }
#pragma omp critical
        if (local_ii != -1 && (ii == -1 || local_ii < ii))
        {
            ii = local_ii;
        }
    }

#pragma omp parallel for private(i, j, sum)
    for (i = n - 1; i >= 0; i--)
    {
        sum = b[i];
#pragma omp simd reduction(- : sum)
        for (j = i + 1; j < n; j++)
        {
            sum -= a[i][j] * b[j];
        }
        b[i] = sum / a[i][i];
    }
}

// Serial Pade approximation function
void serial_pade(double cof[], int n, double *resid)
{
    int j, k, *indx;
    double **a, *b;

    a = dmatrix(0, n - 1, 0, n - 1);
    b = dvector(0, n - 1);
    indx = ivector(0, n - 1);

    for (j = 0; j < n; j++)
    {
        for (k = 0; k < n; k++)
        {
            a[j][k] = (j == k) ? (1.0 + j * 0.1) : 0.1;
        }
        b[j] = 0.1 * (j + 1);
    }

    ludcmp(a, n, indx, resid);
    lubksb(a, n, indx, b);

    for (int i = 0; i < n; i++)
    {
        cof[i] = b[i];
    }

    free_dmatrix(a, 0, n - 1, 0, n - 1);
    free_dvector(b, 0, n - 1);
    free_ivector(indx, 0, n - 1);
}

// Parallel Pade approximation function with OpenMP
void parallel_pade(double cof[], int n, double *resid)
{
    // your code here
    // you can copy the serial code from serial_pade and parallelize it
    // remember to call parallel versions of ludcmp and lubksb
    int i, j, k, *indx;
    double **a, *b;

    a = dmatrix(0, n - 1, 0, n - 1);
    b = dvector(0, n - 1);
    indx = ivector(0, n - 1);

#pragma omp parallel for private(j, k)
    for (i = 0; i < n * n; i++)
    {
        j = i / n;
        k = i % n;

        a[j][k] = (j == k) ? (1.0 + j * 0.1) : 0.1;

        b[j] = 0.1 * (j + 1);
    }

    ludcmp_parallel(a, n, indx, resid);
    lubksb_parallel(a, n, indx, b);

    for (int i = 0; i < n; i++)
    {
        cof[i] = b[i];
    }

    free_dmatrix(a, 0, n - 1, 0, n - 1);
    free_dvector(b, 0, n - 1);
    free_ivector(indx, 0, n - 1);
}

// Compare serial and parallel results
bool check_results(double *serial_results, double *parallel_results, int size)
{
    bool match = true;
    for (int i = 0; i < size; i++)
    {
        if (fabs(serial_results[i] - parallel_results[i]) > 1e-10)
        {
            printf("Mismatch at index %d: Serial: %.15f, Parallel: %.15f\n", i, serial_results[i], parallel_results[i]);
            match = false;
        }
    }
    return match;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <n>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    if (n <= 0)
    {
        printf("n must be a positive integer\n");
        return 1;
    }

    double resid;
    double *c_serial, *c_parallel;

    c_serial = dvector(0, n - 1);
    c_parallel = dvector(0, n - 1);

    auto start_serial = std::chrono::high_resolution_clock::now();
    serial_pade(c_serial, n, &resid);
    auto end_serial = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> serial_time = end_serial - start_serial;

    auto start_parallel = std::chrono::high_resolution_clock::now();
    parallel_pade(c_parallel, n, &resid);
    auto end_parallel = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> parallel_time = end_parallel - start_parallel;

    if (check_results(c_serial, c_parallel, n))
    {
        printf("The serial and parallel results match.\n");
    }
    else
    {
        printf("The serial and parallel results do not match.\n");
    }

    printf("Serial time: %.6f seconds\n", serial_time.count());
    printf("Parallel time: %.6f seconds\n", parallel_time.count());

    free_dvector(c_serial, 0, n - 1);
    free_dvector(c_parallel, 0, n - 1);
    return 0;
}