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
#include <time.h>
#include <pthread.h>
#include <zookeeper/zookeeper.h>
#include "../include/client_stub.h"
#include "../include/server_skeleton.h"
#include "../include/message-private.h"
#include "../include/table.h"
#include "../include/htmessages.pb-c.h"
#include "../include/server_network.h"
#include "../include/stats.h"

#define BUFFER_SIZE 1024

pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;	// mutex para as estatisticas

// Variáveis globais do ZooKeeper
static zhandle_t *zh;				// handler do ZooKeeper
static char znode_path[1024];		// caminho do znode
static char sucessor_path[256];	    // caminho do sucessor
struct rtable_t* sucessor_server;	// tabela de roteamento
static int server_pos = -1; 		// id do servidor
int is_connected = 0;


static int compare_strings(const void *a, const void *b) {
    const char *str_a = *(const char **)a;
    const char *str_b = *(const char **)b;
    return strcmp(str_a, str_b);
}

void is_linked(zhandle_t *zh, int type, int state, const char *path, void *context) {
	if (type == ZOO_SESSION_EVENT) {
		if(state == ZOO_CONNECTED_STATE) {
			is_connected = 1;
		} else {
			is_connected = 0;
		}
	}
}

// Callback do watcher
static void my_watcher_fn(zhandle_t *zzh, int type, int state, const char *path, void *watcherCtx) {
    if (state == ZOO_CONNECTED_STATE){
		if (type == ZOO_CHILD_EVENT) {
			struct String_vector children;
			//Obter lista atualizada dos filhos
    		int ret = zoo_wget_children(zh, "/chain", my_watcher_fn, "Zookeeper Data Watcher", &children);
			if (ret != ZOK) {
				fprintf(stderr, "Erro ao obter filhos de /chain: %s\n", zerror(ret));
				deallocate_String_vector(&children);
				return;
			}
            //Numero incoerente de filhos
			if (children.count <= 0){
				fprintf(stderr, "Erro ao obter filhos de /chain\n");
				deallocate_String_vector(&children);
				return;
			}

			// Ordenar os filhos
			qsort(children.data, children.count, sizeof(char *), compare_strings);
			// Descobrir a posição deste servidor no zk
			for (int i = 0; i < children.count; i++) {
				char aux_path[256];
				snprintf(aux_path, sizeof(aux_path), "/chain/%s", children.data[i]);
				if (strcmp(znode_path, aux_path) == 0) {
					server_pos = i;
					break;
				}
			}

			// Identificar o sucessor
			if(server_pos + 1 < children.count){ // Se houver proximo servidor na lista
				char aux_path[256];
				snprintf(aux_path, sizeof(aux_path), "/chain/%s", children.data[server_pos + 1]);

				if(sucessor_server == NULL || strcmp(sucessor_path, aux_path) != 0){ // verifica se o servidor mudou
					if (sucessor_path[0] =='\0') {
						rtable_disconnect(sucessor_server); // Desconecta o servidor anterior
					}
					snprintf(sucessor_path, sizeof(sucessor_path), "/chain/%s", children.data[server_pos + 1]);

					char successor_data[1024];
					int len = sizeof(successor_data);
					int ret = zoo_get(zh, sucessor_path, 0, successor_data, &len, NULL); // obtem dados endereco e porta do novo sucessor
					if (ret != ZOK) {
						fprintf(stderr, "Erro ao obter dados do sucessor: %s\n", zerror(ret));
						fprintf(stderr, "Successor path: %s\n", sucessor_path);
					} else {
						// Null-terminate the data
						if (len < sizeof(successor_data)) {
							successor_data[len] = '\0';
						} else {
							successor_data[sizeof(successor_data) - 1] = '\0';
						}

						connect_to_next_server(successor_data); // conecta ao novo server
					}
				}
				
			}else{
				// Se nao houver sussesor, for cauda
				sucessor_server = NULL;
				sucessor_path[strlen(sucessor_path)] = '\0';
			}
			deallocate_String_vector(&children);
		}
	}
}

// Função para garantir que o nó /chain existe
void ensure_chain_node_exists() {

    int ret = zoo_exists(zh, "/chain", 0, NULL);

    if (ret == ZNONODE) {
        // Creates the /chain node if it does not exist
        ret = zoo_create(zh, "/chain", NULL, -1, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0);
        if (ret != ZOK) {
            fprintf(stderr, "Erro ao criar o nó /chain: %s\n", zerror(ret));
            exit(EXIT_FAILURE);
        }

    } else if (ret != ZOK) {
        fprintf(stderr, "Erro ao verificar a existência do nó /chain: %s\n", zerror(ret));
        exit(EXIT_FAILURE);
    }
}

// Função para conectar ao ZooKeeper
static void connect_to_zookeeper(char* zk_port, short port) {

	zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);

    zh = zookeeper_init(zk_port, is_linked, 2000, 0, NULL, 0);
    if (!zh) {
        fprintf(stderr, "Erro ao conectar ao ZooKeeper\n");
        exit(EXIT_FAILURE);
    }

	printf("Connected to ZooKeeper in: %s\n", zk_port);

    // Garante que o nó /chain existe
    ensure_chain_node_exists();

	if(is_connected == 1){
		// Cria o ZNode com o IP e porta atuais
		char server_data[64];
		snprintf(server_data, sizeof(server_data), "127.0.0.1:%d", port); // 127.0.0.1:<port> string armazenada no zk
		// Criar ZNode efémero sequencial
		// ZOO_EPHEMERAL garante que é removido quando a conexão falha
		// ZOO_SEQUENCE adiciona nomerador ex: /chain/node0000000001
		int ret = zoo_create(zh, "/chain/node", server_data, strlen(server_data), &ZOO_OPEN_ACL_UNSAFE,
							ZOO_EPHEMERAL | ZOO_SEQUENCE, znode_path, sizeof(znode_path));
		if (ret != ZOK) {
			fprintf(stderr, "Erro ao criar ZNode: %s\n", zerror(ret));
			exit(EXIT_FAILURE);
		}
	}
}

// Adds watcher and position
void set_server() {
    struct String_vector children;
    int ret = zoo_wget_children(zh, "/chain", my_watcher_fn, "Zookeeper Data Watcher", &children);
    if (ret != ZOK) {
        fprintf(stderr, "Erro ao obter filhos de /chain: %s\n", zerror(ret));
        return;
    }

    // Ordenar os filhos
    qsort(children.data, children.count, sizeof(char *), compare_strings);
    //Identificar posicao atual na cadeia (ordem lexicográfica)
    for (int i = 0; i < children.count; i++) {
		char aux_path[256];
		snprintf(aux_path, sizeof(aux_path), "/chain/%s", children.data[i]);
        if (strcmp(znode_path, aux_path) == 0) {
            server_pos = i;
			break;
        }
    }

	sucessor_server = NULL;
	sucessor_path[strlen(sucessor_path)] = '\0';

    deallocate_String_vector(&children);
}

// conecta ao servidor seguinte
void connect_to_next_server(char *server_data) {
    
	sucessor_server = rtable_connect(server_data);

	if (!sucessor_server) {
		fprintf(stderr, "Erro ao conectar ao servidor seguinte\n");
		return;
	}
	printf("Connected to the next server at %s\n", server_data);

	return;

}

int set_table(struct table_t* table){
	
	if(server_pos > 0){
		struct String_vector children;
		int ret = zoo_wget_children(zh, "/chain", my_watcher_fn, "Zookeeper Data Watcher", &children);
		if(ret != ZOK){
			fprintf(stderr, "Erro ao obter filhos de /chain: %s\n", zerror(ret));
			return -1;
		}
		char prev_path[256];
		snprintf(prev_path, sizeof(prev_path), "/chain/%s", children.data[server_pos - 1]);

		char prev_server_ip[1024];
		int len = sizeof(prev_server_ip);
		ret = zoo_get(zh, prev_path, 0, prev_server_ip, &len, NULL);
		if (ret != ZOK) {
			fprintf(stderr, "Erro ao obter dados do servidor anterior: %s\n", zerror(ret));
			fprintf(stderr, "Previous path: %s\n", prev_path);
			return -1;
		}
		struct rtable_t* prev_server = rtable_connect(prev_server_ip);

		if(prev_server == NULL){
			fprintf(stderr, "Erro ao conectar ao servidor anterior\n");
			return -1;
		}

		struct entry_t** entries = rtable_get_table(prev_server);
		int n_entries = rtable_size(prev_server);
		for (size_t i = 0; i < n_entries; i++) {
			if (entries[i] != NULL) {
				int ret = table_put(table, entries[i]->key, entries[i]->value);
				if (ret != 0) {
					fprintf(stderr, "Erro ao inserir entrada na tabela\n");
					return -1;
				}
			}
		}
		//rtable_free_entries(entries);
		rtable_disconnect(prev_server);
	}
	return 0;
}

void *client_handler(void *args){

	struct client_thread_args *thread_args = (struct client_thread_args *)args;
	int client_socket = thread_args->client_socket;
	struct table_t *table = thread_args->table;
	struct statistics_t *stats = thread_args->stats;

	clock_t inicio_tempo, fim_tempo;
	double duracao;

	pthread_mutex_lock(&stats_mutex);
	stats->n_clients++;
	pthread_mutex_unlock(&stats_mutex);

	while(1) {
		// Recebe uma mensagem do cliente
		MessageT *request = network_receive(client_socket);
		if (!request) {

			if (errno == 0) break;
			//perror("Erro ao receber mensagem\n");
			message_t__free_unpacked(request, NULL);
			//close(client_socket);
			//return -1;
		}

		//Tratar o statsaqui para todas as threads poderem ter acesso aos valores de stats
		if(request->opcode == MESSAGE_T__OPCODE__OP_STATS) {

			if (!stats) { // Se houver problema no stat do server
				request->opcode = MESSAGE_T__OPCODE__OP_ERROR;
				request->c_type = MESSAGE_T__C_TYPE__CT_NONE;

			} else { // Formatar a menssagem para enviar com a resposa a stats
				request->opcode = MESSAGE_T__OPCODE__OP_STATS + 1;
				request->c_type = MESSAGE_T__C_TYPE__CT_STATS;
				pthread_mutex_lock(&stats_mutex);
				request->ops = stats->n_ops;
				request->duration = stats->time_spent;
				request->clients = stats->n_clients;
				pthread_mutex_unlock(&stats_mutex);
			}
		} else {

			inicio_tempo = clock(); // mudar para gettimeofday
			if(invoke(request, table) == -1){
				printf("Erro ao executar a operação\n");
			}
			// Verifica se ocorreu um erro
			if (sucessor_server != NULL) {
				
				if(request->opcode == MESSAGE_T__OPCODE__OP_PUT+1){

					struct block_t *bloco = block_create(request->entry->value.len, request->entry->value.data);
					char *key_copy = strdup(request->entry->key);
					struct entry_t *entry = entry_create(key_copy, bloco);
					
					rtable_put(sucessor_server, entry);
				}
				else if(request-> opcode == MESSAGE_T__OPCODE__OP_DEL+1){
					rtable_del(sucessor_server, request -> key);
				}
				
			}
			fim_tempo = clock();

			duracao = (double) fim_tempo - inicio_tempo;
			pthread_mutex_lock(&stats_mutex);
			stats->time_spent += duracao;
			stats->n_ops++;
			pthread_mutex_unlock(&stats_mutex);
		}

		// Envia a resposta ao cliente
		network_send(client_socket, request);
		message_t__free_unpacked(request, NULL);
	}

	close(client_socket);

	pthread_mutex_lock(&stats_mutex);
	stats->n_clients--;
	pthread_mutex_unlock(&stats_mutex);

	return NULL;
}

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

/* Função para preparar um socket de receção de pedidos de ligação
 * num determinado porto.
 * Retorna o descritor do socket ou -1 em caso de erro.
 */
int server_network_init(char* zk_port, short port){

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

	printf("Running server in port: %d.\n", port);

	connect_to_zookeeper(zk_port,port);

	set_server();

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
int network_main_loop(int listening_socket, struct table_t *table){

	struct sockaddr_in client_addr;
	socklen_t client_len= sizeof(client_addr);

	struct statistics_t *stats = statistics_create();

		while (1) {
			int client_socket = accept(listening_socket, (struct sockaddr *)&client_addr, &client_len);
			if (client_socket < 0) {
				perror("Erro ao aceitar conexão");
				continue;
			}

			// Aloca memória para os argumentos da thread
			struct client_thread_args *args = malloc(sizeof(struct client_thread_args));
			if (!args) {
				perror("Erro ao alocar memória para os argumentos da thread");
				close(client_socket);
				continue;
			}

			args->client_socket = client_socket;
			args->table = table;
			args->stats = stats;

			// Cria a thread
			pthread_t client_thread;
			if (pthread_create(&client_thread, NULL, &client_handler, (void *)args) != 0) {
				perror("Erro ao criar thread");
				close(client_socket);
				free(args);
				continue;
			}
			pthread_detach(client_thread);
		}

		return 0;

}
/* A função network_receive() deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 * reservando a memória necessária para a estrutura MessageT.
 * Retorna a mensagem com o pedido ou NULL em caso de erro.
 */
MessageT *network_receive(int client_socket){

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
	MessageT *response_msg = message_t__unpack(NULL, response_size, response_buffer);
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
int network_send(int client_socket, MessageT *msg){

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
	if(sucessor_server)
		rtable_disconnect(sucessor_server);
	
	zookeeper_close(zh);
	return 0;
}
