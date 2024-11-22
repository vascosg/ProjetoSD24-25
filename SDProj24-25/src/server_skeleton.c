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
#include "../include/list.h"
#include "../include/entry.h"
#include "../include/server_skeleton.h"
#include "../include/table.h"
#include "../include/htmessages.pb-c.h"
#include "../include/stats.h"

/* Inicia o skeleton da tabela. 
 * O main() do servidor deve chamar esta função antes de poder usar a 
 * função invoke(). O parâmetro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Retorna a tabela criada ou NULL em caso de erro. 
 */
struct table_t *server_skeleton_init(int n_lists){

	struct table_t *table = table_create(n_lists);
	if (!table) {
		//fprintf(stderr, "Erro ao criar a tabela.\n");
		return NULL;
	}
	return table;
}

/* Liberta toda a memória ocupada pela tabela e todos os recursos
 * e outros recursos usados pelo skeleton.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int server_skeleton_destroy(struct table_t *table){

	if (!table) return -1;
	table_destroy(table);
	return 0;
}

/* Executa na tabela table a operação indicada pelo opcode contido em msg
 * e utiliza a mesma estrutura MessageT para devolver o resultado.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int invoke(struct MessageT *msg, struct table_t *table){		// incluir trataemnto da OP_STATS

	if (!msg  || !table) return -1;

	if(msg->opcode == MESSAGE_T__OPCODE__OP_PUT) { // fazer o put

		// Criar o bloco a partir da info na msg
		struct block_t *bloco = block_create(msg->entry->value.len, msg->entry->value.data);
		if (!bloco) {
			//printf("Erro ao criar o bloco no put\n");
			msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
			msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
			return -1;
		}

		// Colocar a entrada na tabela
		int result = table_put(table, msg->entry->key, bloco);
		if (result < 0) {
			//printf("Erro ao fazer o put na tabela\n");
			msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
			msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
			return -1;
		}

		//Se tudo deu certo
		msg->opcode = MESSAGE_T__OPCODE__OP_PUT+1;
		msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
		//printf("Put realizado com sucesso!\n");

	} else if (msg->opcode == MESSAGE_T__OPCODE__OP_GET) {

		// Obter entrada da tabela
		struct block_t* block = table_get(table, msg->key);
		if(!block) {
			//printf("Erro ao obter a entrada da tabela\n");
			msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
			msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
			return -1;
		}

		msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
		msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;

		// Limpar o campo da key
		if (msg->key != NULL && msg->key != protobuf_c_empty_string) {
			free(msg->key);
			msg->key = NULL;  // Definir para NULL para indicar que não será usado
		}
		msg->value.len = block->datasize;
		msg->value.data = (uint8_t *)block->data;
		//printf("Get realizado com sucesso!\n");

	} else if (msg->opcode == MESSAGE_T__OPCODE__OP_DEL) {

		if(table_get(table, msg->key) == NULL) {
			//printf("Erro ao obter a entrada da tabela no del\n");
			msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
			msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
			return -1;
		}

		// Remove a entrada da tabela
		int result = table_remove(table, msg->key);
		if (result != 0) {
			//printf("Erro ao eliminar entrada na tabela\n");
			msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
			msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
			return -1;
		}

		// Limpar o campo da key
		if (msg->key != NULL && msg->key != protobuf_c_empty_string) {
			free(msg->key);
			msg->key = NULL;  // Definir para NULL para indicar que não será usado
		}

		msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
		msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

		//printf("Del realizado com sucesso!\n");

	} else if (msg->opcode == MESSAGE_T__OPCODE__OP_SIZE) {

		//Obtem o tamanho da tabela
		int size = table_size(table);
		if(size < 0 ) {
			//printf("Erro ao eliminar entrada na tabela\n");
			msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
			msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
			return -1;
		}

		msg->result = (int32_t)size;
		msg->opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
		msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;

		//printf("Size realizado com sucesso!\n");

	} else if (msg->opcode == MESSAGE_T__OPCODE__OP_GETKEYS) {

		// Obtem chaves da tabela
		char ** keys = table_get_keys(table);
		if (!keys) {
			//printf("Erro ao obter as chaves da tabela\n");
			msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
			msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
			return -1;
		}

		msg->n_keys = (size_t) table_size(table); //TODO mudar no cliente porque lá ele n usa esta valor para saber o numero de keys acho
		msg->keys = keys; // TODO Libertar estas keys ? onde ? alocar espaço para elas
		msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
		msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;

		//printf("GetKeys realizado com sucesso!\n");

	} else if (msg->opcode == MESSAGE_T__OPCODE__OP_GETTABLE) {

		// Obter todas as chaves da tabela
		char **keys = table_get_keys(table);
		if (!keys) {
			//printf("Erro ao obter as chaves da tabela no getTable\n");
			msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
			msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
			return -1;
		}

		// Determinar quantas chaves foram obtidas
		size_t num_keys = table_size(table);
		msg->n_entries = (size_t)num_keys;
		msg->entries = malloc(num_keys * sizeof(EntryT*)); //TODO Limpar isto

		if (!msg->entries) {
			//printf("Erro ao alocar espaço para entradas\n");
			msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
			msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
			list_free_keys(keys);
			return -1;
		}

		for (size_t i = 0; i < num_keys; i++) {
			// Obter o bloco correspondente à chave atual
			struct block_t *block = table_get(table, keys[i]);
			if (!block) {
				//printf("Erro ao obter o bloco para a chave %s\n", keys[i]);
				list_free_keys(keys);
				return -1;
			}

			// Criar a entrada com a chave e o bloco
			struct entry_t *entry = entry_create(keys[i], block);
			if (!entry) {
				//printf("Erro ao criar a entrada para a chave %s\n", keys[i]);
				list_free_keys(keys);
				free(msg->entries); // Liberta entradas alocadas antes de retornar
				msg->entries = NULL; // Definir para NULL para indicar que não será usado
				return -1;
			}

			EntryT *newEntry = malloc(sizeof(EntryT)); // Alocar espaço para a nova entrada
			if (!newEntry) { // Verificar se a alocação foi bem sucedida
				//printf("Erro ao alocar memória para a nova entrada\n");
				list_free_keys(keys);
				free(msg->entries); // Libertar entradas alocadas antes de retornar
				msg->entries = NULL; // Definir para NULL para indicar que não será usado
				return -1;
			}

			entry_t__init(newEntry); // Inicializar a nova entrada
			newEntry->key = strdup(entry->key); // Copiar a chave para a nova entrada
			if (!newEntry->key) {
				//printf("Erro ao duplicar a chave para a nova entrada\n");
				list_free_keys(keys);
				free(msg->entries); // Libertar entradas alocadas antes de retornar
				msg->entries = NULL; // Definir para NULL para indicar que não será usado
				return -1;
			}
			newEntry->value.len = entry->value->datasize; // Definir o tamanho do valor
			newEntry->value.data = (uint8_t *)entry->value->data;
			// Adicionar a entrada à mensagem
			msg->entries[i] = newEntry;
		}

		msg->opcode = MESSAGE_T__OPCODE__OP_GETTABLE + 1;
		msg->c_type = MESSAGE_T__C_TYPE__CT_TABLE;
		//printf("GetTable realizado com sucesso!\n");
		list_free_keys(keys);

	} else if (msg->opcode == MESSAGE_T__OPCODE__OP_STATS) { //TODO ACABARRRR


	}

	return 0;
};
