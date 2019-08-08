
#include <stdio.h>
#include <stdlib.h>

#include "hashtable.h"


/* see if PTHREAD_RWLOCK_INITIALIZER works, that way no need for ..._destroy() */


#define LOCK_RD(lock)	pthread_rwlock_rdlock(&lock);
#define LOCK_WR(lock)	pthread_rwlock_wrlock(&lock);
#define UNLOCK(lock)	pthread_rwlock_unlock(&lock);
#define	GET_INDEX(hash_value, size)	{ hash_value % size }


hash_table_t *
hash_table_create(hash_table_compare_function cmp_fn, hash_table_hash_function hash_fn, int size)
{
	hash_table_t * table = malloc(sizeof(hash_table_t));

	if (!table) {
		perror("Error");
		return NULL;
	}

	if (size < 1)
		table->size = DEFAULT_SIZE;
	else
		table->size = size;

	table->count = 0;
	table->elements = calloc(table->size, sizeof(list_node_t*));
	if (!table->elements) {
		perror("Error");
		free(table);
		return NULL;
	}

	table->locks = malloc(table->size * sizeof(pthread_rwlock_t));
	if (!table->locks) {
		perror("Error");
		free(table->elements);
		free(table);
		return NULL;
	}

	for (int i = 0; i < table->size; i++) {
		table->elements[i] = NULL;
		pthread_rwlock_init(&table->locks[i], NULL);
	}

	if (!cmp_fn || !hash_fn) {
		perror("Error");
		free(table->elements);
		free(table->locks);
		free(table);
		return NULL;
	}

	table->compare = cmp_fn;
	table->hash = hash_fn;
	pthread_rwlock_init(&table->global_table_lock, NULL);

	return table;
}


static list_node_t *
list_node_create(void * data)
{
	list_node_t * node = malloc(sizeof(list_node_t));

	if (!node) {
		perror("Error");
		return NULL;
	}

	node->data = data;
	node->next = NULL;

	return node;
}


bool
hash_table_insert(hash_table_t * table, void * element)
{
	int index = GET_INDEX(table->hash(element), table->size);
	list_node_t * new_node = list_node_create(element);

	if (!new_node) {
		perror("Error");
		return false;
	}

	if (!table->elements[index]) {
		LOCK_WR(table->global_table_lock);

		if (!table->elements[index]) {
			table->elements[index] = new_node;
		}

		UNLOCK(table->global_table_lock);
	} else {
		LOCK_WR(table->locks[index]);
		new_node->next = table->elements[index];
		table->elements[index] = new_node;
		UNLOCK(table->locks[index]);
	}

	/*
	LOCK_WR(table->global_table_lock);
	table->count++;
	UNLOCK(table->global_table_lock);
	*/

	return true;
}


bool
hash_table_contains(hash_table_t * table, void * element)
{
	int index = GET_INDEX(table->hash(element), table->size);
	list_node_t * node;

	LOCK_RD(table->locks[index]);

	node = table->elements[index];

	while (node) {
		if (!table->compare(node->data, element)) {
			UNLOCK(table->locks[index]);
			return true;
		}

		node = node->next;
	}

	UNLOCK(table->locks[index]);

	return false;
}


bool
hash_table_remove(hash_table_t * table, void * element)
{
	int index = GET_INDEX(table->hash(element), table->size);
	list_node_t * node, * prev;

	LOCK_WR(table->locks[index]);

	node = table->elements[index];
	prev = NULL;

	while (node) {
		if (!table->compare(node->data, element)) {
			// value is first item in the list
			if (node == table->elements[index]) {
				table->elements[index] = node->next;
				free(node);
				UNLOCK(table->locks[index]);
				/*
				LOCK_WR(table->global_table_lock);
				table->count--;
				UNLOCK(table->global_table_lock);
				*/
				return true;
			} else {
				// link previous node with one after current
				prev->next = node->next;
				free(node);
				UNLOCK(table->locks[index]);
				/*
				LOCK_WR(table->global_table_lock);
				table->count--;
				UNLOCK(table->global_table_lock);
				*/
				return true;
			}
		}
		prev = node;
		node = node->next;
	}

	UNLOCK(table->locks[index]);

	return false;
}


void
hash_table_destroy(hash_table_t * table)
{
	for (int i = 0; i < table->size; i++) {
		pthread_rwlock_destroy(&table->locks[i]);

		list_node_t * node = table->elements[i], * aux = NULL;

		while (node) {
			aux = node->next;
			free(node);
			node = aux;
		}
	}

	free(table->locks);
	free(table->elements);
	free(table);
}


int
hash_table_getCount(hash_table_t * table)
{
	return table->count;
}

