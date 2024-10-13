#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <chrono>
#include <thread>
#include <omp.h>

struct RGB {
  unsigned char r, g, b;
};

void writeImage(const char* filename, int width, int height, const std::vector<RGB>& image) 
{
  std::ofstream ofs(filename, std::ios::out | std::ios::binary);
  ofs << "P6" << "\n" << width << " " << height << "\n255\n";
  for (int i = 0; i < width * height; ++i) {
    ofs << image[i].r << image[i].g << image[i].b;
  }
  ofs.close();
}

RGB getColor(int iterations, int maxIterations) 
{
  if (iterations == maxIterations) return {255, 255, 255};
  int r = (iterations * 9) % 256;
  int g = (iterations * 2) % 256;
  int b = (iterations * 5) % 256;
  return {static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b)};
}

int mandelbrot(double x0, double y0, int maxIterations) 
{
  double x = 0.0, y = 0.0;
  int iterations = 0;
  while (x * x + y * y <= 4.0 && iterations < maxIterations) {
    double xtemp = x * x - y * y + x0;
    y = 2.0 * x * y + y0;
    x = xtemp;
    iterations++;
  }
  return iterations;
}

void computeMandelbrotSection(int width, int height, int maxIterations, std::vector<RGB>& image, int startRow, int endRow) 
{
  for (int i = 0; i < width; ++i) {
    for (int j = startRow; j < endRow; ++j) {
      double x0 = (i - width / 2.0) * 4.0 / width;
      double y0 = (j - height / 2.0) * 4.0 / height;
      int iterations = mandelbrot(x0, y0, maxIterations);
      image[j * width + i] = getColor(iterations, maxIterations);
    }
  }
}

void parallelMandelbrot(int width, int height, int maxIterations, const char* filename, int numThreads) 
{
  auto start = std::chrono::high_resolution_clock::now();
  std::vector<RGB> image(width * height);
  // your code here
  std::vector<std::thread> threads;
  int numRows = height / numThreads;
  for(int t = 0; t < numThreads; ++t){
    int startRow = t * numRows;
    int endRow = (t == numThreads - 1)? height : startRow + numRows;
    threads.emplace_back(computeMandelbrotSection, width, height, maxIterations, std::ref(image), startRow, endRow); 
  }
  for(int t = 0; t < numThreads; t++)
    threads[t].join();
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  std::cout << "Parallel Mandelbrot with " << numThreads << " threads took " << elapsed.count() << " seconds\n";

  writeImage(filename, width, height, image);
}

void serialMandelbrot(int width, int height, int maxIterations, const char* filename) 
{
  auto start = std::chrono::high_resolution_clock::now();
  std::vector<RGB> image(width * height);
  for (int i = 0; i < width; ++i) {
    for (int j = 0; j < height; ++j) {
      double x0 = (i - width / 2.0) * 4.0 / width;
      double y0 = (j - height / 2.0) * 4.0 / height;
      int iterations = mandelbrot(x0, y0, maxIterations);
      image[j * width + i] = getColor(iterations, maxIterations);
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  std::cout << "Serial Mandelbrot took " << elapsed.count() << " seconds\n";
  writeImage(filename, width, height, image);
}

void OMPMandelbrot(int width, int height, int maxIterations, const char* filename) 
{
  auto start = std::chrono::high_resolution_clock::now();
  std::vector<RGB> image(width * height);
  // your code here
  #pragma omp parallel for collapse(2)
  for (int i = 0; i < width; ++i) {
    for (int j = 0; j < height; ++j) {
      double x0 = (i - width / 2.0) * 4.0 / width;
      double y0 = (j - height / 2.0) * 4.0 / height;
      int iterations = mandelbrot(x0, y0, maxIterations);
      image[j * width + i] = getColor(iterations, maxIterations);
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  std::cout << "OMP Mandelbrot took " << elapsed.count() << " seconds\n";
  writeImage(filename, width, height, image);
}

int main(int argc, char* argv[]) 
{
  int width = 480, height = 480, maxIterations = 1000, numThreads = 1;
  bool useOMP = false;
  const char* filename = "out.png";
  
  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "-o") filename = argv[++i];
    else if (std::string(argv[i]) == "-w") width = std::atoi(argv[++i]);
    else if (std::string(argv[i]) == "-h") height = std::atoi(argv[++i]);
    else if (std::string(argv[i]) == "-n") numThreads = std::atoi(argv[++i]);
    else if (std::string(argv[i]) == "-O") useOMP = true;
  }

  if (useOMP) OMPMandelbrot(width, height, maxIterations, filename);
  else if (numThreads > 1) parallelMandelbrot(width, height, maxIterations, filename, numThreads);
  else serialMandelbrot(width, height, maxIterations, filename);
  
  return 0;
}
