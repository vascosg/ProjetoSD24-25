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
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include "../include/server_skeleton.h"
#include "../include/message-private.h"
#include "../include/table.h"
#include "../include/htmessages.pb-c.h"
#include "../include/server_network.h"

#define BUFFER_SIZE 1024

/* Função para preparar um socket de receção de pedidos de ligação
 * num determinado porto.
 * Retorna o descritor do socket ou -1 em caso de erro.
 */
int server_network_init(short port){

	int server_socket;
	struct sockaddr_in server_addr;

	// Criação do socket
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0) {
		//perror("Erro ao criar o socket\n");
		return -1;
	}

	// Configuração do endereço do servidor
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	// Vincula o socket ao endereço e porta especificados
	if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		//perror("Erro ao fazer bind no socket\n");
		close(server_socket);
		return -1;
	}

	// Coloca o socket em modo de escuta para aceitar conexões
	if (listen(server_socket, 0) < 0) {
		//perror("Erro ao escutar no socket");
		close(server_socket);
		return -1;
	}

	return server_socket;
}

/* A função network_main_loop() deve:
 * - Aceitar uma conexão de um cliente;
 * - Receber uma mensagem usando a função network_receive;
 * - Entregar a mensagem de-serializada ao skeleton para ser processada na tabela table;
 * - Esperar a resposta do skeleton;
 * - Enviar a resposta ao cliente usando a função network_send.
 * A função não deve retornar, a menos que ocorra algum erro. Nesse
 * caso retorna -1.
 */
int network_main_loop(int listening_socket, struct table_t *table){ // a listening_socket tem de ser preparada para a receção de pedidos de múltiplas ligações num determinado porto (determinado pelo método listen)
	int client_socket;
	struct sockaddr_in client_addr;
	socklen_t client_len= sizeof(client_addr);


	while ((client_socket = accept(listening_socket,(struct sockaddr *) &client_addr, &client_len)) != -1) {

		printf("Client connection established\n");

		while(1) {
			// Recebe uma mensagem do cliente
			struct MessageT *request = network_receive(client_socket);
			if (!request) {

				if (errno == 0) break;
				//perror("Erro ao receber mensagem\n");
				message_t__free_unpacked(request, NULL);
				//close(client_socket);
				//return -1;
			}

			// começar a contar o tempo

			// Processa a mensagem com a tabela e o skeleton (implementação depende do contexto)
			invoke(request, table);  // Supõe que esta função exista
			// Verifica se ocorreu um erro

			// acabar de contar o tempo e mudar as stats

			// Envia a resposta ao cliente
			network_send(client_socket, request);

			message_t__free_unpacked(request, NULL);
		}

		close(client_socket);
		printf("Client connection closed\n");
	}

	return 0;
}

/* A função network_receive() deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 * reservando a memória necessária para a estrutura MessageT.
 * Retorna a mensagem com o pedido ou NULL em caso de erro.
 */
struct MessageT *network_receive(int client_socket){

	// 1. Receber o tamanho da resposta (2 bytes - short)
	short net_response_size;
	if (read_all(client_socket, &net_response_size, sizeof(net_response_size)) != sizeof(net_response_size)) {
		//perror("Erro ao receber o tamanho da resposta\n");
		return NULL;
	}
	uint16_t response_size = ntohs(net_response_size); // Converte para host byte order

	// 2. Receber a resposta serializada
	uint8_t *response_buffer = malloc(response_size);
	if (!response_buffer) {
		//perror("Erro ao alocar memória para a resposta\n");
		return NULL;
	}

	if (read_all(client_socket, response_buffer, response_size) != response_size) {
		//perror("Erro ao receber a resposta\n");
		free(response_buffer);
		return NULL;
	}

	// 3. Deserializar a resposta
	struct MessageT *response_msg = message_t__unpack(NULL, response_size, response_buffer);
	free(response_buffer); // Liberta o buffer após a deserialização

	if (!response_msg) {
		//perror("Erro ao deserializar a resposta\n");
		return NULL;
	}

	// 4. Retorna a mensagem de resposta
	return response_msg;
}

/* A função network_send() deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Enviar a mensagem serializada, através do client_socket.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int network_send(int client_socket, struct MessageT *msg){

	if (client_socket < 0 || !msg) return -1;

	// 1. Serializar a mensagem
	unsigned int len = message_t__get_packed_size(msg); // Tamanho da mensagem serializada
	if (!len) return -1;
	void* buf = malloc(len);
	if (!buf) {  // Verifica se a alocação foi bem sucedida
		//perror("Erro ao alocar memória para a mensagem");
		return -1;
	}

	int msg_len = message_t__pack(msg, buf); // Serializa a mensagem para o buffer
	if (msg_len != len) { // verifica se o tamanho da mensagem serializada é o esperado
		//perror("Erro ao serializar a mensagem\n");
		free(buf);
		return -1;
	}

	// 2. Enviar o tamanho da mensagem (2 bytes - short)

	short net_msg_size = htons(len); // Converte para network byte order

	if (write_all(client_socket, &net_msg_size, sizeof(net_msg_size)) != sizeof(net_msg_size)) {
		//perror("Erro ao enviar o tamanho da mensagem\n");
		free(buf);
		return -1;
	}

	// 3. Enviar a mensagem serializada
	if (write_all(client_socket, buf, len) != len) {
		//perror("Erro ao enviar a mensagem\n");
		free(buf);
		return -1;
	}

	free(buf); // Liberta o buffer após o envio
	return 0;
}

/* Liberta os recursos alocados por server_network_init(), nomeadamente
 * fechando o socket passado como argumento.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int server_network_close(int socket){

	if (close(socket) < 0) {
		//perror("Erro ao fechar o socket\n");
		return -1;
	}
	return 0;
}
