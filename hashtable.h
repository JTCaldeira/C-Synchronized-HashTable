/* =============================================================================
 *
 * hashtable.h
 *
 * Thread safe hash table with collision resolution by external chaining.
 *
 * =============================================================================
 */


#ifndef HASHTABLE_H
#define HASHTABLE_H


#include <pthread.h>
#include <stdbool.h>


#define DEFAULT_SIZE	1013


/* type definition for a generic comparing function */
typedef int (*hash_table_compare_function)(const void *, const void *);
/* type definition for a for-each function */
typedef unsigned long (*hash_table_hash_function)(const void *);


/* type definition for the node of a generic singly linked list */
typedef struct list_node {
	void * data;				/* pointer to the data */
	struct list_node * next;	/* pointer to the next element */
} list_node_t;


/* type definition for a generic hash table with external chaining */
typedef struct hash_table {
	int size;								/* size of the hash table (num. of buckets) */
	list_node_t * * elements;				/* array of linked lists */
	pthread_mutex_t * locks;				/* array of mutex, one per bucket */
	pthread_mutex_t global_table_lock;		/* global array for the hash table */
	hash_table_compare_function compare;	/* should return 0 if equal */
	hash_table_hash_function hash;			/* returns a hash value for data */
} hash_table_t;



/* =============================================================================
 * hash_table_create
 *
 * Creates an empty hash table with the given size.
 *
 * @param hash_table_compare_function: the function that is used to find
 *				element within a bucket.
 * @param hash_table_hash_function: the function that is used to disperse
 *				data into the buckets.
 * @param max_size: the size of the hash table.
 * @return: a pointer to the created hash table.
 * =============================================================================
 */
hash_table_t *
hash_table_create(hash_table_compare_function, hash_table_hash_function, int max_size);


/* =============================================================================
 * hash_table_insert
 *
 * Inserts a given object into the hash table.
 *
 * @param table: the table in which to insert the object
 * @param element: the data to be inserted in the table
 * @return: true if the operation succeeded, false otherwise
 * =============================================================================
 */
bool
hash_table_insert(hash_table_t * table, void * element);


/* =============================================================================
 * hash_table_contains
 *
 * Check whether the hash table contains a given object.
 *
 * @param table: the hash table in which we want to search the object
 * @param element: the data to search for
 * @return: true if the object is in the hash table, false otherwise
 * =============================================================================
 */
bool
hash_table_contains(hash_table_t * table, void * element);


/* =============================================================================
 * hash_table_remove
 *
 * Removes a given object from the hash table.
 *
 * @param table: the hash table we want to remove the object from
 * @param element: the data to be removed
 * @return: true if the operation succeeded, false otherwise
 * =============================================================================
 */
bool
hash_table_remove(hash_table_t * table, void * element);


/* =============================================================================
 * hash_table_destroy
 *
 * Free all the memory allocated in the creation of the hash table.
 *
 * @param table: the table we want to destroy
 * =============================================================================
 */
void
hash_table_destroy(hash_table_t * table);


#endif /* HASHTABLE_H */
