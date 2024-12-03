/* ------------------------------------
---------------  SD-054 ---------------
    Filipa Inácio       fc59788
    Tomás Canilhas      fc59794
    Vasco Baldé         fc58174
---------------------------------------
------------------------------------ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/client_network.h"
#include "../include/entry.h"
#include "../include/block.h"
#include "../include/client_stub-private.h"
#include "../include/stats.h"
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#define MAX_COMMAND_LEN 1024
#define MAX_TOKENS 3
#define USAGE_MESSAGE "Usage: p[ut] <key> <value> | g[et] <key> | d[el] <key> | s[ize] | [get]k[eys] | [get]t[able] | st[ats] | q[uit]\n"

// inicializa mutexes
//pthread_mutex_t table_mutex = PTHREAD_MUTEX_INITIALIZER;	// mutex para a tabela
//pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;	// mutex para as estatisticas

int main(int argc, char **argv) {

	// Mutexes already initialized with PTHREAD_MUTEX_INITIALIZER

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <server>:<port>\n", argv[0]);
		return -1;
	}

	char command[MAX_COMMAND_LEN];
	char *tokens[MAX_TOKENS]; // Array para guardar os pointers
	int token_count;
	struct rtable_t *rt = rtable_connect(argv[1]);

	if (!rt) {
		printf("Error in connect\n");
		return -1;
	}

	// Loop principal
	while (1) {
		printf("Command: ");

		// Lê o comando do utilizador
		if (fgets(command, MAX_COMMAND_LEN, stdin) == NULL) {
			printf(USAGE_MESSAGE);
			continue;
		}

		// Remover o newline no final da string
		command[strcspn(command, "\n")] = '\0';

		if (strspn(command, " ") == strlen(command)) {
			//printf("");
			continue;  // Volta para o início do loop
		}

		char *token;
		token_count = 0;

		// Primeiro token é a string até o primeiro espaço
		token = strtok(command, " ");

		// Verifica se o comando é 'quit'
		if (token != NULL &&  ( strcmp(token, "quit") == 0 || strcmp(token,"q") == 0 ) ) {
			printf("Bye, bye!\n");
			rtable_disconnect(rt);	// Termina a conexão com o servidor (TODO e se falhar?)
			break;
		}


		// Guarda os tokens que vao ate cada espaco
		while (token != NULL && token_count < MAX_TOKENS) {
			// Aloca memória para cada token e copia o conteúdo
			tokens[token_count] = strdup(token); // Usar strdup para duplicar a string
			token_count++;
			token = strtok(NULL, " "); // Próximo token
		}

		if(token_count > 0) {

			if( strcmp(tokens[0], "put") == 0 || (strcmp(tokens[0], "p") == 0)  ){

				// Verifica se o comando put tem os argumentos necessários
				if (token_count < 3 || !tokens[1] || !tokens[2] ) {
					printf("Invalid arguments. Usage: put <key> <value>\n");
					continue;
				}

				struct block_t *block = block_create(sizeof(tokens[2]),tokens[2]);
				struct entry_t *entry = entry_create(tokens[1],block);

				//pthread_mutex_lock(&table_mutex);

				// Verifica se a conexão com o servidor foi bem sucedida
				if (rt->sockfd < 0) {
					block_destroy(block);
					entry_destroy(entry);
					//pthread_mutex_unlock(&table_mutex);
					break;
				}

				rtable_put(rt,entry);
				//pthread_mutex_unlock(&table_mutex);

			} else if (( strcmp(tokens[0], "get") == 0 || (strcmp(tokens[0], "g") == 0) ) ) {

				// Verifica se o comando get tem os argumentos necessários
				if (token_count < 2 || !tokens[1]  ) {
					printf("Invalid arguments. Usage: get <key>\n");
					continue;
				}

				//pthread_mutex_lock(&table_mutex);

				// Verifica se a conexão com o servidor foi bem sucedida
				if (rt->sockfd < 0) {
					//pthread_mutex_unlock(&table_mutex);
					break;
				}

				struct block_t *block_received = rtable_get(rt, tokens[1]);
				//pthread_mutex_unlock(&table_mutex);

				// Verifica se a operação get foi bem sucedida
				if (!block_received) {
					printf("Error in rtable_get or key not found!\n");
					continue;
				}

			} else if (( strcmp(tokens[0], "del") == 0 || (strcmp(tokens[0], "d") == 0) )  ) {

				// Verifica se o comando del tem os argumentos necessários
				if (token_count < 2 || !tokens[1]  ) {
					printf("Invalid arguments. Usage: del <key>\n");
					continue;
				}

				//pthread_mutex_lock(&table_mutex);

				// Verifica se a conexão com o servidor foi bem sucedida
				if (rt->sockfd < 0) {
					//pthread_mutex_unlock(&table_mutex);
					break;
				}

				int del_result = rtable_del(rt,tokens[1]);
				//pthread_mutex_unlock(&table_mutex);

				// Verifica se a operação del foi bem sucedida
				if (del_result != 0) {
					printf("Error in rtable_del or key not found! \n");
					continue;
				}


			} else if (( strcmp(tokens[0], "size") == 0 || (strcmp(tokens[0], "s") == 0))) {

				//pthread_mutex_lock(&table_mutex);

				// Verifica se a conexão com o servidor foi bem sucedida
				if (rt->sockfd < 0) {
					//pthread_mutex_unlock(&table_mutex);
					break;
				}

				int size = rtable_size(rt);
				//pthread_mutex_unlock(&table_mutex);

				if (size == -1) {
					printf("Error in rtable_size! \n");
					continue;
				}

				printf("Table size: %d\n",size);

			} else if (( strcmp(tokens[0], "getkeys") == 0 || (strcmp(tokens[0], "k") == 0) )) {

				//pthread_mutex_lock(&table_mutex);

				// Verifica se a conexão com o servidor foi bem sucedida
				if (rt->sockfd < 0) {
					//pthread_mutex_unlock(&table_mutex);
					break;
				}

				size_t num_keys = rtable_size(rt);
				char **keys = rtable_get_keys(rt);
				//pthread_mutex_unlock(&table_mutex);

				// Verifica se a operação get_keys foi bem sucedida
				if (keys) {

					// Imprime as chaves
					for (size_t i = 0; i < num_keys; i++) {
						printf("%s\n", keys[i]);
					}

				}

			} else if ((strcmp(tokens[0], "gettable") == 0 || (strcmp(tokens[0], "t") == 0)) ) {

				//pthread_mutex_lock(&table_mutex);

				// Verifica se a conexão com o servidor foi bem sucedida
				if (rt->sockfd < 0) {
					//pthread_mutex_unlock(&table_mutex);
					break;
				}

				rtable_get_table(rt);	// TODO não estamos a usar o retorno do get_table
				//pthread_mutex_unlock(&table_mutex);


			} else if ((strcmp(tokens[0], "stats") == 0) || (strcmp(tokens[0], "st") == 0)) {

				//pthread_mutex_lock(&stats_mutex);

				// Verifica se a conexão com o servidor foi bem sucedida
				if (rt->sockfd < 0) {
					//pthread_mutex_unlock(&stats_mutex);
					break;
				}
				
				int stats_result = rtable_stats(rt);
				//pthread_mutex_unlock(&stats_mutex);

				// Verifica se a operação del foi bem sucedida
				if (stats_result != 0) {
					printf("Error in rtable_stats! \n");
					continue;
				}

			} else {
				printf(USAGE_MESSAGE);
			}

		}
	}

	//pthread_mutex_destroy(&table_mutex);
	//pthread_mutex_destroy(&stats_mutex);

	return 0;

}
