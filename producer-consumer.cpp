/*
 File: Main.cpp
 Author: Vlad
 Procedures:
 main		- opens semaphores for cross process communication, then creates one producer process and ten consumer processes,
 	 	 	 then closes all processes and semaphores
 produce	- returns a random number between one and one hundred
 producer 	- opens shared memory one thousand integer buffer, waits for signal to start, produces items in buffer, sends
 	 	 	 ready signal to consumers
 consume	- iterates through buffer and returns first available element or zero if buffer is empty
 consumer	- opens shared memory one thousand integer buffer, waits for signal to start, consumes a item in buffer, sends
 	 	 	 ready signal to producer
 */

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>
// Semaphore name constants
#define SEM_NAME "/producer_ready"
#define SEM_NAME2 "/consumer_ready"
#define SEM_NAME3 "/buffer_ready"
#define SEM_NAME4 "/total_elements"
// Buffer name constant
#define BUFFER_NAME "/buffer"
// define size of buffer
#define elementsbuffer 1000
// define the total elements to produce
#define total 1000000
// define the size of memory that buffer takes up
#define sizeOfArray 4000

/*
 int produce()
 Author: Vlad
 Date: 10/1/2020
 Description: return random number 1-100

 Parameters:
 produce		O/P		int		Status code
 */
int produce() {
	return (rand() % 100) + 1;
}

/*
 void producer()
 Author: Vlad
 Date: 10/1/2020
 Description: fill buffer with numbers for consumers to consume

 Parameters:
 fd				I/P		int			file descriptor for POSIX shared memory object
 producer		O/P		void		Status code
 */
void producer(int fd) {
	// create a new mapping in the vertual address for the buffer
	int *buffer = (int*) mmap(0, sizeOfArray, PROT_WRITE | PROT_READ,
			MAP_SHARED, fd, 0);
	// initilize the buffer elements to zero (meaning empty)
	for (int i = 0; i < elementsbuffer; i++) {
		buffer[i] = 0;
	}
	// open semaphores within process
	sem_t *p = sem_open(SEM_NAME, O_RDWR);
	sem_t *c = sem_open(SEM_NAME2, O_RDWR);
	sem_t *b = sem_open(SEM_NAME3, O_RDWR);
	sem_t *te = sem_open(SEM_NAME4, O_RDWR);

	// seed rand for random number generation
	srand(time(NULL));

	printf("Please wait for consumers to consume items...\n");

	while (true) {
		// check semaphore signifying the end of program
		int endProg = 1;
		sem_getvalue(te, &endProg);
		if (endProg == 0) {
			break;
		}
		// decrements buffer empty item count semaphore
		sem_wait(b);
		// wait on producer to be ready
		sem_wait(p);
		// get index of first empty location of buffer
		int index = 0;
		while (index < elementsbuffer) {
			if (buffer[index] == 0) {
				break;
			} else {
				index++;
			}
		}
		// get and place data in first available location in buffer
		int data = produce();
		buffer[index] = data;
		// post signal that producer is done and consumer is ready
		sem_post(p);
		sem_post(c);

	}
	return;
}

/*
 int consume()
 Author: Vlad
 Date: 10/1/2020
 Description: return first available element in buffer or 0 if buffer is empty

 Parameters:
 buffer			I/P		int *	pointer to memory location of first element in buffer
 consume		O/P		int		Status code
 */
int consume(int *buffer) {
	// traverse buffer and find available data, then remove data from buffer and return it
	int data = 0;
	int index = -1;
	while (data == 0 && index < elementsbuffer) {
		index++;
		data = buffer[index];
	}
	if (data != 0) {
		buffer[index] = 0;
		return data;
	} else
		return 0;
}

/*
 void consumer()
 Author: Vlad
 Date: 10/1/2020
 Description: waits for producer to signal that the buffer is ready, then consumes an element of the buffer
 in the critical section, finally sends signal back to the producer to repopulate the buffer

 Parameters:
 fd				I/P		int			file descriptor for POSIX shared memory object
 consumer		O/P		void		Status code
 */
void consumer(int fd) {
	// open shared memory buffer
	int *buffer = (int*) mmap(0, sizeOfArray, PROT_WRITE | PROT_READ,
			MAP_SHARED, fd, 0);
	// count for number of items a consumer consumed
	int consumerindex = 0;
	// buffer for consumed items
	int consumerbuffer[200000];
	// open semaphores
	sem_t *p = sem_open(SEM_NAME, O_RDWR);
	sem_t *c = sem_open(SEM_NAME2, O_RDWR);
	sem_t *b = sem_open(SEM_NAME3, O_RDWR);
	sem_t *te = sem_open(SEM_NAME4, O_RDWR);

	while (true) {
		// check semaphore signifying the end of program
		int endProg;
		sem_getvalue(te, &endProg);
		if (endProg == 0) {
			sem_post(c);
			// print number of items consumer consumed
			printf("Consumer PID %d consumed %d items!\n", getpid(), consumerindex);
			return;
		}
		// wait for consumer to be ready
		sem_wait(c);
		sem_wait(p);
		// consume element from buffer
		int bufferValue = consume(buffer);
		if (bufferValue != 0) {
			consumerbuffer[consumerindex] = bufferValue;
			consumerindex++;
		// decrement semaphore value initialized to 1000000
			sem_trywait(te);
		}
		// post producer ready
		sem_post(p);
		sem_post(b);

		usleep(5);

	}

	return;

}

/*
 int main()
 Author: Vlad
 Date: 10/1/2020
 Description: generates four semaphores, then opens shared memory for buffer, then creates producer and consumer processes,
 finally it closes all outstanding semaphores and processes

 Parameters:
 main		O/P		int		Status code
 */
int main() {

	sem_unlink(SEM_NAME);
	sem_unlink(SEM_NAME2);
	sem_unlink(SEM_NAME3);
	sem_unlink(SEM_NAME4);
	// create semaphores for producer ready, consumer ready, buffer ready, and total elements
	sem_t *p = sem_open(SEM_NAME, O_CREAT, 0644, 1);
	sem_t *c = sem_open(SEM_NAME2, O_CREAT, 0644, 0);
	sem_t *b = sem_open(SEM_NAME3, O_CREAT, 0644, 1000);
	sem_t *te = sem_open(SEM_NAME4, O_CREAT | O_EXCL, 0644, total);

	if (te == SEM_FAILED) {
		printf("failed\n");
	}
	// open shared memory for buffer
	int fd = shm_open(BUFFER_NAME, O_CREAT | O_RDWR, 0666);
	// truncate size of buffer to 1000 integers
	ftruncate(fd, sizeOfArray);

	bool isParent = true;
	// create forked processes
	int pid[11];
	for (int i = 0; i < 11; i++) {
		int newProcessID = 0;
		newProcessID = fork();

		if (newProcessID == 0) {
			isParent = false;
			if (i == 0) {
				producer(fd);
				break;
			} else {
				consumer(fd);
				break;
			}
			break;
		} else {
			pid[i] = newProcessID;
			continue;
		}

	}
	// close processes
	if (isParent) {
		for (int i = 0; i < 11; i++) {
			if (waitpid(pid[i], NULL, 0) < 0) {
				perror("waitpid(2) failed");
			}
		}
	}
	// unlink and close semaphores
	sem_unlink(SEM_NAME);
	sem_unlink(SEM_NAME2);
	sem_unlink(SEM_NAME3);
	sem_unlink(SEM_NAME4);
	shm_unlink(BUFFER_NAME);
	sem_close(p);
	sem_close(c);
	sem_close(b);
	sem_close(te);
	return 0;
}
