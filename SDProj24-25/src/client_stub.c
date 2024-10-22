#include "../include/client_stub-private.h"
#include "../include/htmessages.pb-c.h"
#include "../include/client_network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

/* Função para estabelecer uma associação entre o cliente e o servidor,
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna a estrutura rtable preenchida, ou NULL em caso de erro.
 */
struct rtable_t *rtable_connect(char *address_port) {


	// Aloca memoria para rtable
	struct rtable_t *rtable = (struct rtable_t *) malloc(sizeof(struct rtable_t));
	if (!rtable) return NULL;

	// Parse the server address and port
	char *address = strtok(address_port, ":");
	char *port_str = strtok(NULL, ":");
	if (!address || !port_str ) {
		free(rtable);
		return NULL;
	}

	// Definir ip e porto na rtabçe
	rtable->server_address = strdup(address);
	rtable->server_port = atoi(port_str);

	printf("Conectar ao server IP: %s, Port: %d\n", rtable->server_address, rtable->server_port);


	// Cria socket
	rtable->sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (rtable->sockfd < 0) {
		printf("Falha ao conecar ao criar a socket.");
		free(rtable->server_address);
		free(rtable);
		return NULL;
	}

	if(network_connect(rtable) == -1) {
		printf("Falha ao conectar ao servidor.");
	}

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

	// Criar menssagem ainda nao serializada
	MessageT msg = MESSAGE_T__INIT;
	msg.opcode = MESSAGE_T__OPCODE__OP_PUT;  // Codigos de commando
	msg.c_type = MESSAGE_T__C_TYPE__CT_ENTRY; // Codigos de tipo de informacao

	//Criar nova EntryT apartir da entry_t
	EntryT newEntry = {
			PROTOBUF_C_MESSAGE_INIT(&entry_t__descriptor),
			entry -> key,
			{entry->value->datasize, block_duplicate(entry->value->data)}
	};


	entry_t__init(&newEntry);
	msg.entry = &newEntry;

	entry_destroy(entry);

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

	//Por nobuffer
	message_t__pack(&msg, packed_msg);

	//Enviar e receber
	if (network_send_receive(rtable, &msg) < 0) {
		entry_destroy(msg.entry);
		return -1;
	}

	entry_destroy(msg.entry);
	return 0;

}

/* Retorna a entrada da tabela com chave key, ou NULL caso não exista
 * ou se ocorrer algum erro.
 */
struct block_t *rtable_get(struct rtable_t *rtable, char *key){

	if (!rtable || !key) {
		return -1;
	}

	// Criar menssagem ainda nao serializada
	MessageT msg = MESSAGE_T__INIT;
	msg.opcode = MESSAGE_T__OPCODE__OP_GET;  // Codigos de commando
	msg.c_type = MESSAGE_T__C_TYPE__CT_KEY;  // Codigos de tipo de informacao
	msg.key = key;

	MessageT *response = network_send_receive(rtable->sockfd, &msg);
	if (!response || response->opcode == MESSAGE_T__OPCODE__OP_BAD) {
		return NULL;
	}

	if (response->c_type == MESSAGE_T__C_TYPE__CT_ENTRY) { // verificar se realmente recebeu a entrada
		struct entry_t *entry = entry_duplicate(response->entry);
		return entry;
	}

	return NULL;
}

/* Função para remover um elemento da tabela. Vai libertar
 * toda a memoria alocada na respetiva operação rtable_put().
 * Retorna 0 (OK), ou -1 (chave não encontrada ou erro).
 */
int rtable_del(struct rtable_t *rtable, char *key){

	if (!rtable  || !key == NULL) {
		return -1;
	}

	// Criar menssagem ainda nao serializada
	MessageT msg = MESSAGE_T__INIT;
	msg.opcode = MESSAGE_T__OPCODE__OP_DEL;  // Codigos de commando
	msg.c_type = MESSAGE_T__C_TYPE__CT_KEY;  // Codigos de tipo de informacao
	msg.key = key;

	if (network_send_receive(rtable->sockfd, &msg) < 0) {
		return -1;
	}

	return 0;

}

/* Retorna o número de elementos contidos na tabela ou -1 em caso de erro.
 */
int rtable_size(struct rtable_t *rtable) {

	if (rtable == NULL) return -1;

	// Criar menssagem ainda nao serializada
	MessageT msg = MESSAGE_T__INIT;
	msg.opcode = MESSAGE_T__OPCODE__OP_SIZE;

	// Recebe tamanho
	MessageT *response = network_send_receive(rtable->sockfd, &msg);
	if (!response || response->opcode == MESSAGE_T__OPCODE__OP_BAD) {
		return -1;
	}

	return response->n_entries; // TODO isto e o numero de entradas ?? talvez
}

/* Retorna um array de char* com a cópia de todas as keys da tabela,
 * colocando um último elemento do array a NULL.
 * Retorna NULL em caso de erro.
 */
char **rtable_get_keys(struct rtable_t *rtable) {

	if (rtable == NULL) return NULL;

	// Criar menssagem ainda nao serializada
	MessageT msg = MESSAGE_T__INIT;
	msg.opcode = MESSAGE_T__OPCODE__OP_GETKEYS;

	// Receber as keys
	MessageT *response = network_send_receive(rtable->sockfd, &msg);
	if (!response || response->opcode == MESSAGE_T__OPCODE__OP_BAD) {
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

	if (!keys ) return;

	list_free_keys(keys); //TODO utilizamos os metodos do modulo list?

}

/* Retorna um array de entry_t* com todo o conteúdo da tabela, colocando
 * um último elemento do array a NULL. Retorna NULL em caso de erro.
 */
struct entry_t **rtable_get_table(struct rtable_t *rtable) {

	if (!rtable ) return NULL;

	// Create a new message
	MessageT msg = MESSAGE_T__INIT;
	msg.opcode = MESSAGE_T__OPCODE__OP_GETTABLE;

	// Receber tabela
	MessageT *response = network_send_receive(rtable->sockfd, &msg);
	if (!response || response->opcode == MESSAGE_T__OPCODE__OP_BAD) {
		return NULL;
	}

	// Aloca memoria para
	struct entry_t **entries = malloc(sizeof(struct entry_t *) * response->n_entries);
	for (size_t i = 0; i < response->n_entries; i++) {
		entries[i] = entry_duplicate(response->entries[i]);
	}

	return entries;
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


