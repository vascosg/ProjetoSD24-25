#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include "../include/message-private.h"
#include "../include/htmessages.pb-c.h"
#include "../include/client_network.h"
#include "../include/server_network.h"
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
		//perror("Erro ao converter endereço IP\n");
		close(rtable->sockfd);
		return -1;
	}

	// verifica se o socket é válido
	if (rtable->sockfd < 0) {
		//perror("Socket inválido\n");
		return -1;
	}

	// Tenta conectar ao servidor
	if (connect(rtable->sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
		//perror("Erro ao conectar ao servidor\n");
		close(rtable->sockfd);
		return -1;
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
struct MessageT *network_send_receive(struct rtable_t *rtable, struct MessageT *msg) {
	if (!rtable || !msg) return NULL;

	// Serializa a mensagem
	int code = network_send(rtable->sockfd, msg);

	if (code < 0) {
		//perror("Erro ao enviar a mensagem\n");
		return NULL;
	}

	// Recebe a mensagem de resposta
	struct MessageT *response = network_receive(rtable->sockfd);
	if (!response) {
		//perror("Erro ao receber a mensagem\n");
		return NULL;
	}

	return response;
}

/* Fecha a ligação estabelecida por network_connect().
 * Retorna 0 (OK) ou -1 (erro).
 */
int network_close(struct rtable_t *rtable) {
	if (!rtable) return -1;

	if (close(rtable->sockfd) < 0) {
		//perror("Erro ao fechar o socket\n");
		return -1;
	}

	return 0;
}
