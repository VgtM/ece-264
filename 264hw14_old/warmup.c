#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>


/*  A small structure to hold
	1. The starting pointer to the array.
	2. Members in the array.
*/

typedef struct{
	int length; // count of members in the source array
	int* source_array; // the source array itself. point to the beginning.
} MY_ARRAY;

/* A small function to take an int array and zeroize the int array */
void *to_zero(void *arg){
	MY_ARRAY *my_array;
	my_array = (MY_ARRAY*)arg;
	int length = (*my_array).length;
	int *members; // the int array itself ;
	members = (*my_array).source_array;
	for (int i=0; i<length; i++){ // each member inside array will be zeroized.
		members[i] = 0;
	}
	return NULL; // don't return anything.
}


bool zero_array(int* array, int size, int num_threads, char** a_error) {
	/* Given an array, set every element to 0 using multiple threads.
	 *
	 * - If successful, return true and do not modify error at all.
	 *
	 * - In case of any failure, return false and, if a_error != NULL,
	 *   set *a_error to the address of an error message on the heap.
	 *   The caller will be responsible for freeing it.
	 */
	/* Will divide the array into 4 pieces, the last part is the longest part.
		eg. 10 members: 10%4 = 2, will be 2,2,2,2+2
		eg. 15 members: 15%4 = 3, will be 3,3,3,3+3
		eg. 27 members: 27%4 = 3, will be 6,6,6,6+3
		if size <= num_threads, will be only a single thread.
	*/
	if (size <= num_threads){ // if the size of array is smaller than number of threads.
		for(int i=0; i<size;i++){
			array[i] = 0;
		}	
	}else{
		int tail = size % num_threads; // the tailing number after mod.
		int members_in_each_thread = (size-tail)/num_threads; // the members in each previous threads.
		int members_in_last_thread = tail + num_threads; // the members in the last thread.
		// pthread_t pth;
		// MY_ARRAY temp_array;
		// temp_array.length = size;
		// temp_array.source_array = array;
		// pthread_create(&pth,NULL,to_zero,&temp_array);
		// pthread_join(pth,NULL); // wait for the thread to join before close program.
		
		MY_ARRAY structures[num_threads]; // a pool of structures.
		for(int i=0;i<num_threads;i++){
			structures[i].source_array = array + (i * members_in_each_thread);

			if (i != num_threads - 1){
				structures[i].length = members_in_each_thread;
			}else{
				structures[i].length = members_in_last_thread;
			}
			
		}
		pthread_t threads[num_threads]; // a pool of threads.

		for(int i=0;i<num_threads;i++){ // create threads and run.
			pthread_create(&threads[i],NULL,to_zero,&structures[i]);
		}

		for(int i=0;i<num_threads;i++){ // wait for all the threads to join then exit main.
			pthread_join(threads[i],NULL);
		}
	}
	return true;
}

int main(int argc, char *argv[]) {
	int array[16] = {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};
	int size = sizeof(array)/sizeof(*array); // full memory area divide by size of first content
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

