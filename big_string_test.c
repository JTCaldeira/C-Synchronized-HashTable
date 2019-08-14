


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <time.h>

#include "hashtable.h"


#define PRINT_USAGE	printf("USAGE:\t./test <NUM_THREADS> <NUM_STRINGS> <NUM_LOOKUPS>\n\n\
					<NUM_THREADS>: The number of threads to do work.\n\
					<NUM_STRINGS>: The total number of strings.\n\t\t\
					Should be a common multiple to all integers between and including\n\t\t\
					1 and the number of cores.\n\
					<NUM_LOOKUPS>: The total number of lookups.\n\t\t\
					Should be a common multiple to all integers between and including\n\t\t\
					1 and the number of cores.\n");


// gcc -Wall -Wextra -ggdb3 -std=c99 -D_GNU_SOURCE -pthread -o test big_string_test.c hashtable.c -lm
// valgrind -v --leak-check=full --show-leak-kinds=all --track-origins=yes ./test


int num_threads;
int num_strings;
int num_lookups;

hash_table_t * table;


void
parse_args(int argc, const char * argv[])
{
	if (argc != 4) {
		PRINT_USAGE;
		exit(1);
	}

	num_threads = atoi(argv[1]);
	num_strings = atoi(argv[2]);
	num_lookups = atoi(argv[3]);

	if (num_threads < 1) {
		fprintf(stderr, "ERROR: Invalid number of threads.\n");
		PRINT_USAGE;
		exit(1);
	} else if (num_strings < 1) {
		fprintf(stderr, "ERROR: Invalid number of strings.\n");
		PRINT_USAGE;
		exit(1);
	} else if (num_strings % num_threads != 0) {
		fprintf(stderr, "WARNING: Number of strings is not a multiple of the number of threads.\n");
	} else if (num_lookups < 1) {
		fprintf(stderr, "ERROR: Invalid number of lookups.\n");
		PRINT_USAGE;
		exit(1);
	}
}


unsigned long
str_hash_function(void * a)
{
	unsigned long hash = 5381;
	int c;
	char * str = (char*)a;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}


void *
do_work(void * data)
{
	int thread_id = *(int*)data;

	// write "threadX.txt" to filename, where X is the given thread id
	char filename[64];
	snprintf(filename, sizeof(filename), "thread%d.txt", thread_id);

	FILE * file = fopen(filename, "r");
	char buffer[128];
	unsigned long i;
	unsigned long num_str_per_thread = num_strings / num_threads;
	unsigned long num_lookups_per_thread = num_lookups / num_threads;
	char * str_array[num_str_per_thread];

	for (i = 0; i < num_str_per_thread; i++) {
		if (!fgets(buffer, 128, file)) {
			fprintf(stderr, "fgets() call failed.");
			exit(1);
		}

		str_array[i] = calloc((strlen(buffer) + 1), sizeof(char));
		strcpy(str_array[i], buffer);
	}

	fclose(file);

	for (i = 0; i < num_str_per_thread; i++)
		hash_table_insert(table, str_array[i]);

	// ************************************
	struct timespec start, end;
	clock_gettime(CLOCK_REALTIME, &start);

	for (i = 0; i < num_lookups_per_thread; i++)
		hash_table_contains(table, str_array[rand() % num_str_per_thread]);

	clock_gettime(CLOCK_REALTIME, &end);

	double a1 = start.tv_sec + start.tv_nsec*1e-9;
	double a2 = end.tv_sec + end.tv_nsec*1e-9;

	printf("Lookup time: %.4lf\n", a2 - a1);

	int str_index;
	for (i = 0; i < num_str_per_thread; i++) {
		str_index = rand() % num_str_per_thread;

		if (str_array[str_index]) {
			hash_table_remove(table, str_array[str_index]);
			free(str_array[str_index]);
			str_array[str_index] = NULL;
		}
	}

	for (i = 0; i < num_str_per_thread; i++)
		if (str_array[i])
			free(str_array[i]);

	return NULL;
}


void
create_workers()
{
	pthread_t threads[num_threads];
	int ids[num_threads];
	int i;

	for (i = 0; i < num_threads; i++)
		ids[i] = i + 1;

	for (i = 0; i < num_threads; i++)
		pthread_create(&threads[i], NULL, do_work, (void*)&ids[i]);

	for (i = 0; i < num_threads; i++)
		pthread_join(threads[i], NULL);

	int bigcounter = 0;

	int counter;
	for (int i = 0; i < table->size; i++) {
		list_node_t * iter = table->elements[i];
		counter = 0;

		while (iter) {
			counter++;
			iter = iter->next;
		}

		if (counter > 4)
			bigcounter++;
	}
}


int
main(int argc, const char * argv[])
{
	parse_args(argc, argv);
	table = hash_table_create((hash_table_compare_function)strcmp, (hash_table_hash_function)str_hash_function, 50723);

	struct timespec start, end;
	clock_gettime(CLOCK_REALTIME, &start);

	// create threads to do work
	create_workers(num_threads);

	clock_gettime(CLOCK_REALTIME, &end);

	double a1 = start.tv_sec + start.tv_nsec*1e-9;
	double a2 = end.tv_sec + end.tv_nsec*1e-9;

	printf("Time: %.4lf\n", a2 - a1);

	hash_table_destroy(table);

	return EXIT_SUCCESS;
}
