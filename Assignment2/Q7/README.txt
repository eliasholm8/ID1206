This program creates a random array of M (array_size) doubles in the range [0,1]. It then calculates the sum of all those random doubles, first using serial execution and then with parallel execution with N (num_threads) number of threads. N is provided as the first argument when starting the program, and M is provided as the second argument. The program also creates a histogram of the array data. For the serial execution, the histogram is created serially. For the parallel execution, smaller histograms are created for each thread, which then are merged in the main thread.

To compile:
* Run command: make

Running:
* Usage: ./q7 <num_threads> <array_size>

Examples:
* ./q7 4 1000
* ./q7 8 10000
* ./q7 12 100000
* ./q7 16 1000000