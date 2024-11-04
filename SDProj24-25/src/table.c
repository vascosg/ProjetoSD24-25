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
#include "../include/table.h"
#include "../include/list.h"
#include "../include/table-private.h"

/* Função para criar e inicializar uma nova tabela hash, com n
 * linhas (n = módulo da função hash).
 * Retorna a tabela ou NULL em caso de erro.
 */
struct table_t *table_create(int n) {

	if (n <= 0) return NULL; // Argumentos invalidos

	struct table_t *table = (struct table_t *)malloc(sizeof(struct table_t));
	if (!table ) return NULL; // Falha ao alocar memoria

	table->size = n;
	table->lists = (struct list_t **)malloc(n * sizeof(struct list_t *));
	if ( !table->lists ) { // Falha ao alocar memoria
		free(table);
		return NULL;
	}

	// Inicializar as listas
	for (int i = 0; i < n; i++) {
		table->lists[i] = list_create();
		if (!table->lists[i] ) { // Verifica se falha ao cruar a lista i
			for (int j = 0; j < i; j++) {
				list_destroy(table->lists[j]);
			}
			free(table->lists);
			free(table);
			return NULL;
		}
	}

	return table;
}

/* Função para adicionar um par chave-valor à tabela. Os dados de entrada
 * desta função deverão ser copiados, ou seja, a função vai criar uma nova
 * entry com *CÓPIAS* da key (string) e dos dados. Se a key já existir na
 * tabela, a função tem de substituir a entry existente na tabela pela
 * nova, fazendo a necessária gestão da memória.
 * Retorna 0 (ok) ou -1 em caso de erro.
 */
int table_put(struct table_t *t, char *key, struct block_t *value) {

	if ( !t || !key || !value ) { // Argumentos invalidos
		return -1;
	}

	int index = hash_function(key, t->size); // Obtem o index da lista  para key na tabela

	char *key_copy = strdup(key);
	struct entry_t *new_entry = entry_create(key_copy, block_duplicate(value));
	struct entry_t *copy_entry = entry_duplicate(new_entry);

	entry_destroy(new_entry);

	if(list_add(t->lists[index],copy_entry) == -1 ) return -1;

	//free(value); ???
	return 0;
}

/* Função que procura na tabela uma entry com a chave key.
 * Retorna uma *CÓPIA* dos dados (estrutura block_t) nessa entry ou
 * NULL se não encontrar a entry ou em caso de erro.
 */
struct block_t *table_get(struct table_t *t, char *key) {

	if (!t || !key) { // Argumentos Invalidos
		return NULL;
	}

	int index = hash_function(key, t->size);// Obtem o index da lista  para key na tabela
	struct entry_t *entry = list_get(t->lists[index], key);

	if (!entry) return NULL; // Falha ao obter a entry da lista

	struct block_t *duplicated = block_duplicate(entry->value);

	if (!duplicated) return NULL; // Falha ao duplicar o block

	return duplicated;
}

/* Função que conta o número de entries na tabela passada como argumento.
 * Retorna o tamanho da tabela ou -1 em caso de erro.
 */
int table_size(struct table_t *t) {

	if (!t ) return -1; // Argumentos invalidos

	int count = 0;
	for (int i = 0; i < t->size; i++) {
		count += list_size(t->lists[i]); // Conta cada linha
	}

	return count;
}

/* Função auxiliar que constrói um array de char* com a cópia de todas as keys na
 * tabela, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 * Retorna o array de strings ou NULL em caso de erro.
 */
char **table_get_keys(struct table_t *t) {

	if ( !t ) return NULL; // Argumentos invalidos

	int size = table_size(t);
	if (size <= 0) return NULL; // Lista sem keys

	char **keys = (char **)malloc((size + 1) * sizeof(char *));
	if (!keys ) return NULL; // Falha ao alocar memoria para a lista de keys

	int idx = 0;
	for (int i = 0; i < t->size; i++) {
		char **list_keys = list_get_keys(t->lists[i]); // Obtem a lista com as chaves da lista i

		if(list_size(t->lists[i]) > 0) {
			for (int j = 0; list_keys[j] != NULL; j++) { // Percorre a lista de keys obtida anteriormente e copia cada key para a var keys
				keys[idx] = strdup(list_keys[j]);
				idx++;
			}
		}

		list_free_keys(list_keys);
	}

	keys[idx] = NULL;
	return keys;
}

/* Função auxiliar que liberta a memória ocupada pelo array de keys obtido pela
 * função table_get_keys.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int table_free_keys(char **keys) {

	if ( !keys ) return -1; // Argumentos invalidos

	return list_free_keys(keys);
}

/* Função que remove da lista a entry com a chave key, libertando a
 * memória ocupada pela entry.
 * Retorna 0 se encontrou e removeu a entry, 1 se não encontrou a entry,
 * ou -1 em caso de erro.
 */
int table_remove(struct table_t *t, char *key) {
	if ( !t || !key ) return -1; // Argumentos invalidos

	int index = hash_function(key, t->size); // Obtem o index da lista  para key na tabela
	return list_remove(t->lists[index], key);
}

/* Função que elimina uma tabela, libertando *toda* a memória utilizada
 * pela tabela.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int table_destroy(struct table_t *t) {

	if ( !t ) return -1; // Argumentos invalidos

	for (int i = 0; i < t->size; i++) { //  Destroi cada lista da table
		list_destroy(t->lists[i]);
	}

	free(t->lists);
	free(t);

	return 0;
}

