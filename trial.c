#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

void send_appropriate_offsets(int pipes_arrays[][2], int size_of_array, int number_of_processors)
{
	//A function that will send a child process the appropriate start and stop values it should
	//be using when summing up the array.
	
	//I will be assuming that all the numbers I have given will be able to be exactly divisible
	int size_of_chunk = size_of_array/number_of_processors;
	int startIndex=0;
	int i;
	for (i=0; i<number_of_processors; i++)
	{
		//I'm going to write the start value and then the stop value we should be thinking about
		// printf("The start point is: %d\n", startIndex);
		write(pipes_arrays[i][1], startIndex, sizeof(startIndex));
		
		//This may be a bit confusing but here we are just spending the ending index to the child.
		startIndex+=size_of_chunk;
		write(pipes_arrays[i][1], startIndex, sizeof(startIndex));
		// printf("The end point is: %d\n", startIndex);
	}
}

void determine_appropriate_offsets(int pipes_arrays[][2], int array_from_file[], int size_of_array, int number_of_processors)
{
	//A function that send appropriate offsets to the send portion of array function,
	//so that it can forward along appropriate slices of the array.
	
	//I think this will be a proxy to the function that will actually do the work here
	//I think I'll name that function send_portion_of_array??
	
	//I will be assuming that all the numbers I have given will be able to be exactly divisible
	int size_of_chunk = size_of_array/number_of_processors;
	int startIndex=0;
	int i;
	for (i=0; i<number_of_processors; i++)
	{
		send_portion_of_array(pipes_arrays[i][1], array_from_file, startIndex, (startIndex+size_of_chunk));
		startIndex+=size_of_chunk;
	}
	
}

void send_portion_of_array(int send_pipe, int *array_from_file, int beginning, int ending)
{
	//A function that send a specific portion of an array down a specific pipe
	int i;
	for ( i = beginning; i < ending; ++i)
	{
		write(send_pipe, array_from_file[i], sizeof(array_from_file[i]));
	}
} 

int sum_from_pipes_child(int read_pipe)
{
	//A function that reads in all the values the parent would send down to it and sums them
	int sum = 0;
	char value[20];
	
	while (1)
	{
		bzero(value, sizeof(value));
		read(read_pipe, value, sizeof(value));
		
		if (strcmp (value, "Exit")==0)
		{
			break;
		}
		else
		{
			sum += atoi(value);
		}
	}
	
	return sum;
}


int sum_portion_of_array(int arr[], int lowerbound, int upperBound)
{
	//This will be used by children to sum up a portion of their array?
	int sum = 0;
	int i;

	for ( i = lowerbound; i < upperBound; ++i)
	{
	  sum += arr[i];
	}

	return sum;
}


int sum_all_children(int pipes_arrays[][2], int number_of_processes)
{
	char value[10];
	int i;
	int returned=0;
	for (i=0; i<number_of_processes; i++)
	{
		read(pipes_arrays[i][0], value, sizeof(value));
		
		int value_from_pipe=atoi(value);
		returned += value_from_pipe;
		//Debugging
		printf("Child process %d, got %d as its sum\n", i, value_from_pipe);
	}
	
	return returned;
}

void create_children(int pipes_arrays[][2], int array_from_file[], int size_of_array, int number_of_processes)
{
	int i;
	int pid[number_of_processes];
	for (i = 0; i < number_of_processes; i++)  
	{
		if ((pid[i] = fork()) < 0) 
		{
			//Error
			perror("fork");
			exit(EXIT_FAILURE);
		}
	   else if (pid[i] == 0) 
	   {
		   //Child Process
		   int sum = sum_from_pipes_child(pipes_arrays[i][0]);
		   //write(send_pipe, array_from_file[i], sizeof(array_from_file[i]));
		   write(pipes_arrays[i][1], sum, sizeof(sum));
		   
		   printf("Child %i, will be exiting.\n", pid[i]);
		   exit(1);
	   }
	   else
	   {
		   //Parent Process
		   
	   }
	}
	
	//I'm pretty sure only the parent process would get here.....
	determine_appropriate_offsets(pipes_arrays, array_from_file, size_of_array, number_of_processes);
	int sum = sum_all_children(pipes_arrays, number_of_processes);
	printf("The sum of all the numbers is: %d\n", sum);
}

void close_pipes(int pipes_array[][2], int number_of_processes)
{
	int i = 0;
	for(i; i<number_of_processes; i++)
	{
		close(pipes_array[i][0]);
		close(pipes_array[i][1]);
	}
}

void create_pipes(int pipes_array[][2], int number_of_processes)
{
	int i=0;
	for (i; i<number_of_processes; i++)
	{
		int fd[2];
		pipe(fd);
		pipes_array[i][0]=fd[0]; //Hopefully this works?
		pipes_array[i][1]=fd[1]; //Hopefully this works?
	}
}

int main(int argc, char *argv[])
{	
	if (argc < 3)
	{
		printf("Usage: program_name number_of_processes file_name\n");
	}
	
	int array_size;
	if (strcmp (argv[2], "medium.txt")==0)
	{
		array_size=1000000;
	}
	else if (strcmp (argv[2], "small.txt")==0)
	{
		array_size=1000;
	}
	else
	{
		//Assuming tiny
		array_size=10;
	}
	
	//Start timer
	create_children(); //
	//End timer
	
	
	// int myArray[1][1];
	// send_appropriate_offsets(myArray, 100000, 8);
}
//Main program
/*
int main() {
   
    //Read integers to be added from file
    FILE *myFile;
	int fileSize;
	
	char k[10];
    printf("\nEnter the number of the text file you want to read: ");
    gets(k);
	
	if (strcmp (k, "medium.txt")==0)
	{
		myFile = fopen("medium.txt", "r");
	}
	
    myFile = fopen("medium.txt", "r");

    //read file into array
    int arrInt[100000];
    int i;
    
    //1000 -small
    //10000 - medium
    //100000 - large
    for (i = 0; i < 100000; i++)
    {
        fscanf(myFile, "%1d", &arrInt[i]);
    }
    
    //Start timer for addition
   struct timeval  t1, t2;
   gettimeofday(&t1,NULL);
   
   int fd[2];
 
   //On error
   if (pipe(fd) < 0) {
     perror("pipe");
     exit(EXIT_FAILURE);
   }
   
   //No of Processes
   int noOfChildren = getNoOfChildren();//getNoOfChildren(); //1,2,4
   int pid[noOfChildren];
   
   //For each child process 
   int r;
   for (r = 0; r < noOfChildren; ++r)  
   {
        if ((pid[r] = fork()) < 0) 
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
       else if (pid[r] == 0) 
       { 
           //Child process 
            int sum = 0;
            //Close the unread end of pipe
            close(fd[0]); 
            int n;
            for (n = 0; n < 1000000; n+=noOfChildren)
              sum += Add(arrInt, sizeof(arrInt) / sizeof(*arrInt));

           // Till the sum is written
            int q;
            for (q = 0; q != sizeof(sum); ) {
              int m = write(fd[1], &sum + q, sizeof(sum) - q);
              if (m != -1)
                q += m;
              
              else if (errno != EINTR) 
              { 
                perror("write");
                _exit(EXIT_FAILURE);
              }
            }
            //Close pipe
            close(fd[1]); 
            //Exit child process
            _exit(EXIT_SUCCESS); 
       }
   }
   
   //Close the unused end of the pipe
   close(fd[1]); 
 
   // Parent Process
   int exit_status = EXIT_SUCCESS;
   { 
     //All child sums
     FILE *fp = fdopen(fd[0], "rb");
     
     int childSums[noOfChildren];
     
     //Output sums
     if (fp && fread(childSums, sizeof(*childSums), noOfChildren, fp) == noOfChildren) 
     {
         gettimeofday(&t2,NULL);
         double t = (t2.tv_sec - t1.tv_sec);
         int s = Add(childSums, noOfChildren);  
       printf("\nSum: %d \n",s);
       printf("\nTime %d s\n",t);
     }
     else {
       fputs("error\n",stderr);
       exit_status = EXIT_FAILURE;
     }
     if (fp)
       fclose(fp);
 
     // Child process status
     int r;
     for ( r = 0; r < noOfChildren; ) 
     {
       int j = 0;
       
       if (waitpid(pid[r], &j, 0) == -1) 
       {
         if (errno != EINTR) {  //exit
           perror("waitpid");
           exit_status = EXIT_FAILURE;
         }
         else 
           continue;    //continue with child
       }
       ++r;
     }
   }
   exit(exit_status);
}
*/