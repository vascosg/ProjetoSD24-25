#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server_skeleton.h"
#include "table.h"
#include "htmessages.pb-c.h"

/* Inicia o skeleton da tabela. * O main() do servidor deve chamar esta função antes de poder usar a * função invoke(). O parâmetro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Retorna a tabela criada ou NULL em caso de erro. */
struct table_t *server_skeleton_init(int n_lists){

	struct table_t *table = table_create(n_lists);
	if (!table) {
		fprintf(stderr, "Erro ao criar a tabela.\n");
		return NULL;
	}
	return table;
}

/* Liberta toda a memória ocupada pela tabela e todos os recursos
 * e outros recursos usados pelo skeleton.
 * Retorna 0 (OK) ou -1 em caso de erro. */
int server_skeleton_destroy(struct table_t *table){

	if (!table ) return -1;
	table_destroy(table);
	return 0;
}

/* Executa na tabela table a operação indicada pelo opcode contido em msg
 * e utiliza a mesma estrutura MessageT para devolver o resultado. * Retorna 0 (OK) ou -1 em caso de erro. */
int invoke(struct MessageT *msg, struct table_t *table){

	if (!msg  || !table ) return -1;
}
