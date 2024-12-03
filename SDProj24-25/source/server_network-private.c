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
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <pthread.h>
#include "../include/server_network-private.h"
#include "../include/server_skeleton.h"
#include "../include/message-private.h"
#include "../include/table.h"
#include "../include/htmessages.pb-c.h"
#include "../include/server_network.h"
#include "../include/stats.h"

pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;	// mutex para as estatisticas

void *client_handler(void *args){

	struct client_thread_args *thread_args = (struct client_thread_args *)args;
	int client_socket = thread_args->client_socket;
	struct table_t *table = thread_args->table;
	struct statistics_t *stats = thread_args->stats;

	clock_t inicio_tempo, fim_tempo;
	double duracao;

	printf("Client connection established\n");

	pthread_mutex_lock(&stats_mutex);
	stats->n_clients++;
	pthread_mutex_unlock(&stats_mutex);

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

			// Processa a mensagem com a tabela e o skeleton (implementação depende do contexto)
			invoke(request, table);  // Supõe que esta função exista
			// Verifica se ocorreu um erro

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

	printf("Client connection closed\n");

	return NULL;
}
