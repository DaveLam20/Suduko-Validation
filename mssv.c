#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define SIZE 9

int Sol[SIZE][SIZE];
int Row[SIZE] = {0};
int Col[SIZE] = {0};
int Sub[SIZE] = {0};
int Counter = 0;
int delay;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int threads_finished = 0;

void read_sudoku_solution(const char *solution){
	int i,j;
	FILE *file = fopen(solution, "r");
	if (file == NULL){
		perror("Failed to open file");
		exit(EXIT_FAILURE);
	}

	for(i = 0; i < SIZE; i++){
		for(j = 0; j < SIZE; j++){
			fscanf(file, "%d", &Sol[i][j]);
		}
	}
	fclose(file);
}

void *row_subgrids_validation(void *arg){
	int *args = (int *)arg;
	int start_row = args[0];
	int end_row = args[1];
	int start_subgrid = args[2];
	int end_subgrid = args[3];

	int thread_id = args[4];
	int valid = 1;

	int invalid_rows[SIZE] = {0};
	int invalid_subgrid[SIZE] = {0};
	
	for(int row = start_row; row <= end_row; row++){
		int values[SIZE + 1] = {0};
		int is_invalid = 0;

		for(int col = 0; col < SIZE; col++){
			int val = Sol[row][col];
			if (values[val]) {
				is_invalid = 1;
			}
			values[val] = 1;
		
		}
		
		if (is_invalid) {
			printf("Thread ID-%d: row %d is Invalid\n", thread_id, row + 1);
			invalid_rows[row] = 1;
			valid = 0;
		}
		else {
			pthread_mutex_lock(&mutex);
			Row[row] = 1;
			Counter++;
			pthread_mutex_unlock(&mutex);
		}

	}

	for(int sub = start_subgrid; sub <= end_subgrid; sub++){
		int values[SIZE + 1] = {0};
		int row_offset = (sub / 3) * 3;
		int col_offset = (sub % 3) * 3;
		int is_invalid = 0;

		for(int row = 0; row < 3; row++){
			for(int col = 0; col < 3; col++){
				int val = Sol[row_offset + row][col_offset + col];
				if(values[val]){
					is_invalid = 1;
				}
				values[val] = 1;
			}
		}
		if (is_invalid){
			printf("Thread ID-%d: Sub-grid %d is Invalid\n", thread_id, sub + 1);
			invalid_subgrid[sub] = 1;
			valid = 0;
		}
		else{
			pthread_mutex_lock(&mutex);
			Sub[sub] = 1;
			Counter++;
			pthread_mutex_unlock(&mutex);
		}
	}

	pthread_mutex_lock(&mutex);
	threads_finished++;
	if(threads_finished == 4){
		printf("Thread ID-%d is the last thread.\n", thread_id);
		pthread_cond_signal(&cond);
	}
	pthread_mutex_unlock(&mutex);

	if(valid){
		printf("Thread ID-%d is valid\n", thread_id);
	}
	free(args);
	return NULL;
}



void *validate_columns(void *arg) {
    	(void)arg;
	int invalid_columns[SIZE] = {0};
	int valid = 1; 
       
	for (int col = 0; col < SIZE; col++) {
		int values[SIZE + 1] = {0};
		int is_invalid = 0;
		
		for (int row = 0; row < SIZE; row++) {
	    		int val = Sol[row][col];
	    		if (values[val]) {
			    	is_invalid = 1;
	    		}
			values[val] = 1;
		}
	
		if (is_invalid) {
		    	printf("Thread ID-4: column %d is invalid\n", col + 1);
		     	invalid_columns[col] = 1;
			valid = 0;
		} 
		else {
	    		pthread_mutex_lock(&mutex);
	    		Col[col] = 1;
	    		Counter++;
	      		pthread_mutex_unlock(&mutex);
		}
    	}
 	if (valid) {
 		printf("Thread ID-4 is valid\n");
    	}   
 	pthread_mutex_lock(&mutex);
    	threads_finished++;
    
	if (threads_finished == 4) {
		printf("Thread ID-4 is the last thread.\n");
		pthread_cond_signal(&cond);
    	}
    	pthread_mutex_unlock(&mutex);
    

	return NULL;
}

int main(int argc, char *argv[]){
	if (argc < 3){
		printf("Usage: %s <solution_file> <delay>\n",argv[0]);
		return EXIT_FAILURE;

	}

	read_sudoku_solution(argv[1]);
	delay = atoi(argv[2]);

	pthread_t thread1, thread2, thread3, thread4;
	
	int *args1 = (int *)malloc(5 * sizeof(int));
    	args1[0] = 0; args1[1] = 2; args1[2] = 0; args1[3] = 2; args1[4] = 1;
    	pthread_create(&thread1, NULL, row_subgrids_validation, args1);
    	sleep(delay);
     
	int *args2 = (int *)malloc(5 * sizeof(int));
    	args2[0] = 3; args2[1] = 5; args2[2] = 3; args2[3] = 5; args2[4] = 2;
    	pthread_create(&thread2, NULL, row_subgrids_validation, args2);
    	sleep(delay);

    	int *args3 = (int *)malloc(5 * sizeof(int));
   	args3[0] = 6; args3[1] = 8; args3[2] = 6; args3[3] = 8; args3[4] = 3;
     	pthread_create(&thread3, NULL, row_subgrids_validation, args3);
     	sleep(delay);
       
	pthread_create(&thread4, NULL, validate_columns, NULL);

       
	pthread_mutex_lock(&mutex);
    	while (threads_finished < 4) {
	  	pthread_cond_wait(&cond, &mutex);
       	}
    	pthread_mutex_unlock(&mutex);
      
	printf("There are %d valid rows, columns, and sub-grids.\n", Counter);
    	if (Counter == 27) {
	 	printf("The solution is valid.\n");
    	} 
	else {
	       	printf("The solution is invalid.\n");
     	}
      
	pthread_mutex_destroy(&mutex);
    	pthread_cond_destroy(&cond);

	return 0;
}
                              
