#!/bin/bash
#SBATCH --job-name=cs21b060_matrix_multiply
#SBATCH --nodes=1               # Number of nodes
#SBATCH --ntasks=1              # Number of tasks (processes)
#SBATCH --cpus-per-task=1       # Number of CPUs per task
#SBATCH --partition=defq
#SBATCH --time=59:00
#SBATCH -o my_super_job.o
#SBATCH -e my_super_job.e

# Function to run the a.out program and capture serial and parallel times
run_executable() {
    INPUT_SIZE=$1
    NUM_THREADS=$2
    export OMP_NUM_THREADS=$NUM_THREADS

    echo "Running with INPUT_SIZE=$INPUT_SIZE, NUM_THREADS=$NUM_THREADS"
    
    # Capture the output of the program
    output=$(./a.out $INPUT_SIZE)
    
    # Extract serial and parallel times using pattern matching
    serial_time=$(echo "$output" | grep -oP 'Serial time: \K[\d.]+')
    parallel_time=$(echo "$output" | grep -oP 'Parallel time: \K[\d.]+')

    # Log the results
    echo "$INPUT_SIZE, $NUM_THREADS, $serial_time, $parallel_time" >> performance_data.csv
    echo "Serial time: $serial_time, Parallel time: $parallel_time"
    echo "----------------------------------------"
}

# Remove previous performance data
rm -f performance_data.csv
echo "InputSize, NumThreads, SerialTime, ParallelTime" > performance_data.csv

# Arrays for varying parameters
INPUT_SIZES=(10 100 500 1000 2000 3000 4000)
NUM_THREADS=(2 4 8 16 20)

# Vary number of input sizes and threads
for SIZE in "${INPUT_SIZES[@]}"; do
    for THREADS in "${NUM_THREADS[@]}"; do
        run_executable $SIZE $THREADS
    done
done
