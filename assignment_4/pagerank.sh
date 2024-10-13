#!/bin/bash
#SBATCH --job-name=cs21b060_pagerank
#SBATCH --nodes=1               # Number of nodes
#SBATCH --ntasks=1              # Number of tasks (processes)
#SBATCH --cpus-per-task=1       # Number of CPUs per task
#SBATCH --partition=defq
#SBATCH --time=59:00
#SBATCH -o my_super_job.o
#SBATCH -e my_super_job.e

# Function to run the a.out program and capture total execution time
run_executable() {
    FILENAME=$1
    N=$2
    THRESHOLD=$3
    DAMPING=$4
    NUM_THREADS=$5

    export OMP_NUM_THREADS=$NUM_THREADS

    echo "Running with FILENAME=$FILENAME, N=$N, THRESHOLD=$THRESHOLD, DAMPING=$DAMPING, NUM_THREADS=$NUM_THREADS"
    
    # Capture the output of the program
    output=$(./a.out $FILENAME $N $THRESHOLD $DAMPING $NUM_THREADS)
    
    # Extract total time using pattern matching
    total_time=$(echo "$output" | grep -oP 'Total Time = \K[\d.]+')

    # Log the results
    echo "$N, $NUM_THREADS, $total_time" >> performance_data.csv
    echo "Num Threads: $NUM_THREADS, Total Time: $total_time seconds"
    echo "----------------------------------------"
}

# Remove previous performance data
rm -f performance_data.csv
echo "N, NumThreads, TotalTime" > performance_data.csv

# Parameters for input sizes and number of threads
FILENAME="./web-Stanford.txt"
N=281903
THRESHOLD=0.0001
DAMPING=0.85
MODE=1

# Array for varying number of threads
NUM_THREADS=(1 2 4 8 10 12 14 16 18 20)

# Run the executable with varying number of threads
for THREADS in "${NUM_THREADS[@]}"; do
    run_executable $FILENAME $N $THRESHOLD $DAMPING $THREADS $MODE
done
