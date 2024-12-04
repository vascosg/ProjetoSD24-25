/* ------------------------------------
---------------  SD-054 ---------------
    Filipa Inácio       fc59788
    Tomás Canilhas      fc59794
    Vasco Baldé         fc58174
---------------------------------------
------------------------------------ */

#include "../include/client_stub-private.h"
#include "../include/htmessages.pb-c.h"
#include "../include/client_network.h"
#include "../include/block.h"
#include "../include/entry.h"
#include "../include/list.h"
#include "../include/stats.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <inttypes.h>

/* Função para estabelecer uma associação entre o cliente e o servidor,
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna a estrutura rtable preenchida, ou NULL em caso de erro.
 */
struct rtable_t *rtable_connect(char *address_port) {


	// Aloca memoria para rtable
	struct rtable_t *rtable = malloc(sizeof(struct rtable_t));
	if (!rtable) return NULL;

	// Parse the server address and port
	char *address = strtok(address_port, ":");
	char *port_str = strtok(NULL, ":");
	if (!address || !port_str ) {
		free(rtable);
		return NULL;
	}

	// Definir ip e porto na rtable
	rtable->server_address = strdup(address);
	rtable->server_port = atoi(port_str);




	// Cria socket
	rtable->sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (rtable->sockfd < 0) {
		printf("Falha ao conectar ao criar a socket.");
		free(rtable->server_address);
		free(rtable);
		return NULL;
	}

	if(network_connect(rtable) == -1) return NULL;

	return rtable;
}

/* Termina a associação entre o cliente e o servidor, fechando a
 * ligação com o servidor e libertando toda a memória local.
 * Retorna 0 se tudo correr bem, ou -1 em caso de erro.
 */
int rtable_disconnect(struct rtable_t *rtable) {

	if (!rtable) return -1;

	close(rtable->sockfd);
	free(rtable->server_address);
	free(rtable);
	return 0;
}

/* Função para adicionar uma entrada na tabela.
 * Se a key já existe, vai substituir essa entrada pelos novos dados.
 * Retorna 0 (OK, em adição/substituição), ou -1 (erro).
 */
int rtable_put(struct rtable_t *rtable, struct entry_t *entry) {

	if (!rtable || !entry ) {
		return -1;
	}

	// Criar mensagem ainda nao serializada
	MessageT msg = MESSAGE_T__INIT;
	msg.opcode = MESSAGE_T__OPCODE__OP_PUT;  // Codigos de commando
	msg.c_type = MESSAGE_T__C_TYPE__CT_ENTRY; // Codigos de tipo de informacao

	// Criar nova EntryT a partir da entry_t
	EntryT newEntry = ENTRY_T__INIT;
	newEntry.key = entry->key;
	newEntry.value.len = entry->value->datasize;
	newEntry.value.data = (uint8_t *)entry->value->data; // porque em entryT esta definido que "ProtobufCBinaryData value;"
	// em vez de ser block_t

	msg.entry = &newEntry;

	if (!msg.entry ) {
		fprintf(stderr, "msg.entry is NULL!\n");
		return -1; // Handle error case
	}

	// Empacotar menssagem
	unsigned int packed_size = message_t__get_packed_size(&msg);
	uint8_t *packed_msg = malloc(packed_size);
	if (!packed_msg) {
		return -1;
	}

	// Por no buffer
	message_t__pack(&msg, packed_msg);

	// Enviar e receber
	MessageT *received_msg = network_send_receive(rtable, &msg);
	if (!received_msg) {
		//fprintf(stderr, "Erro no network_send_receive\n");
		free(packed_msg);
		return -1;
	}


	message_t__free_unpacked(received_msg, NULL);
	free(packed_msg);


	return 0;

}

/* Retorna a entrada da tabela com chave key, ou NULL caso não exista
 * ou se ocorrer algum erro.
 */
struct block_t *rtable_get(struct rtable_t *rtable, char *key){

	if (!rtable || !key) {
		printf("rtable ou key e NULL\n");
		return NULL;
	}

	// Criar mensagem ainda nao serializada
	MessageT msg = MESSAGE_T__INIT;
	msg.opcode = MESSAGE_T__OPCODE__OP_GET;  // Codigos de comando
	msg.c_type = MESSAGE_T__C_TYPE__CT_KEY;  // Codigos de tipo de informacao
	msg.key = key;

	MessageT *response = network_send_receive(rtable, &msg);
	if (!response || response->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
		message_t__free_unpacked(response, NULL);
		//printf("Erro no network_send_receive\n");
		return NULL;
	}

	if (response->c_type == MESSAGE_T__C_TYPE__CT_VALUE) { // Verificar se realmente recebeu o block

		char *new_str = (char *)response->value.data;
		printf("%s\n", new_str);
		struct block_t *newBlock = block_create(response->value.len, response->value.data);

		message_t__free_unpacked(response, NULL);
		return newBlock;

	}

	return NULL;
}

/* Função para remover um elemento da tabela. Vai libertar
 * toda a memoria alocada na resp1etiva operação rtable_put().
 * Retorna 0 (OK), ou -1 (chave não encontrada ou erro).
 */
int rtable_del(struct rtable_t *rtable, char *key){

	if (!rtable  || !key ) {
		return -1;
	}

	// Criar mensagem ainda nao serializada
	MessageT msg = MESSAGE_T__INIT;
	msg.opcode = MESSAGE_T__OPCODE__OP_DEL;  // Codigos de commando
	msg.c_type = MESSAGE_T__C_TYPE__CT_KEY;  // Codigos de tipo de informacao
	msg.key = key;

	MessageT *response = network_send_receive(rtable, &msg);
	if (!response || response->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
		message_t__free_unpacked(response, NULL);
		return -1;
	}
	printf("Entry removed\n");
	return 0;

}

/* 
 * Retorna o número de elementos contidos na tabela ou -1 em caso de erro.
 */
int rtable_size(struct rtable_t *rtable) {

	if (!rtable) return -1;

	// Criar mensagem ainda nao serializada
	MessageT msg = MESSAGE_T__INIT;
	msg.opcode = MESSAGE_T__OPCODE__OP_SIZE;
	msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

	// Recebe mensagem
	MessageT *response = network_send_receive(rtable, &msg); // mensagem deserializada
	if (!response || response->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
		message_t__free_unpacked(response, NULL);
		return -1;
	}
	
	int result = response->result;

	message_t__free_unpacked(response, NULL);

	return result; 
}

/* Retorna um array de char* com a cópia de todas as keys da tabela,
 * colocando um último elemento do array a NULL.
 * Retorna NULL em caso de erro.
 */
char **rtable_get_keys(struct rtable_t *rtable) {

	if (!rtable) return NULL;

	// Criar mensagem ainda nao serializada
	MessageT msg = MESSAGE_T__INIT;
	msg.opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
	msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

	// Receber as keys
	MessageT *response = network_send_receive(rtable, &msg);
	if (!response || response->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
		message_t__free_unpacked(response, NULL);
		return NULL;
	}


	// Alocar memoria  para as keys
	char **keys = malloc(sizeof(char *) * response->n_keys);
	for (size_t i = 0; i < response->n_keys; i++) {
		keys[i] = strdup(response->keys[i]);
	}

	return keys;
}

/* Liberta a memória alocada por rtable_get_keys().
 */
void rtable_free_keys(char **keys) {

	list_free_keys(keys);

}

/* Retorna um array de entry_t* com todo o conteúdo da tabela, colocando
 * um último elemento do array a NULL. Retorna NULL em caso de erro.
 */
struct entry_t **rtable_get_table(struct rtable_t *rtable) {

	if (!rtable ) return NULL;

	// Cria nova mensagem
	MessageT msg = MESSAGE_T__INIT;
	msg.opcode = MESSAGE_T__OPCODE__OP_GETTABLE;
	msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

	// Recebe tabela
	MessageT *response = network_send_receive(rtable, &msg);
	if (!response || response->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
		return NULL;
	}

	size_t n = response->n_entries;
	EntryT **entries = response->entries;
	struct entry_t ** s_entries = malloc(sizeof(struct entry_t)*n);

	for (size_t i = 0; i < n; i++) {
		printf("%s :: %s\n",entries[i]->key,(char *) entries[i]->value.data);
		s_entries[i] = entry_create(entries[i]->key, block_create(entries[i]->value.len,entries[i]->value.data));
	}

	return s_entries; // Deveriam ser entry_t**
}

/* Liberta a memória alocada por rtable_get_table().
 */
void rtable_free_entries(struct entry_t **entries) {

	if (!entries ) return;

	for (int i = 0; entries[i] != NULL; i++) {
		entry_destroy(entries[i]);
	}

	free(entries);
}

/* Obtém as estatísticas do servidor.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
struct statistics_t *rtable_stats(struct rtable_t *rtable) {	// rever!!!!

	if (!rtable) return NULL;

	// Criar mensagem ainda nao serializada
	MessageT msg = MESSAGE_T__INIT;
	msg.opcode = MESSAGE_T__OPCODE__OP_STATS;
	msg.c_type = MESSAGE_T__C_TYPE__CT_STATS;

	// Envia e verifica se recebe tudo bem
	MessageT *response = network_send_receive(rtable, &msg);
	if (!response || response->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
		message_t__free_unpacked(response, NULL);
		return NULL;
	}

	struct statistics_t *statistic = statistics_create();
	if (!statistic) { 
		message_t__free_unpacked(response, NULL);
		return NULL; 
	}

	statistic->n_ops = response->ops;
	statistic->time_spent = response->duration;
	statistic->n_clients = response->clients;

	

    printf("Número de operações (n_ops): %d\n", statistic->n_ops);
    printf("Tempo total gasto (time_spent): %.2f segundos\n", statistic->time_spent);
    printf("Número de clientes conectados (n_clients): %d\n", statistic->n_clients);

	return statistic;
}
