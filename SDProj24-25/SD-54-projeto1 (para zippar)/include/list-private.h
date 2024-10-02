#ifndef _LIST_PRIVATE_H
#define _LIST_PRIVATE_H

#include "entry.h"

// Define a estrutura de cada node na List
struct node_t {
	struct node_t *prev;
    struct entry_t *entry;
    struct node_t *next;
};

// Define the list structure
struct list_t {
    struct node_t *head; // Aponta para o pŕoximo elemento da lista
    int size;            // Tamanho da lista
};

#endif
