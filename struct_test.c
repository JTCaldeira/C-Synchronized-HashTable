


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include "hashtable.h"


#define	AMOUNT_PEOPLE	5


typedef struct person {
	int id;
	char * name;
} person_t;


// gcc -Wall -Wextra -ggdb3 -std=c99 -D_GNU_SOURCE -pthread -o test struct_test.c hashtable.c -lm
// valgrind -v --leak-check=full --show-leak-kinds=all --track-origins=yes ./test


int
person_compare_function(const void * p, const void * id)
{
	return ((person_t*)p)->id - *(int*)id;
}


int
person_hash_function(const void * p)
{
	unsigned long hash = 5381;
	int c;
	char * str = (char*)(((person_t*)p)->name);

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash % 1013;
}


person_t *
person_create(int id, char * name)
{
	person_t * new_person = malloc(sizeof(person_t));

	new_person->id = id;
	new_person->name = malloc((strlen(name) + 1) * sizeof(char));
	strcpy(new_person->name, name);

	return new_person;
}


int
main(void)
{
	hash_table_t * table = hash_table_create(person_compare_function, person_hash_function, -1);
	person_t * people[AMOUNT_PEOPLE] = {
						person_create(1,	"Freddie Mercury"),
						person_create(111,	"John Bonham"),
						person_create(10,	"David Gilmour"),
						person_create(90,	"James Hetfield"),
						person_create(9999,	"Tobias Forge")
						};

	int i;
	for (i = 0; i < AMOUNT_PEOPLE; i++)
		if (!hash_table_insert(table, people[i])) {
			fprintf(stderr, "Error inserting %s with ID number %d into the table.\n", people[i]->name, people[i]->id);
			return EXIT_FAILURE;
		}

	int id_to_remove = 2222;
	if (!hash_table_remove(table, (void*)&id_to_remove))
		printf("No person found with ID number 2222\n");

	id_to_remove = 10;
	if (!hash_table_remove(table, (void*)&id_to_remove))
		printf("Removed person with ID number 10\n");
	else
		fprintf(stderr, "Error removing person with ID number %d from the table.\n", people[i]->id);

	hash_table_destroy(table);

	for (i = 0; i < AMOUNT_PEOPLE; i++) {
		free(people[i]->name);
		free(people[i]);
	}

	return EXIT_SUCCESS;
}