#include <iostream>
#include <cstdlib>
#include <ctime>
#include <immintrin.h>
#include <chrono>
#include <iomanip>
#include <cmath>
#include <cstring>

double *allocate_matrix(int rows, int cols)
{
  double *matrix = (double *)_mm_malloc(rows * cols * sizeof(double), 32);
  for (int i = 0; i < rows * cols; i++)
  {
    matrix[i] = static_cast<double>(rand()) / RAND_MAX;
  }
  return matrix;
}

void naive_multiply(double *A, double *B, double *C, int M, int N, int K)
{
  for (int i = 0; i < M; ++i)
  {
    for (int j = 0; j < N; ++j)
    {
      C[i * N + j] = 0;
      for (int k = 0; k < K; ++k)
      {
        C[i * N + j] += A[i * K + k] * B[k * N + j];
      }
    }
  }
}

void tiled_multiply(double *A, double *B, double *C, int M, int N, int K, int tileSize)
{
  // setting C to 0
  memset(C, 0, M * N * sizeof(double));

  // Tiled multiplication
  for (int ii = 0; ii < M; ii += tileSize)
  {
    for (int jj = 0; jj < N; jj += tileSize)
    {
      for (int kk = 0; kk < K; kk += tileSize)
      {

        for (int i = ii; i < ii + tileSize && i < M; i++)
        {
          for (int j = jj; j < jj + tileSize && j < N; j++)
          {
            double sum = 0.0;
            for (int k = kk; k < kk + tileSize && k < K; k++)
            {
              sum += A[i * K + k] * B[k * N + j];
            }
            C[i * N + j] += sum;
          }
        }
      }
    }
  }
}

bool compare_matrices(double *C1, double *C2, int rows, int cols, double tolerance = 1e-9)
{
  for (int i = 0; i < rows * cols; ++i)
  {
    if (std::fabs(C1[i] - C2[i]) > tolerance)
    {
      return false;
    }
  }
  return true;
}

int main(int argc, char *argv[])
{
  if (argc != 5)
  {
    std::cerr << "Usage: " << argv[0] << " <M> <N> <K> <tile_size>" << std::endl;
    return 1;
  }

  int M = std::atoi(argv[1]);
  int N = std::atoi(argv[2]);
  int K = std::atoi(argv[3]);
  int tileSize = std::atoi(argv[4]);

  double *A = allocate_matrix(M, K);
  double *B = allocate_matrix(K, N);
  double *C_naive = allocate_matrix(M, N);
  double *C_tiled = allocate_matrix(M, N);

  auto start = std::chrono::high_resolution_clock::now();
  naive_multiply(A, B, C_naive, M, N, K);
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> naive_time = end - start;
  std::cout << std::fixed << std::setprecision(2)
            << "Naive multiplication time: " << naive_time.count() << " seconds" << std::endl;

  start = std::chrono::high_resolution_clock::now();
  tiled_multiply(A, B, C_tiled, M, N, K, tileSize);
  end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> tiled_time = end - start;
  std::cout << std::fixed << std::setprecision(2)
            << "Tiled multiplication time: " << tiled_time.count() << " seconds" << std::endl;

  bool correct = compare_matrices(C_naive, C_tiled, M, N);

  if (correct)
  {
    std::cout << "Multiplication results are correct." << std::endl;
  }
  else
  {
    std::cout << "Multiplication results are incorrect." << std::endl;
  }

  _mm_free(A);
  _mm_free(B);
  _mm_free(C_naive);
  _mm_free(C_tiled);

  return 0;
}
