


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include "hashtable.h"


#define	NUM_THREADS 2
#define NUM_STRINGS	154560
#define	MAX_STRING_LEN	100
#define MAX_THREADS	16
#define	NUM_LOOKUPS	10000


// gcc -Wall -Wextra -ggdb3 -std=c99 -D_GNU_SOURCE -pthread -o test test.c hashtable.c -lm
// valgrind -v --leak-check=full --show-leak-kinds=all --track-origins=yes ./test


hash_table_t * table;


int
str_hash_function(void * a)
{
	unsigned long hash = 5381;
	int c;
	char * str = (char*)a;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash % 1013;
}


void *
do_work(void * data)
{
	int thread_id = *(int*)data;

	// write "threadX.txt" to filename, where X is the given thread id
	char filename[64];
	strcpy(filename, "thread");
	char thread_id_str[4];
	sprintf(thread_id_str, "%d", thread_id);
	strcat(filename, thread_id_str);
	strcat(filename, ".txt");

	FILE * file = fopen(filename, "r");
	char buffer[128];
	int i, num_str_per_thread = NUM_STRINGS / NUM_THREADS;
	char * str_array[num_str_per_thread];

	for (i = 0; i < num_str_per_thread; i++) {
		fgets(buffer, 128, file);

		str_array[i] = calloc((strlen(buffer) + 1), sizeof(char));
		strcpy(str_array[i], buffer);
	}

	fclose(file);

	for (i = 0; i < num_str_per_thread; i++)
		hash_table_insert(table, str_array[i]);

	for (i = 0; i < NUM_LOOKUPS; i++)
		hash_table_contains(table, str_array[rand() % num_str_per_thread]);

	for (i = 0; i < num_str_per_thread / 2; i++) {
		int str_index = rand() % num_str_per_thread;

		if (str_array[str_index]) {
			hash_table_remove(table, str_array[str_index]);
			free(str_array[str_index]);
			str_array[str_index] = NULL;
		}
	}

	//sleep(2);
	/*
	for (i = 0; i < num_str_per_thread / 2; i++)
{
    int str_index = rand() % num_str_per_thread;

    if (str_array[str_index])
    {
        hash_table_remove(table, str_array[str_index]);
        free(str_array[str_index]);
        str_array[str_index] = NULL;
    }
}
*/

	for (i = 0; i < num_str_per_thread; i++)
		if (str_array[i])
			free(str_array[i]);

	return NULL;
}


void
create_workers()
{
	pthread_t threads[NUM_THREADS];
	int ids[NUM_THREADS];
	int i;

	for (i = 0; i < NUM_THREADS; i++)
		ids[i] = i + 1;

	for (i = 0; i < NUM_THREADS; i++)
		pthread_create(&threads[i], NULL, do_work, (void*)&ids[i]);

	for (i = 0; i < NUM_THREADS; i++)
		pthread_join(threads[i], NULL);
}


int
main(void)
{
	table = hash_table_create((hash_table_compare_function)strcmp, (hash_table_hash_function)str_hash_function, -1);

	// do le thread stuff
	create_workers(NUM_THREADS);

	hash_table_destroy(table);

	return 0;
}