# Readme for st_pipeline

This is a C++ program called `st_pipeline` that implements a pipeline of active objects to perform a series of operations on generated numbers. The program takes one or two arguments: N (the number of tasks) and an optional random seed.

## Prerequisites

Before running the `st_pipeline` program, ensure that you have the following software installed on your system:

- GCC (GNU Compiler Collection)
- Linux or Unix-like operating system (e.g., Ubuntu 22.04)

## Building the Program

To build the `st_pipeline` program, follow these steps:

1. Open a terminal and navigate to the project directory.
2. Compile the source files using the provided Makefile by running the following command: `make all`

This command will compile the necessary source files and generate the executable for the program.

## Running the Program

To run the `st_pipeline` program, follow these steps:

1. In the terminal, navigate to the project directory (if not already there).
2. Start the program by running the following command: `./st_pipeline N [seed]`

Replace `N` with the desired number of tasks.
If you provide a `seed` as the second argument, it will be used as the random seed. If no random seed is provided, the program will generate one using the current time.

The program will perform the following steps:

1. Create four active objects (AO) with their respective queues.
2. Generate N numbers, each with 6 digits, using the random generator.
3. Enqueue tasks in the first AO's queue, which will process the numbers and transfer them to the subsequent AOs.
4. Each AO will print the received number, check if it is prime, and perform specific operations on the number as described in the task.
5. The program will stop all active objects and wait for their threads to finish.

## Conclusion

The `st_pipeline` program implements a pipeline of active objects to process generated numbers and perform various operations on them. Each active object has its specific tasks and transfers the processed numbers to the next active object in the pipeline. The program provides the flexibility to specify the number of tasks and an optional random seed.

If you have any questions or issues, please don't hesitate to reach out.

Enjoy using `st_pipeline`!
