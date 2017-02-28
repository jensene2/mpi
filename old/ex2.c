 // required MPI include file  
   #include "mpi.h"
  #include <stdlib.h>
   #include <stdio.h>

   int main(int argc, char *argv[]) 
	{
	char hostname[MPI_MAX_PROCESSOR_NAME];
   	// Find out rank, size
	int  numtasks, len, rc; 
	
	int world_rank;
	   // initialize MPI  
   	MPI_Init(&argc,&argv);
	MPI_Get_processor_name(hostname, &len);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	int init;
	int number;
	int * array;
	if (world_rank == 0) 
		{
			printf("Enter the number:");
			scanf("%d",&number);
			init = 1;
			for(int i= 1; i < world_size; i++)
			{
    				MPI_Send(&number, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			}
			array = malloc(sizeof(int) * number * number);
			for(int i= 1; i < world_size; i++)
                        {
				MPI_Recv(array,number*number, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				char del = ' ';
				for(int i = 0; i < number*number; i++)
				{
					printf("%c%d", del, array[i]);
					del = ',';
				}
				puts("");
			}
	

		} 
	else if (world_rank != 0) 
		{
    			MPI_Recv(&number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			array = malloc(sizeof(int) * number * number);
			array[0] = world_rank -1;
			for(int i = 1; i < number * number;i++)
			{
				array[i] = 0;
			}
			//an algorithm goes here!!

			MPI_Send(array, number * number, MPI_INT, 0, 0, MPI_COMM_WORLD);
		}
	printf ("Number of tasks= %d My rank= %d Running on %s\n", world_size,world_rank,hostname);
        
   // done with MPI  
   MPI_Finalize();
   }

int getIndex(int row, int col, int size)
{
	int result = (row * size)+ col;
	return result;
}	


