This program creates a random array of 1,000,000 floats in the range [0,1]. It then calculates the sum of all those random floats, first using serial exeuciton and then with parallel execution with N number of threads. N is provided as an argument when starting the program.

To compile:
* Run command: make

Running:
* Usage: ./q6 <num_threads>

Examples:
* ./q6 4
* ./q6 8
* ./q6 12
* ./q6 16