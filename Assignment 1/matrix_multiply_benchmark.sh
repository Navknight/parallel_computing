#!/bin/bash
#SBATCH --job-name=cs21b060_matrix_multiply
#SBATCH --nodes=1               # Number of nodes
#SBATCH --ntasks=1              # Number of tasks (processes)
#SBATCH --cpus-per-task=1       # Number of CPUs per task
#SBATCH --partition=defq
#SBATCH --time=59:00
#SBATCH -o my_super_job.o
#SBATCH -e my_super_job.e


# Function to run the executable with given parameters
run_executable() {
    M=$1
    N=$2
    K=$3
    TILE_SIZE=$4
    echo "Running with M=$M, N=$N, K=$K, TILE_SIZE=$TILE_SIZE"
    ./a.out $M $N $K $TILE_SIZE
    echo "----------------------------------------"
}

# Fixed values
M_FIXED=1024
N_FIXED=1024
K_FIXED=1024
TILE_SIZE_FIXED=32

# Arrays for varying parameters
M_VALUES=(512 1024 2048 4096)
N_VALUES=(512 1024 2048 4096)
K_VALUES=(512 1024 2048 4096)
TILE_SIZES=(32 64 128 256)


# 1. Vary tile size for fixed M, N, K (M=N=K=1024)
for TILE_SIZE in "${TILE_SIZES[@]}"; do
    run_executable $M_FIXED $N_FIXED $K_FIXED $TILE_SIZE
done

# 2. Vary M for fixed N, K, and tile size (N=K=1024, tile size=32)
for M in "${M_VALUES[@]}"; do
    run_executable $M $N_FIXED $K_FIXED $TILE_SIZE_FIXED
done

# 3. Vary N for fixed M, K, and tile size (M=K=1024, tile size=32)
for N in "${N_VALUES[@]}"; do
    run_executable $M_FIXED $N $K_FIXED $TILE_SIZE_FIXED
done

# 4. Vary K for fixed M, N, and tile size (M=N=1024, tile size=32)
for K in "${K_VALUES[@]}"; do
    run_executable $M_FIXED $N_FIXED $K $TILE_SIZE_FIXED
done

# 5. Vary M, N, K together with fixed tile size (tile size=32)
for SIZE in "${M_VALUES[@]}"; do
    run_executable $SIZE $SIZE $SIZE $TILE_SIZE_FIXED
done
