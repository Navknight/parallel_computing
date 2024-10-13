import pandas as pd
import matplotlib.pyplot as plt

# Load the CSV data
data = pd.read_csv('performance_data.csv')

# Trim whitespace from column names
data.columns = data.columns.str.strip()

# Print column names for verification
print("Column names in the DataFrame:")
print(data.columns)

# Check the first few rows of data
print(data.head())

# Plotting function for speedup (serial/parallel) vs number of threads for a fixed input size
def plot_speedup_by_threads(data, input_size):
    # Filter data for the specified input size
    filtered_data = data[data['InputSize'] == input_size].drop_duplicates(subset=['NumThreads'])

    # Check how many entries are in the filtered data
    print(f"Filtered data for Input Size {input_size}:")
    print(filtered_data)

    # Calculate speedup
    filtered_data['Speedup'] = filtered_data['SerialTime'] / filtered_data['ParallelTime']

    # Check if the filtered data has at least one entry
    if not filtered_data.empty:
        plt.figure(figsize=(10, 6))
        plt.plot(filtered_data['NumThreads'], filtered_data['Speedup'], label=f'Speedup (Input Size {input_size})', marker='o')

        # Annotate each point with its speedup value
        for i in range(len(filtered_data)):
            plt.text(filtered_data['NumThreads'].iloc[i], 
                     filtered_data['Speedup'].iloc[i], 
                     f"{filtered_data['Speedup'].iloc[i]:.2f}", 
                     ha='center', 
                     va='bottom')

        plt.title(f'Speedup by Number of Threads (Input Size {input_size})')
        plt.xlabel('Number of Threads')
        plt.ylabel('Speedup')

        # Set custom ticks for the x-axis
        xticks = filtered_data['NumThreads'].unique()
        plt.xticks(xticks)  # Use the unique thread counts directly

        # Set x-axis limits if needed
        plt.xlim(min(xticks) - 1, max(xticks) + 1)  # Adjust as necessary for better spacing
        plt.legend()
        plt.grid(True)
        plt.savefig(f'speedup_vs_threads_{input_size}_input_size.png')
        plt.show()
    else:
        print(f"No data available for Input Size {input_size}.")

# Plotting function for serial and parallel times for a fixed number of threads across input sizes
def plot_serial_parallel_times(data, num_threads):
    plt.figure(figsize=(10, 6))

    filtered_data = data[data['NumThreads'] == num_threads]
    plt.plot(filtered_data['InputSize'], filtered_data['SerialTime'], label='Serial Time', marker='o')
    plt.plot(filtered_data['InputSize'], filtered_data['ParallelTime'], label='Parallel Time', marker='o')

    plt.title(f'Serial and Parallel Times for {num_threads} Threads')
    plt.xlabel('Input Size')
    plt.ylabel('Time (seconds)')
    plt.legend()
    plt.grid(True)
    plt.savefig(f'serial_parallel_times_{num_threads}_threads.png')
    plt.show()

# Generate the two plots
# 1. Speedup for a fixed input size
plot_speedup_by_threads(data, input_size=2000)  # Plot speedup for input size 2000

# 2. Serial and Parallel Times for a fixed number of threads
plot_serial_parallel_times(data, num_threads=16)  # Plot for 20 threads
