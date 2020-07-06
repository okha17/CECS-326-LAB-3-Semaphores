#include <stdio.h> // For output functions
#include <unistd.h> // Access POSIX OS API
#include <stdlib.h> // Used for srand and exit
#include <sys/shm.h> // Library for shared memory functions
#include <sys/ipc.h> // Header file for Interprocess communication
#include <sys/types.h> //Header file for pid_t data type 
#include <signal.h> // Header file used for signals
#include <time.h> // Used for srand 
#include <semaphore.h> // Header file to use semaphores for POSIX
#include <fcntl.h> // Header file for using O constants

void pellet_move(char(*)[10][10]); // Function that moves pellets and handles termination 
void Interrupt(); //  Function that interrupts the process 
void Terminate(); // Function that terminates the process

char (*mill)[10][10];  // char pointer to 2D array used as shared memory 
int shm_id; // int value the stores shared memory ID
sem_t *sem_mill; //Semaphore for synchronization 

int main(int argc, char *argv[])
{
	signal(SIGINT, Interrupt); //Signal for interrupt caught and calls Interrupt function
	signal(SIGTERM, Terminate); //Signal for termination and calls Terminate function
	
	sem_mill = sem_open("/sem_mill", 0); // Open already created semaphore
	
	// Check if semaphore failed 
	if(sem_mill == SEM_FAILED)
	{
		printf("Semaphore failed!/n");
		exit(1);
	}
	
	// Generate unique key used for shared memory
	// Pass in the SwimMill file and a random char
	key_t key = ftok("SwimMill.c", 'a');
	// Shmget is used to return identifier for shared memory
	// Unique key, size of 2D array, and IPC_CREAT used to 
	// create new memory segment for shared memory while 
	// 0666 is sets access permission for shmflag parameter  
	shm_id = shmget(key, sizeof(char[10][10]), IPC_CREAT|0666);
	
	// Check to make sure shared ID is valid
	if(shm_id < 0)
	{
		perror("Failed to generate memory ID\n");
		exit(1);
	} 
	
	// Attach shared memory before you can use the segment 
	// Pass in shared memory ID 
	mill = (char(*)[10][10])shmat(shm_id, NULL, 0);
	
	// Check to make sure attach did not fail 
	if((mill = (char(*)[10][10])shmat(shm_id, NULL, 0))  < (char(*)[10][10]) 0)
	{
		perror("Failed to attach memory!\n");
		exit(1);
	}

	pellet_move(mill); 
	return 0;
}

// Added semaphores to help with synchronization for shared memory access
void pellet_move(char (* stream)[10][10])
{
	// File pointer to write to output text document
	FILE *filepointer = fopen("output.txt", "w");
	// In case the filepointer fails and file doesn't exist
	if(filepointer == NULL)
	{
		FILE *filepointer = fopen("output.txt", "w"); // Creates file in current folder
	}

	// Generate unique random coordinates for each pellet
	srand(getpid());
	// Set to random values between 0-9
	int row = rand() % 10;
	int col = rand() % 10;
	// Check if spawns on fish
	if (*stream[row][col] == 'F') 
	{
		printf("\nPID of Pellet:%d\n", getpid());
		printf("Coordinates:(%d,%d)\n", row, col);
		printf("Pellet was eaten!\n");
		fprintf(filepointer,"\nProcess ID of Pellet: %d\n", getpid());
		fprintf(filepointer,"Coordinates of Pellet:(%d,%d)\n", row, col);
		fprintf(filepointer,"Pellet was eaten!\n");	
		*stream[row][col] = 'F'; // Precaution to make sure fish is at current location
	}
	// If it didn't spawn on the fish
	else
	{
		// Random position set to P for pellet 
		*stream[row][col] = 'P';
		// Assigns pellet position to ~ as it moves downstream
		// Following row value is set to P 
		// Sleep after each movement to have 1 sec refresh rate
		// Stops before row 8 to do a check on following row
		// Added semaphores for controlling movement after each increment
		while (row < 8)
		{
			// Semaphores used for pellet movement
			if(*stream[row][col] == 'P')
			{
				sem_wait(sem_mill); // Wait if fish is moving
				*stream[row][col] = '~'; // assign coordinate to tilde
				row++;
				*stream[row][col] = 'P'; // move the pellet down one column
				sem_post(sem_mill); // Increments after moving down
				sleep(1); // Sleep to have a 1 sec refresh rate
			}
		}
		
		// If pellet spawned in the bottom row and isn't on same coordinate as fish 
		if(row == 9 && *stream[row][col] == 'P')
		{
			sem_wait(sem_mill); // Wait if fish is moving
			if(*stream[row][col] != 'F')
			{
				printf("\nPID of Pellet:%d\n", getpid());
				printf("Coordinates of Pellet:(%d,%d)\n", row, col);
				printf("Pellet was missed!\n");
				fprintf(filepointer,"\nProcess ID of Pellet: %d\n", getpid());
				fprintf(filepointer,"Coordinates of Pellet:(%d,%d)\n", row, col);
				fprintf(filepointer,"Pellet was missed!\n");	
				*stream[row][col] = '~'; // Pellet is now removed from the swim mill
			}
			sem_post(sem_mill); // Increments after pellet has missed/eaten
		}
		
		// If pellet didn't spawn on the final row and must move one more row down 
		else if(row == 8 && *stream[row][col] == 'P')
		{
			sem_wait(sem_mill); // Wait if fish is moving
			// Assigns current row value to ~ and checks following row for match
			*stream[row][col] = '~';
			row++;
			// If the pellet will be in the same spot as the fish
			// It is eaten by the fish 
			// Prints out pellet process information
			if(*stream[row][col] == 'F')
			{
				printf("\nPID of Pellet:%d\n", getpid());
				printf("Coordinates of Pellet:(%d,%d)\n", row, col);
				printf("Pellet was eaten!\n");
				fprintf(filepointer,"\nProcess ID of Pellet: %d\n", getpid());
				fprintf(filepointer,"Coordinates of Pellet:(%d,%d)\n", row, col);
				fprintf(filepointer,"Pellet was eaten!\n");	
				*stream[row][col] = 'F'; // Set the coordinate to F for the fish
				sem_post(sem_mill); // Increments after pellet has missed/eaten
			}
			// If the pellet missed the fish it prints out as a miss 
			// and is terminated as it is replaced with a ~ value 
			else if(*stream[row][col] != 'F')
			{
				*stream[row][col] = 'P'; // Display pellet on bottom row if missed
				sem_post(sem_mill); // Increments after pellet has missed/eaten
				sleep(1); //Sleep to have 1 sec refresh rate
				
				if(*stream[row][col] != 'F')
				{
					sem_wait(sem_mill); // To make sure output doesn't intersect with shared memory
					printf("\nPID of Pellet:%d\n", getpid());
					printf("Coordinates of Pellet:(%d,%d)\n", row, col);
					printf("Pellet was missed!\n");
					fprintf(filepointer,"\nProcess ID of Pellet: %d\n", getpid());
					fprintf(filepointer,"Coordinates of Pellet:(%d,%d)\n", row, col);
					fprintf(filepointer,"Pellet was missed!\n");	
					*stream[row][col] = '~'; // Pellet is now removed from swim mill
					sem_post(sem_mill); // Increment semaphore
				}
			}
		}
	}
	fclose(filepointer); // Close the file
}

void Interrupt()
{
	printf("Interrupt on Pellet!\n");
	sem_close(sem_mill); // Close semaphore to free semaphore
	shmdt(mill); // Detach from shared memory
	exit(0); // Exit the process
}

void Terminate()
{
	printf("Terminate Pellet!\n");
	sem_close(sem_mill); // Close semaphore to free semaphore
	shmdt(mill); // Detach from shared memory
	exit(0); // Exit the process
}


