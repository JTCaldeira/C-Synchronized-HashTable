
#include <stdio.h>
#include <stdlib.h>

#include "hashtable.h"


/* see if PTHREAD_RWLOCK_INITIALIZER works, that way no need for ..._destroy() */


#define LOCK_RD(lock)	pthread_rwlock_rdlock(&lock);
#define LOCK_WR(lock)	pthread_rwlock_wrlock(&lock);
#define UNLOCK(lock)	pthread_rwlock_unlock(&lock);


hash_table_t *
hash_table_create(hash_table_compare_function cmp_fn, hash_table_hash_function hash_fn, int max_size)
{
	hash_table_t * table = malloc(sizeof(hash_table_t));

	if (!table) {
		perror("Error");
		return NULL;
	}

	if (max_size < 1)
		table->max_size = DEFAULT_SIZE;

	table->count = 0;
	table->elements = calloc(table->max_size, sizeof(list_node_t*));
	if (!table->elements) {
		perror("Error");
		free(table);
		return NULL;
	}

	table->locks = malloc(table->max_size * sizeof(pthread_rwlock_t));
	if (!table->locks) {
		perror("Error");
		free(table->elements);
		free(table);
		return NULL;
	}

	for (int i = 0; i < table->max_size; i++) {
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
	int hash_value = table->hash(element);
	list_node_t * new_node = list_node_create(element);

	if (!new_node) {
		perror("Error");
		return false;
	}

	if (!table->elements[hash_value]) {
		LOCK_WR(table->global_table_lock);

		if (!table->elements[hash_value]) {
			table->elements[hash_value] = new_node;
		}

		UNLOCK(table->global_table_lock);
	} else {
		LOCK_WR(table->locks[hash_value]);
		new_node->next = table->elements[hash_value];
		table->elements[hash_value] = new_node;
		UNLOCK(table->locks[hash_value]);
	}

	LOCK_WR(table->global_table_lock);
	table->count++;
	UNLOCK(table->global_table_lock);

	return true;
}


bool
hash_table_contains(hash_table_t * table, void * element)
{
	int hash_value = table->hash(element);
	list_node_t * node;

	LOCK_RD(table->locks[hash_value]);

	node = table->elements[hash_value];

	while (node) {
		if (!table->compare(node->data, element)) {
			UNLOCK(table->locks[hash_value]);
			return true;
		}

		node = node->next;
	}

	UNLOCK(table->locks[hash_value]);

	return false;
}


bool
hash_table_remove(hash_table_t * table, void * element)
{
	int hash_value = table->hash(element);
	list_node_t * node, * prev;

	LOCK_WR(table->locks[hash_value]);

	node = table->elements[hash_value];
	prev = NULL;

	while (node) {
		if (!table->compare(node->data, element)) {
			// value is first item in the list
			if (node == table->elements[hash_value]) {
				table->elements[hash_value] = node->next;
				free(node);
				UNLOCK(table->locks[hash_value]);
				LOCK_WR(table->global_table_lock);
				table->count--;
				UNLOCK(table->global_table_lock);
				return true;
			} else {
				// link previous node with one after current
				prev->next = node->next;
				free(node);
				UNLOCK(table->locks[hash_value]);
				LOCK_WR(table->global_table_lock);
				table->count--;
				UNLOCK(table->global_table_lock);
				return true;
			}
		}
		prev = node;
		node = node->next;
	}

	UNLOCK(table->locks[hash_value]);

	return false;
}


void
hash_table_destroy(hash_table_t * table)
{
	for (int i = 0; i < table->max_size; i++) {
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

