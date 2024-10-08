#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/entry.h"

/* Função que cria uma entry, reservando a memória necessária e
 * inicializando-a com a string e o bloco de dados de entrada.
 * Retorna a nova entry ou NULL em caso de erro.
 */
struct entry_t *entry_create(char *key, struct block_t *value) {

	if ( !key || !value ) { // Verifica se a chave é válida
		return NULL;
	}

	struct entry_t *new_entry = malloc(sizeof(struct entry_t)); // Aloca memória para a nova entry

	if (!new_entry) return NULL; // Falha ao alocar memoria

	new_entry->key = key;
	new_entry->value = value;

	return new_entry;
}

/* Função que compara duas entries e retorna a ordem das mesmas, sendo esta
 * ordem definida pela ordem das suas chaves.
 * Retorna 0 se as chaves forem iguais, -1 se e1 < e2,
 * 1 se e1 > e2 ou -2 em caso de erro.
 */
int entry_compare(struct entry_t *e1, struct entry_t *e2) {

	if ( !e1  || !e2 ) {
		return -2; // Retorna -2 em caso de erro
	}

	return strcmp(e1->key, e2->key); // Compara as chaves
}

/* Função que duplica uma entry, reservando a memória necessária para a
 * nova estrutura.
 * Retorna a nova entry ou NULL em caso de erro.
 */
struct entry_t *entry_duplicate(struct entry_t *e) {

	if (!e ) return NULL; // Retorna NULL se a entrada não existir

	struct block_t *dup_value = block_duplicate(e->value); // Duplicar o bloco de dados para garantir uma cópia independente
	if ( !dup_value ) return NULL; // Falha ao duplicar bloco

	char *dup_key = strdup(e->key);

	if ( !dup_key ) { // Falha ao duplicar key == destruir bloco
	    block_destroy(dup_value);
	    return NULL;
	}

	struct entry_t *dup_entry = entry_create(dup_key, dup_value);// Cria uma nova entry, dup_value é destruido e dup_entry fica com uma cópia

	if (!dup_entry) return NULL; // Falha ao fazer o dup

	return dup_entry;
}

/* Função que substitui o conteúdo de uma entry, usando a nova chave e
 * o novo valor passados como argumentos, e eliminando a memória ocupada
 * pelos conteúdos antigos da mesma.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int entry_replace(struct entry_t *e, char *new_key, struct block_t *new_value) {

	if (!e  || !new_key  || !new_value ) {
		return -1; // Retorna -1 em caso de erro
	}

	entry_destroy(e); // Liberta a memória ocupada pela entry
	
	// Aloca memória para a nova chave e copia
	entry_create(new_key, new_value);

	return 0;
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
	free(e); // Libera a estrutura

	return 0;
}
