// Based on the Pagerank implementation by Jure Leskovec https://www.youtube.com/watch?v=E9aoTVmQvok
// Added comments using chatGPT

#include <bits/stdc++.h>
#include <chrono>
#include <omp.h>
using namespace std;

typedef struct
{
    int outd; // Number of outgoing edges (out-degree)
    int *to;  // Array of nodes this node points to
} Node;

int N, threads;
double d, threshold;
double *ri, *rj;
vector<double> times;
Node *Nodes;

// Function to read the graph from a file
void Read_Graph(char *filename, int mode)
{
    ifstream file(filename);

    // Reading edge list from the file (node1 -> node2)
    // For reading the edgelist format
    if (mode == 0)
    {
        int node1, node2, size;
        string weight;
        while (file >> node1 >> node2 >> weight)
        {
            Nodes[node1].outd++;
            size = Nodes[node1].outd;
            Nodes[node1].to = (int *)realloc(Nodes[node1].to, size * sizeof(int));
            Nodes[node1].to[size - 1] = node2;
        }
    }
    // For reading the txt format
    else if (mode == 1)
    {
        string line;
        while (getline(file, line))
        {
            if (line.empty() || line[0] == '#')
                continue;

            int node1, node2, size;
            istringstream iss(line);
            if (iss >> node1 >> node2)
            {
                Nodes[node1].outd++;
                size = Nodes[node1].outd;
                Nodes[node1].to = (int *)realloc(Nodes[node1].to, size * sizeof(int));
                Nodes[node1].to[size - 1] = node2;
            }
        }
    }

    file.close();

    // Initialize the rank vectors
    ri = (double *)malloc(N * sizeof(double));
    rj = (double *)malloc(N * sizeof(double));

    // Initialize rank vectors: ri starts as all zeros, rj is 1/N for all nodes
#pragma omp parallel for num_threads(threads)
    for (int i = 0; i < N; i++)
    {
        ri[i] = 0;
        rj[i] = 1.0 / N;
    }
}

// Function to calculate the difference between ri and rj (for convergence check)
double diff()
{
    double sum = 0;

    // Parallel reduction to compute the total difference between ri and rj
#pragma omp parallel for num_threads(threads) reduction(+ : sum)
    for (int i = 0; i < N; i++)
    {
        sum += abs(ri[i] - rj[i]);
    }
    return sum;
}

int main(int argc, char **argv)
{
    const char *filename = "./soc-Stanford.txt"; // Input file name
    N = 281903;           // Number of nodes
    threshold = 0.0001;    // Convergence threshold
    d = 0.85;              // Damping factor for PageRank
    threads = 1;           // Default number of threads
    int mode = 1;          // Mode for reading input graph (0 = edgelist, 1 = txt)

    // Override default parameters with command-line arguments
    if (argc >= 2)
        filename = argv[1];
    if (argc >= 3)
        N = atoi(argv[2]);
    if (argc >= 4)
        threshold = atof(argv[3]);
    if (argc >= 5)
        d = atof(argv[4]);
    if (argc >= 6)
        threads = atoi(argv[5]);
    if (argc >= 7)
        mode = atoi(argv[6]);

    printf("Filename = %s, Threshold = %f, Damping Factor = %f, Threads = %d\n", filename, threshold, d, threads);

    // Allocate memory for the graph nodes
    Nodes = new Node[N];

    // Initialize the node data
#pragma omp parallel for num_threads(threads)
    for (int i = 0; i < N; i++)
    {
        Nodes[i].outd = 0;
        Nodes[i].to = new int[1];
    }

    // Read the graph from the input file
    Read_Graph((char *)filename, mode);

    int iterations = 0;
    double error = 1;  // Initialize error for convergence loop

    // Loop until the error is below the threshold
    while (error > threshold)
    {
        auto start = chrono::high_resolution_clock::now();  // Start timer for iteration

        // Swap ri and rj to start the next iteration
        swap(ri, rj);
        fill(rj, rj + N, 0.0);  // Reset rj for new values

        // PageRank formula:
        // rj[node] += d * ri[i] / Nodes[i].outd
        // where:
        // - d is the damping factor
        // - ri[i] is the rank of node i in the previous iteration
        // - Nodes[i].outd is the out-degree of node i
        // This computes the portion of node i's rank distributed to node j

#pragma omp parallel for num_threads(threads) schedule(dynamic)
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < Nodes[i].outd; j++)
            {
                int node = Nodes[i].to[j];  // Get the node to which i points
#pragma omp atomic
                rj[node] += d * ri[i] / Nodes[i].outd;
            }
        }

        // Handling dangling nodes and teleportation effect
        // s represents the leftover rank that should be evenly distributed among all nodes
        double s = 1.0;

#pragma omp parallel for num_threads(threads) reduction(- : s)
        for (int i = 0; i < N; i++)
        {
            s -= rj[i];
        }
        s /= N;  // Distribute the leftover rank evenly across all nodes

#pragma omp parallel for num_threads(threads)
        for (int i = 0; i < N; i++)
        {
            rj[i] += s;  // Adjust rj values with the leftover rank
        }

        // Compute the error for this iteration (difference between ri and rj)
        error = diff();
        iterations++;

        auto end = chrono::high_resolution_clock::now();  // End timer for iteration
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
        times.push_back(duration.count() / 1e6);  // Store the time taken for this iteration

        printf("Iteration %d, Error = %f, Time = %f\n", iterations, error, times.back());
    }

    // Calculate total time spent on all iterations
    double total_time = accumulate(times.begin(), times.end(), 0.0);
    printf("Total Time = %f\n", total_time);

    // Free allocated memory
    for (int i = 0; i < N; i++)
    {
        delete[] Nodes[i].to;
    }
    delete[] Nodes;
    free(ri);
    free(rj);

    return 0;
}
