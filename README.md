# Description
The following repository contains solutions to the assignments for the course [CS347](https://www.cse.iitb.ac.in/~mythili/os/) at IIT Bombay.
Each asigment also contains a bash file for verifying/testing purposes.

# Assignment 1: Simple shell

1. Build a simple shell to run Linux built-in commands like ls, cd
2. Serial, parallel, and background execution: 
	a. If a command is followed by &, the command must be executed in the background.
	b. Multiple user commands separated by && should be executed one after the other in sequence in the foreground.
	c. Multiple commands separated by &&& should be executed in parallel in the foreground.
3. Signal handling and exit: When the user hits Ctrl+C, the shell must correctly handle the signal and not terminate itself. Instead, it should terminate the current foreground processes (the current command in serial execution, or all the commands in a parallel execution), and return to the command prompt. The background processes should remain unaffected by the SIGINT. When the shell receives the exit command, it must terminate all background processes, clean up any internal state (e.g., free dynamically allocated memory), and finally terminate.

# Assignment 2: Memory management

1. init() must initialise the manager.
2. cleanup() must return the memory to the system.
3. alloc(integer) takes a buffer size that must be allocated and should return a char* pointer to the buffer on sucess, NULL on failure.
4. dealloc(char\*) should free up the previously allocated chunk.

# Assignment 3: Worker Thread Pool

1. In this part, the program takes 3 command line arguments: how many numbers “produce” (M), the maximum size of the buffer in which the pro- duced numbers should be stored (N), and the number of worker threads to spawn to consume these numbers (W ). The skeleton code spawns a master thread, that produces the specified number of integers from 0 to M − 1 and exits. The main program waits for this thread to join and terminates.
2. Identical to part 1, except the shared data structure is a dynamically sized linked list, and not a fixed size array.