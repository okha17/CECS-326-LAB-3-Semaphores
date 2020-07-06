#include <stdio.h> // For output functions
#include <unistd.h> // Access POSIX OS API
#include <stdlib.h> // Used for srand and exit
#include <sys/shm.h> // Library for shared memory functions
#include <sys/ipc.h> // Header file for Interprocess communication
#include <sys/types.h> //Header file for pid_t data type 
#include <signal.h> // Header file used for signals
#include <sys/wait.h> // Header file used for wait function 
#include <semaphore.h> // Header file to use semaphores for POSIX
#include <fcntl.h> // Header file for using O constants

void Interrupt(); //  Function that interrupts the process 
void Terminate(); // Function that terminates the process

pid_t fish, pellet; // Used for forking for fish and pellet process
char (*mill)[10][10];  // char pointer to 2D array used as shared memory 
int shm_id; // int value the stores shared memory ID

sem_t *sem_mill; //Semaphore for synchronization 

int main(int argc, char *argv[])
{
	signal(SIGINT, Interrupt); //Signal for interrupt caught and calls Interrupt function
	signal(SIGTERM, SIG_IGN); // Ignore the terminate signal for swim mill
	// Create a named semaphore with read and write permissions
	// Value of semaphore initially 1
	sem_mill = sem_open("/sem_mill", O_CREAT, 0666, 1);
	
	// Check if semaphore failed to create
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
	
	// Set 2D array to tilde value to display empty swim mill 
	for(int i = 0; i < 10; i++)
	{
		for(int j = 0; j < 10; j++)
		{
			*mill[i][j] = '~'; // tilde represents the swim mill
		}
	}	
	
	// Fork process 
	fish = fork();
	
	// Error occurred 
	if(fish == -1)
	{
		perror("Fork failed!\n");
		exit(1);
	}
	
	//Child process created
	else if(fish == 0)
	{
		// Replace core image with Fish
		execv("Fish", argv);
	}
	
	// Parent Process
	else	
	{	
		// Parent process 
		// Loop for 30 iterations and sleep for 30 second run time
		for(int i = 0; i < 30; i++)
		{
			pellet = fork();
			//Error occurred
			if(pellet == -1)
			{
				perror("Fork failed!\n");
				exit(1);
			}
			//Child process created
			else if(pellet == 0)
			{
				// Replace core image with Pellet
				execv("Pellet", argv);
				
			}
			// Parent process 
			// Display 2D array to show movements
			// Added semaphore 
			sem_wait(sem_mill); // Wait to display the swim mill
			printf("\n");
			for(int i = 0; i < 10; i++)
			{
				for(int j = 0; j < 10; j++)
				{
					printf("%c", *mill[i][j]);
				}
			printf("\n");
			}
			sem_post(sem_mill); // Increment the semaphore 
			sleep(1); //Sleep in order to run display for 30 seconds
		}	
	}
	Terminate(); // call termination function to end program
	return 0;
}

void Interrupt()
{
	printf("\nInterupt on SwimMill!\n");
	kill(fish, SIGINT); // Kill the child process
	kill(pellet, SIGINT); // Kill the child process
	shmdt(mill); // Detach from shared memory
	shmctl(shm_id, IPC_RMID, 0); // Delete the shared memory
	sem_close(sem_mill); // Close semaphore to free semaphore
	sem_unlink("sem_mill"); // removed named semaphore
	sleep(3); // make sure all pellets are terminated before swim mill exits
	exit(0); // Exit the program
}

void Terminate()
{
	kill(0, SIGTERM); // Kill all child processes
	while(wait(NULL) > 0); // Wait for child process to exit before moving to next line
	shmdt(mill); // Detach from shared memory
	shmctl(shm_id, IPC_RMID, 0);  // Delete the shared memory
	sem_close(sem_mill); // Close semaphore to free semaphore
	sem_unlink("sem_mill"); // removed named semaphore
	printf("\nTermination of SwimMill!\n");
	sleep(3); // make sure all pellets are terminated before swim mill exits
	exit(0); // Exit the program
}

