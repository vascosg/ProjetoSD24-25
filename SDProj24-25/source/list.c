/* ------------------------------------
---------------  SD-054 ---------------
    Filipa Inácio       fc59788
    Tomás Canilhas      fc59794
    Vasco Baldé         fc58174
---------------------------------------
------------------------------------ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/list-private.h"
#include "../include/list.h"

/* Função que cria e inicializa uma nova lista (estrutura list_t a
 * ser definida pelo grupo no ficheiro list-private.h).
 * Retorna a lista ou NULL em caso de erro.
 */
struct list_t *list_create() {

	struct list_t *list = (struct list_t *)malloc(sizeof(struct list_t)); // ALocar memoria para a lista

	if (!list) return NULL; // Falha a alocar memoria

	list->head = NULL;
	list->size = 0;

	return list;
}

/* Função que adiciona à lista a entry passada como argumento.
 * A entry é inserida de forma ordenada, tendo por base a comparação
 * de entries feita pela função entry_compare do módulo entry e
 * considerando que a entry menor deve ficar na cabeça da lista.
 * Se já existir uma entry igual (com a mesma chave), a entry
 * já existente na lista será substituída pela nova entry,
 * sendo libertada a memória ocupada pela entry antiga.
 * Retorna 0 se a entry ainda não existia, 1 se já existia e foi
 * substituída, ou -1 em caso de erro.
 */
int list_add(struct list_t *l, struct entry_t *entry) {

	if (!l || !entry) { // Em cas ode argumentos invalidos
		return -1;
	}

	struct node_t *new_node = malloc(sizeof(struct node_t)); // Criar o novo node que vai conter a entrada e
	                                                         // prev node e next node

	if (!new_node) return -1; // Falha ao criar o novo node

	new_node->entry = entry;

	if (!new_node->entry) {
		free(new_node);
		return -1;
	}

	new_node->next = NULL;
	new_node->prev = NULL;

	if (l->size == 0) { // Caso da lista vazia

		l->head = new_node;
		l->size++;
		return 0;
	}

	// percorre a lista ate encontrar onde por o node

	for (struct node_t *current = l->head; current != NULL; current = current->next) {


		if (entry_compare(current->entry, new_node->entry) == 0) { // Entrys iguais

			//Como têm a mesma key, devem se dar replace
			//entry_replace(current->entry, new_node->entry->key, new_node->entry->value);

			entry_destroy(current->entry);
			current->entry = new_node->entry;

			free(new_node);
			return 1;

		} else if (entry_compare(current->entry, new_node->entry) == 1) { // New_node e mais pequeno que o current
			                                                              // Logo vai new_node fica atraz de current na lisa

			if (current == l->head) { // Adicionar a cabeça da lista

				l->head = new_node;
				new_node->next = current;
				current->prev = new_node;

			} else { //Adicionar no meio da l ista

				current->prev->next = new_node;
				new_node->prev = current->prev;
				current->prev = new_node;
				new_node->next = current;
			}


			l->size++; // Tamanho da lista aumenta

			return 0;

		} else if (current->next == NULL) { // New_node e o maior elemento, vai para fim da lista

			new_node->prev = current;
			current->next = new_node;
			l->size++; // Tamanho da lista aumenta

			return 0;
		}

	}

	return -1;
}

/* Função que conta o número de entries na lista passada como argumento.
 * Retorna o tamanho da lista ou -1 em caso de erro.
 */
int list_size(struct list_t *l) {

	if (!l) return -1;
	return l->size;
}

/* Função que obtém da lista a entry com a chave key.
 * Retorna a referência da entry na lista ou NULL se não encontrar a
 * entry ou em caso de erro.
 */
struct entry_t *list_get(struct list_t *l, char *key) {

	if (!l || !key) { // Argumentos invalidos
		return NULL;
	}

	struct node_t *current = l->head;

	while (current != NULL) {// Enquanto o current n for o fim da lista

		if (strcmp(current->entry->key, key) == 0) { // Encontra chave
			return current->entry;
		}

		current = current->next;
	}

	return NULL; // Nao encontra chave
}

/* Função auxiliar que constrói um array de char* com a cópia de todas as keys na
 * lista, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 * Retorna o array de strings ou NULL em caso de erro.
 */
char **list_get_keys(struct list_t *l) {

	if (!l || l->size <=0 || !l->head) return NULL;

	char **keys = malloc((l->size + 1) * sizeof(char *));
	if (!keys) return NULL; // Falha ao alocar memoria para as chaves

	struct node_t *current = l->head;
	int i = 0;
	while (current != NULL) {// Enquanto o current n for o fim da lista
		keys[i] = strdup(current->entry->key); // Copia a chave

		if (!keys[i]) {
			list_free_keys(keys); // Se a copia dachave falhar
			return NULL;
		}

		current = current->next;
		i++;
	}
	keys[i] = NULL; // Null a terminar o array
	return keys;
}

/* Função auxiliar que liberta a memória ocupada pelo array de keys obtido pela
 * função list_get_keys.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int list_free_keys(char **keys) {

	if (!keys) return -1; // Argumentos invalidos

	int i = 0;
	while (keys[i] != NULL) {
		free(keys[i]);
		i++;
	}

	free(keys);
	return 0;
}

/* Função que elimina da lista a entry com a chave key, libertando a
 * memória ocupada pela entry.
 * Retorna 0 se encontrou e removeu a entry, 1 se não encontrou a entry,
 * ou -1 em caso de erro.
 */
int list_remove(struct list_t *l, char *key) {

	if (!l || !key) { // Argumentos invalidos
		return -1;
	}

	struct node_t *current = l->head;

	while (current != NULL) { // Enquanto o current n for o fim da lista
		if (strcmp(current->entry->key, key) == 0) { // Encontra a chave para remover
			if (current == l->head) { // Retirar cabeça

				l->head = current->next;

			}else if (current->next == NULL) { // Retira no fim
				current->prev->next = current->next;

			} else { // Retira no meio
				current->prev->next = current->next;
				current->next->prev = current->prev;
			}

			entry_destroy(current->entry); // Elimina a entry

			free(current);
			l->size--;
			return 0;
		}
		current = current->next;
	}

	return 1; // Entrada nao encontrada
}

/* Função que elimina uma lista, libertando *toda* a memória utilizada
 * pela lista (incluindo todas as suas entradas).
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
/* Destroys the entire list and frees all memory */
int list_destroy(struct list_t *l) {

	if (!l) return -1; // Argumentos invalidos

	struct node_t *current = l->head;
	while (current != NULL) { // Enquanto o current n for o fim da lista
		struct node_t *next_node = current->next;
		entry_destroy(current->entry); // Destroi cada entry
		free(current);
		current = next_node;
	}

	free(l);
	return 0;
}
