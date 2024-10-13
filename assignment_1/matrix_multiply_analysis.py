import re
import matplotlib.pyplot as plt

def parse_output_file(filename):
    results = {
        'tile_size': [],
        'M': [],
        'N': [],
        'K': [],
        'MNK': []  # New category for varying M, N, K together
    }
    current_params = None
    
    with open(filename, 'r') as f:
        for line in f:
            if line.startswith("Running with"):
                params = re.findall(r'(\w+)=(\d+)', line)
                current_params = {k: int(v) for k, v in params}
            elif "Naive multiplication time:" in line:
                naive_time = float(re.search(r'Naive multiplication time: ([\d.]+) seconds', line).group(1))
            elif "Tiled multiplication time:" in line:
                tiled_time = float(re.search(r'Tiled multiplication time: ([\d.]+) seconds', line).group(1))
                
                if current_params['M'] == current_params['N'] == current_params['K'] == 1024:
                    results['tile_size'].append((current_params['TILE_SIZE'], naive_time, tiled_time))
                elif current_params['N'] == current_params['K'] == 1024 and current_params['TILE_SIZE'] == 32:
                    results['M'].append((current_params['M'], naive_time, tiled_time))
                elif current_params['M'] == current_params['K'] == 1024 and current_params['TILE_SIZE'] == 32:
                    results['N'].append((current_params['N'], naive_time, tiled_time))
                elif current_params['M'] == current_params['N'] == 1024 and current_params['TILE_SIZE'] == 32:
                    results['K'].append((current_params['K'], naive_time, tiled_time))
                elif current_params['M'] == current_params['N'] == current_params['K'] and current_params['TILE_SIZE'] == 32:
                    results['MNK'].append((current_params['M'], naive_time, tiled_time))
    
    return results

# Parse the output file
results = parse_output_file('my_super_job.o')
print(results)
# Graph 1: Vary tile size for fixed M=N=K=1024
plt.figure(figsize=(12, 6))
tile_sizes, naive_times, tiled_times = zip(*sorted(results['tile_size']))
plt.plot(tile_sizes, tiled_times, label='Tiled Matrix Multiply', marker='o')
plt.axhline(y=naive_times[0], color='r', linestyle='--', label='Naive Matrix Multiply')
plt.xlabel('Tile Size')
plt.ylabel('Time (seconds)')
plt.title('Performance of Tiled Matrix Multiplication with Varying Tile Sizes (M=N=K=1024)')
plt.legend()
plt.grid(True)
plt.savefig('tile_size_performance.jpg')
plt.close()

# Graph 2: Vary M for fixed N=K=1024 and tile size=32
plt.figure(figsize=(12, 6))
M_values, naive_times, tiled_times = zip(*sorted(results['M']))
plt.plot(M_values, naive_times, label='Naive Matrix Multiply', marker='o')
plt.plot(M_values, tiled_times, label='Tiled Matrix Multiply (tile size=32)', marker='x')
plt.xlabel('M (Number of Rows)')
plt.ylabel('Time (seconds)')
plt.title('Performance Comparison by Varying M (Rows) for N=K=1024, Tile Size=32')
plt.legend()
plt.grid(True)
plt.savefig('vary_M_performance.jpg')
plt.close()

# Graph 3: Vary N for fixed M=K=1024 and tile size=32
plt.figure(figsize=(12, 6))
N_values, naive_times, tiled_times = zip(*sorted(results['N']))
plt.plot(N_values, naive_times, label='Naive Matrix Multiply', marker='o')
plt.plot(N_values, tiled_times, label='Tiled Matrix Multiply (tile size=32)', marker='x')
plt.xlabel('N (Number of Columns)')
plt.ylabel('Time (seconds)')
plt.title('Performance Comparison by Varying N (Columns) for M=K=1024, Tile Size=32')
plt.legend()
plt.grid(True)
plt.savefig('vary_N_performance.jpg')
plt.close()

# Graph 4: Vary K for fixed M=N=1024 and tile size=32
plt.figure(figsize=(12, 6))
K_values, naive_times, tiled_times = zip(*sorted(results['K']))
plt.plot(K_values, naive_times, label='Naive Matrix Multiply', marker='o')
plt.plot(K_values, tiled_times, label='Tiled Matrix Multiply (tile size=32)', marker='x')
plt.xlabel('K (Depth)')
plt.ylabel('Time (seconds)')
plt.title('Performance Comparison by Varying K (Depth) for M=N=1024, Tile Size=32')
plt.legend()
plt.grid(True)
plt.savefig('vary_K_performance.jpg')
plt.close()

# Graph 5: Vary M, N, K together with fixed tile size=32
plt.figure(figsize=(12, 6))
MNK_values, naive_times, tiled_times = zip(*sorted(results['MNK']))
plt.plot(MNK_values, naive_times, label='Naive Matrix Multiply', marker='o')
plt.plot(MNK_values, tiled_times, label='Tiled Matrix Multiply (tile size=32)', marker='x')
plt.xlabel('Matrix Size (M=N=K)')
plt.ylabel('Time (seconds)')
plt.title('Performance Comparison by Varying Matrix Size (M=N=K) with Tile Size=32')
plt.legend()
plt.grid(True)
plt.savefig('vary_MNK_performance.jpg')
plt.close()
