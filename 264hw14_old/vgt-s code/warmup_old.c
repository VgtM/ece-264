#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>

/* TODO:  Create a worker function to go with zero_array(..) */

typedef struct{
	int* array;
	int thidx;
	int count;
}ZeroArg;

char* _strdup(const char* src){
	return strcpy(malloc((strlen(src)+1)*sizeof(*src)), src);
}

void* zero_arry_worker(void* arg){
	int* clr = (int*)arg;
	while(*++clr != '\0'){
		*clr = 0;
	}
	return NULL;
}

bool zero_array(int* array, int size, int num_threads, char** a_error) {
	ZeroArg* zero = malloc(sizeof(*zero));
	zero->thidx = 0;
	zero->array = malloc(sizeof(*array));
	zero->count = 0;
	if(size < 0){
		if(*a_error == NULL){
			char* message = "cannot open the array because the array size is wrong.";
			*a_error = _strdup(message);
		}
		return false;
	}
	if(array == NULL){
		if(*a_error == NULL){
			char* message = "cannot open the array because the array is empty.";
			*a_error = _strdup(message);
		}
		return false;
	}
	pthread_t *thread = malloc(sizeof(*thread)*num_threads);
	for(int i = 0; i < num_threads; i++){
		int j = 0;
		while(j < size/num_threads){
			int check_thread = pthread_create(&thread[i], NULL, zero_arry_worker, &array[j + thidx]);
			j++;
			if(check_thread){
				if(*a_error == NULL){
				char* message = "cannot create the thread.";
				*a_error = _strdup(message);
				}
			return false;
			}
		}
		thidx += size/num_threads;
	}
		for(int i = 0; i < num_threads; i++){
		pthread_join(thread[i], NULL);
	}
			
	/* Given an array, set every element to 0 using multiple threads.
	 *
	 * - If successful, return true and do not modify error at all.
	 *
	 * - In case of any failure, return false and, if a_error != NULL,
	 *   set *a_error to the address of an error message on the heap.
	 *   The caller will be responsible for freeing it.
	 */
	return true; /* TODO: finish this function and remove this stub */
}

int main(int argc, char *argv[]) {
	int array[16] = {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};
	int size = sizeof(array)/sizeof(*array);
	for(int i=0; i<size; i++) {
		assert(array[i] == 2);
	}
	char* error = NULL;
	if( zero_array(array, sizeof(array)/sizeof(*array), 4, &error) ) {
		for(int i=0; i<size; i++) {
			assert(array[i] == 0);
		}
		return EXIT_SUCCESS;
	}
	else {
		fprintf(stderr, "%s\n", error);
		free(error);
		return EXIT_FAILURE;
	}
}

