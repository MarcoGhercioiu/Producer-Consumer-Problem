Problem Statement
 The assigned task is to write a program in C/C++ that simulates the Producer/Consumer
problem with a buffer size of one thousand. The program needs to perform a normal exit
process after all items are consumed. The producer and consumers are running as separate
processes generated with a fork. The importance of the project is to learn more about how to
manage separate processes running at the same time with POSIX semaphores. Essentially, I will
be writing a program to manage a producer producing items in a buffer and consumers
consuming items from the buffer with semaphores.

Approach
 The approach I took for this program was to utilize POSIX semaphores to send signals
between processes. The semaphore functions I used are:
 Sem_open(), creates a new POSIX semaphore or opens an existing semaphore
 Sem_wait(), decrements the semaphore, I used this semaphore to lock processes
 Sem_post(), increments the semaphore, I used this semaphore to unlock processes
To share the buffer between processes I first opened a POSIX shared memory object with
the function shm_open(). Then I used the function ftruncate() to truncate the size of the
memory object to the length of 1000 integer values in bytes. Finally, I used mmap() to map the
virtual address space for the buffer.
To handle the producer/consumer problem with multi-process synchronization I used
semaphores to ensure mutual exclusion. I implemented 4 semaphores:
 p – used to signal that the producer is ready to produce, initialized to 1
 c – used to signal that the consumer is ready to consume, initialized to 0
 b – used to count the total number of empty elements in the buffer, when this
semaphore is 0 the buffer is filled with 1000 integers, the total size of the buffer
 te – used to terminate the program, this semaphore is initialized to 1000000 and
decrements when a consumer consumes an item, when this semaphore value is 0 the
program is finished because all 1000000 elements have been consumed
My producer function calls sem_wait() on my buffer semaphore and my producer
semaphore. Then the function finds the first available location for a new element in the buffer.
Then it calls the produce function which returns a random number in the range of 1 to 100
through the use of the rand() function. The number produced is then placed in the buffer at the
memory location previously found. Finally, the producer function calls sem_post() on my
producer and consumer semaphore. The producer process runs before my consumer processes
because my consumer ready semaphore is initialized to 0 so my consumers are waiting for the
producer() function to post the consumer ready semaphore.
My consumer function calls sem_wait() on my consumer and producer ready
semaphore. Then, in the critical section the consumer processes execute the consume()
function one at a time. My consume() function traverses the buffer from its first element
memory location till it reaches an element to consume or reaches the end of the buffer. If the
consume() function reaches an element to consume the element is taken out of the buffer and
returned to the consumer() function. If the buffer is empty the consume() function will return a
0 to the consumer() function. If the consumer() function receives an element that was
consumed then it adds that number to its processes consumer buffer which holds all the
elements a specific consumer has consumed. Finally, the consumer() function calls sem_post()
on the producer ready and buffer ready semaphores and the process sleeps for 5 microseconds.
It is important to consume items in the critical section because we only want one
consumer process to access the buffer at one time.
Solution
The assigned task was to write a program that simulates the Producer/Consumer
problem with a buffer size of one thousand and includes one producer and ten consumers. To
implement a solution, I used C++. I also used POSIX semaphores and shared memory.
Figure 1 shows that I used the g++ compiler on my Ubuntu 20.04 LTS environment to
compile my program.

As seen in figure 1, I compiled my program with -pthread and -lrt.
 -pthread tells the compiler to link the pthread library, without including this link my
program will not compile and print out errors for semaphore functions
 -lrt tells the compiler to link the Realtime Extensions library, without including this link
my program will not compile and print out errors for shared memory functions
Figure 2 shows the execution of my program. The program takes about 15 seconds for the
producer and consumers to finish processing 1000000 items. Then the program outputs the
total number of consumed elements by each consumer, signified by their PID.

Figure 3 shows multiple executions to indicate that the amount consumed by each
consumer changes slightly, but each run of the program has a total of 1000000 elements
consumed.

A problem that I ran into while trying to get my semaphores to change across processes
was, I thought that since my semaphores were created in main before the processes were
forked it would automatically share the semaphores across the processes. In actuality, the
processes had their own copy of semaphores and were not changing semaphore values in
shared memory. To fix this problem I opened the same shared memory semaphores in my
producer() and consumer() functions to adjust the semaphores across processes.
Another problem that I ran into was creating a shared buffer for each process to read
and write to. To solve this problem I used the shm_open() function to open a POSIX shared
memory object. Then I used the function ftruncate() to truncate the size of the memory object
to the length of 1000 integer values in bytes. Finally, I used mmap() to map the virtual address
space for the buffer.
To use the POSIX semaphore functions and shared memory I consulted the Linux manual
pages to reference the format of the functions.
