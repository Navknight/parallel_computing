import matplotlib.pyplot as plt

# Read data from the text file
filename = 'my_super_job.o'

num_threads = []
times = []

# Reading the file
with open(filename, 'r') as file:
    for line in file:
        # Extract data from each line
        if "Num Threads:" in line and "Total Time:" in line:
            # Split based on the pattern in your file
            threads = int(line.split("Num Threads:")[1].split(",")[0].strip())
            time = float(line.split("Total Time:")[1].strip().split()[0])
            num_threads.append(threads)
            times.append(time)

# Plotting
plt.figure(figsize=(10, 6))
plt.plot(num_threads, times, marker='o', linestyle='-', color='b', label='Total Time')

# Annotate each point with time value
for i, time in enumerate(times):
    plt.text(num_threads[i], times[i], f'{time:.2f}s', fontsize=9, ha='right')

# Adding labels and title
plt.xlabel('Number of Threads')
plt.ylabel('Total Time (seconds)')
plt.title('Performance of PageRank with Varying Threads')
plt.grid(True)

# Highlight the best time (minimum)
min_time_idx = times.index(min(times))
plt.scatter(num_threads[min_time_idx], times[min_time_idx], color='r', zorder=5, label=f'Best Time: {times[min_time_idx]:.2f}s')

# Add a vertical dotted line from the best time down to the x-axis
plt.axvline(x=num_threads[min_time_idx], color='r', linestyle='--', label=f'{num_threads[min_time_idx]} Threads', ymax=0.9)

# Explicitly set the x-ticks to powers of 2 from 1 to 20 (1, 2, 4, 8, 16, 20)
plt.xticks([1, 2, 4, 8, 10, 12, 14, 16, 18, 20])

plt.legend()

# Show the plot
plt.tight_layout()

# Save the plot
plt.savefig('perf.png')  # Save as PNG

# Optionally display the plot
plt.show()
