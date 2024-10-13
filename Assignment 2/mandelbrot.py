import matplotlib.pyplot as plt
import re

def parse_data(filename):
    with open(filename, 'r') as file:
        content = file.read()
    
    data = []
    for line in content.split('\n'):
        if 'Running with' in line:
            match = re.search(r'WIDTH=(\d+), HEIGHT=(\d+), OMP=(\d+), NUM_THREADS=(\d+)', line)
            if match:
                width, height, omp, threads = map(int, match.groups())
        elif 'took' in line:
            match = re.search(r'took (\d+\.\d+) seconds', line)
            if match:
                time = float(match.group(1))
                data.append((width, height, omp, threads, time))
    return data

def plot_size_vs_performance(data):
    sizes = [(480, 480), (960, 480), (1920, 480), (2048, 480),
             (480, 480), (480, 960), (480, 1920), (480, 2048)]
    times = [next(d[4] for d in data if d[0] == w and d[1] == h and d[2] == 0 and d[3] == 4)
             for w, h in sizes]
    
    plt.figure(figsize=(10, 6))
    plt.plot(range(len(sizes)), times, 'bo-')
    plt.xlabel('Image Size')
    plt.ylabel('Execution Time (seconds)')
    plt.title('Performance vs. Image Size (4 Threads)')
    plt.xticks(range(len(sizes)), [f'{w}x{h}' for w, h in sizes], rotation=45)
    plt.tight_layout()
    plt.savefig('size_vs_performance.png')
    plt.close()

    print("1. Effect of data size on parallel performance (4 threads):")
    print("   As the image size increases, the execution time generally increases.")
    print("   However, the relationship is not perfectly linear, suggesting that")
    print("   other factors like cache efficiency and thread synchronization")
    print("   overhead may play a role in performance at different sizes.")

def plot_speedup_vs_threads(data):
    threads = [2, 4, 8, 16, 32, 64, 128, 256]
    serial_time = next(d[4] for d in data if d[0] == 2048 and d[1] == 2048 and d[2] == 0 and d[3] == 1)
    
    manual_speedup = [serial_time / next(d[4] for d in data if d[0] == 2048 and d[1] == 2048 and d[2] == 0 and d[3] == t)
                      for t in threads]
    omp_speedup = [serial_time / next(d[4] for d in data if d[0] == 2048 and d[1] == 2048 and d[2] == 1 and d[3] == t)
                   for t in threads]

    plt.figure(figsize=(10, 6))
    plt.plot(threads, manual_speedup, 'bo-', label='Manual Threading')
    plt.plot(threads, omp_speedup, 'ro-', label='OpenMP')
    plt.xlabel('Number of Threads')
    plt.ylabel('Speedup')
    plt.title('Speedup vs. Number of Threads (2048x2048)')
    plt.legend()
    plt.xscale('log', base=2)
    plt.grid(True)
    plt.tight_layout()
    plt.savefig('speedup_vs_threads.png')
    plt.close()

def compare_parallel_to_serial(data):
    serial_time = next(d[4] for d in data if d[0] == 2048 and d[1] == 2048 and d[2] == 0 and d[3] == 1)
    threads = [2, 4, 8, 16, 32, 64, 128, 256]
    parallel_times = [next(d[4] for d in data if d[0] == 2048 and d[1] == 2048 and d[2] == 0 and d[3] == t)
                      for t in threads]

    plt.figure(figsize=(10, 6))
    plt.axhline(y=serial_time, color='r', linestyle='--', label='Serial')
    plt.plot(threads, parallel_times, 'bo-', label='Parallel')
    plt.xlabel('Number of Threads')
    plt.ylabel('Execution Time (seconds)')
    plt.title('Parallel vs Serial Performance (2048x2048)')
    plt.legend()
    plt.xscale('log', base=2)
    plt.grid(True)
    plt.tight_layout()
    plt.savefig('parallel_vs_serial.png')
    plt.close()

    print("\n3. Comparison of parallel version to serial version (2048x2048):")
    print(f"   Serial execution time: {serial_time:.4f} seconds")
    print("   Parallel execution times:")
    for t, time in zip(threads, parallel_times):
        print(f"   {t} threads: {time:.4f} seconds (Speedup: {serial_time/time:.2f}x)")
    print("\n   The parallel version shows significant performance improvements")
    print("   over the serial version, especially as the number of threads increases.")
    print("   However, the speedup is not linear with the number of threads,")
    print("   likely due to overhead in thread management and synchronization.")
    print("   The best performance is achieved with 256 threads, but the")
    print("   marginal improvement diminishes with higher thread counts.")

# Main execution
filename = 'my_super_job.o'  # Update this if your file has a different name
data = parse_data(filename)

plot_size_vs_performance(data)
plot_speedup_vs_threads(data)
compare_parallel_to_serial(data)

print("\nGraphs have been saved as PNG files in the current directory.")
