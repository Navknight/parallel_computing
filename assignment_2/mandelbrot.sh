#!/bin/bash
#SBATCH --job-name=cs21b060_matrix_multiply
#SBATCH --nodes=1               # Number of nodes
#SBATCH --ntasks=1              # Number of tasks (processes)
#SBATCH --cpus-per-task=1       # Number of CPUs per task
#SBATCH --partition=defq
#SBATCH --time=59:00
#SBATCH -o my_super_job.o
#SBATCH -e my_super_job.e

run_executable() {
    WIDTH=$1
    HEIGHT=$2
    OMP=$3
    NUM_THREADS=$4
    OUTPUT_FILE="mandelbrot_w${WIDTH}_h${HEIGHT}_n${NUM_THREADS}.png"
    echo "Running with WIDTH=$WIDTH, HEIGHT=$HEIGHT, OMP=$OMP, NUM_THREADS=$NUM_THREADS"
    if [ $OMP -eq 1 ]; then
        ./a.out -w $WIDTH -h $HEIGHT -n $NUM_THREADS -o $OUTPUT_FILE -O
    else
        ./a.out -w $WIDTH -h $HEIGHT -n $NUM_THREADS -o $OUTPUT_FILE
    fi
    echo "Output saved to $OUTPUT_FILE"
    echo "----------------------------------------"
}

# Arrays for varying parameters
WIDTHS=(480 960 1920 2048)
HEIGHTS=(480 960 1920 2048)
NUM_THREADS=(2 4 8 16 32 64 128 256)
# 1. Vary number of threads for fixed width and height
for THREADS in "${NUM_THREADS[@]}"; do
    export OMP_NUM_THREADS=$THREADS
    run_executable 2048 2048 0 $THREADS
    run_executable 2048 2048 1 $THREADS
    run_executable 2048 2048 0 1
done

# 2. Vary width for fixed height and number of threads
for WIDTH in "${WIDTHS[@]}"; do
    run_executable $WIDTH 480 0 4
done

# 3. Vary height for fixed width and number of threads
for HEIGHT in "${HEIGHTS[@]}"; do
    run_executable 480 $HEIGHT 0 4
done

