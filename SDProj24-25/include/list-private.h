#ifndef _LIST_PRIVATE_H
#define _LIST_PRIVATE_H

#include "entry.h"

struct node_t {
	struct node_t *prev; // Node anterior na lista, Null se for head
    struct entry_t *entry; // Entry guardada no node
    struct node_t *next; // Node a frente deste na lista
};

struct list_t {
    struct node_t *head; // Aponta para o pŕoximo elemento da lista
    int size; // Tamanho da lista
};

#endif
