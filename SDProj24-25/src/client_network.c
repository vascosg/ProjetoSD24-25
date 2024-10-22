#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include "../include/message-private.h"
#include "../include/client_network.h"
#include "../include/client_stub-private.h"

/* Esta função deve:
 * - Obter o endereço do servidor (struct sockaddr_in) com base na
 *   informação guardada na estrutura rtable;
 * - Estabelecer a ligação com o servidor;
 * - Guardar toda a informação necessária (e.g., descritor do socket)
 *   na estrutura rtable;
 * - Retornar 0 (OK) ou -1 (erro).
 */
int network_connect(struct rtable_t *rtable) {

	if (!rtable || !rtable->server_address || rtable->server_port <= 0) {
		return -1; // Verifica se os parâmetros são válidos
	}

	// Configura o endereço do servidor
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(rtable->server_port);

	// Converte o endereço IP de string para binário
	if (inet_pton(AF_INET, rtable->server_address, &server.sin_addr) <= 0) {
		perror("Erro ao converter endereço IP");
		close(rtable->sockfd);
		return -1;
	}

	// Tenta conectar ao servidor
	if (connect(rtable->sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("Erro ao conectar ao servidor");
		close(rtable->sockfd);
		return -1;
	}

	if (rtable->sockfd < 0) {
		    perror("Socket inválido");
		    return NULL;
	}

	return 0; // Conexão estabelecida com sucesso
}

/* Esta função deve:
 * - Obter o descritor da ligação (socket) da estrutura rtable_t;
 * - Serializar a mensagem contida em msg;
 * - Enviar a mensagem serializada para o servidor;
 * - Esperar a resposta do servidor;
 * - De-serializar a mensagem de resposta;
 * - Tratar de forma apropriada erros de comunicação;
 * - Retornar a mensagem de-serializada ou NULL em caso de erro.
 */
MessageT *network_send_receive(struct rtable_t *rtable, MessageT *msg) {
	if (!rtable || !msg) return NULL;

	// 1. Serializar a mensagem
	unsigned int len = message_t__get_packed_size(msg); // Tamanho da mensagem serializada
	void* buf = malloc(len);
	if (!len) return NULL;

	message_t__pack(msg, buf); // Serializa a mensagem para o buffer

	// 2. Enviar o tamanho da mensagem (2 bytes - short)

	int net_msg_size = htons(len); // Converte para network byte order
	if (write_all(rtable->sockfd, &net_msg_size, sizeof(net_msg_size)) != sizeof(net_msg_size)) {
		perror("Erro ao enviar o tamanho da mensagem");
		free(buf);
		return NULL;
	}

	// 3. Enviar a mensagem serializada
	if (write_all(rtable->sockfd, buf, len) != len) {
		perror("Erro ao enviar a mensagem");
		free(buf);
		return NULL;
	}

	free(buf); // Liberta o buffer após o envio

	// 4. Receber o tamanho da resposta (2 bytes - short)
	int net_response_size;
	if (read_all(rtable->sockfd, &net_response_size, sizeof(net_response_size)) != sizeof(net_response_size)) {
		perror("Erro ao receber o tamanho da resposta");
		return NULL;
	}
	uint16_t response_size = ntohs(net_response_size); // Converte para host byte order

	// 5. Receber a resposta serializada
	uint8_t *response_buffer = malloc(response_size);
	if (!response_buffer) {
		perror("Erro ao alocar memória para a resposta");
		return NULL;
	}

	if (read_all(rtable->sockfd, response_buffer, response_size) != response_size) {
		perror("Erro ao receber a resposta");
		free(response_buffer);
		return NULL;
	}

	// 6. Deserializar a resposta
	MessageT *response_msg = message_t__unpack(NULL, response_size, response_buffer);
	free(response_buffer); // Liberta o buffer após a deserialização

	if (!response_msg) {
		perror("Erro ao deserializar a resposta");
		return NULL;
	}

	// 7. Retorna a mensagem de resposta
	return response_msg;
}

/* Fecha a ligação estabelecida por network_connect().
 * Retorna 0 (OK) ou -1 (erro).
 */
int network_close(struct rtable_t *rtable) {
	if (!rtable) return -1;

	if (close(rtable->sockfd) < 0) {
		perror("Erro ao fechar o socket");
		return -1;
	}

	return 0;
}


