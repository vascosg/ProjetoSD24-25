#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/entry.h"

/* Função que cria uma entry, reservando a memória necessária e
 * inicializando-a com a string e o bloco de dados de entrada.
 * Retorna a nova entry ou NULL em caso de erro.
 */
struct entry_t *entry_create(char *key, struct block_t *value) {

	if (key == NULL || value == NULL) { // Verifica se a chave é válida
		return NULL; // Retorna NULL se a verificação falhar
	}

	struct entry_t *new_entry = malloc(sizeof(struct entry_t)); // Aloca memória para a nova entry

	if (!new_entry) {
		return NULL; // Falha ao alocar memória
	}

	// Aloca memória para a chave e copia a string
	/*new_entry->key = malloc(strlen(key) + 1); // +1 para o terminador nulo
	if (!new_entry->key) {
		free(new_entry); // Libera a entry se falhar
		return NULL;
	}

	strcpy(new_entry->key, key); // Copia a chave*/
	new_entry->key = key;

	// Atribui o valor do bloco a nova entrada
	new_entry->value = value; // Duplica o bloco
	/*if (!new_entry->value) {
		free(new_entry->key); // Liberta a chave se a duplicação falhar
		free(new_entry); // Liberta a entry
		return NULL;
	}*/

	return new_entry; // Retorna a nova entry criada
}

/* Função que compara duas entries e retorna a ordem das mesmas, sendo esta
 * ordem definida pela ordem das suas chaves.
 * Retorna 0 se as chaves forem iguais, -1 se e1 < e2,
 * 1 se e1 > e2 ou -2 em caso de erro.
 */
int entry_compare(struct entry_t *e1, struct entry_t *e2) {

	if (e1 == NULL || e2 == NULL) {
		return -2; // Retorna -2 em caso de erro
	}
	return strcmp(e1->key, e2->key); // Compara as chaves
}

/* Função que duplica uma entry, reservando a memória necessária para a
 * nova estrutura.
 * Retorna a nova entry ou NULL em caso de erro.
 */
struct entry_t *entry_duplicate(struct entry_t *e) {

	if (e == NULL) return NULL; // Retorna NULL se a entrada não existir

	struct block_t *dup_value = block_duplicate(e->value); // Duplicar o bloco de dados para garantir uma cópia independente
	if (dup_value == NULL) {
		return NULL; // Retorna NULL se a duplicação do bloco falhar
	}

	char *dup_key = strdup(e->key);
	if (dup_key == NULL) {
	    block_destroy(dup_value);
	    return NULL;
	}
	struct entry_t *dup_entry = entry_create(dup_key, dup_value);// Cria uma nova entry, dup_value é destruido e dup_entry fica com uma cópia

	if (!dup_entry) {
		return NULL; // Retorna NULL em caso de falha
	}

	return dup_entry; // Retorna a nova entry duplicada
}

/* Função que substitui o conteúdo de uma entry, usando a nova chave e
 * o novo valor passados como argumentos, e eliminando a memória ocupada
 * pelos conteúdos antigos da mesma.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int entry_replace(struct entry_t *e, char *new_key, struct block_t *new_value) {

	if (e == NULL || new_key == NULL || new_value == NULL) {
		return -1; // Retorna -1 em caso de erro
	}

	// Libera a memória ocupada pela chave antiga
	free(e->key);

	// Aloca memória para a nova chave e copia
	e->key = new_key;

	// Substitui o valor antigo pelo novo valor
	if (block_replace(e->value, new_value->datasize, new_value->data) != 0) {
		free(e->key); // Liberta a chave se a substituição falhar
		return -1; // Retorna -1 em caso de erro
	}

	return 0; // Retorna 0 em caso de sucesso
}

/* Função que elimina uma entry, libertando a memória por ela ocupada.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int entry_destroy(struct entry_t *e) {

	if (!e || !e->key || !e->value ) {
		return -1; // Retorna -1 se a entry não existir
	}

	free(e->key); // Liberta a memória da chave
	block_destroy(e->value); // Libera o bloco de dados
	free(e); // Liberta a memória da entry

	return 0; // Retorna 0 em caso de sucesso
}
