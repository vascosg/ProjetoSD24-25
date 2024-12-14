/* ------------------------------------
---------------  SD-054 ---------------
    Filipa Inácio       fc59788
    Tomás Canilhas      fc59794
    Vasco Baldé         fc58174
---------------------------------------
------------------------------------ */

/*#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <zookeeper/zookeeper.h>

#include "../include/client_network.h"
#include "../include/entry.h"
#include "../include/block.h"
#include "../include/client_stub-private.h"
#include "../include/stats.h"

#define MAX_COMMAND_LEN 1024
#define MAX_TOKENS 3
#define USAGE_MESSAGE "Usage: p[ut] <key> <value> | g[et] <key> | d[el] <key> | s[ize] | [get]k[eys] | [get]t[able] | st[ats] | q[uit]\n"

// Variáveis globais do ZooKeeper
static zhandle_t *zh;
static char head_path[256];
static char tail_path[256];
struct rtable_t *head;
struct rtable_t *tail;

static int compare_strings(const void *a, const void *b) {
    const char *str_a = *(const char **)a;
    const char *str_b = *(const char **)b;
    return strcmp(str_a, str_b);
}

// Callback do watcher
static void my_watcher_fn(zhandle_t *zzh, int type, int state, const char *path, void *watcherCtx) {
    if (type == ZOO_CHILD_EVENT) {
        printf("Alteração detectada nos filhos de /chain\n");

    }
}

// Função para conectar ao ZooKeeper
static void connect_to_zookeeper(char* zk_port) {

	zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);
	
    zh = zookeeper_init(zk_port, NULL, 2000, 0, NULL, 0);
    if (!zh) {
        fprintf(stderr, "Erro ao conectar ao ZooKeeper\n");
        exit(EXIT_FAILURE);
    }
    printf("Cliente conectado ao ZooKeeper\n");
}

// Atualiza o head e o tail com base nos filhos de /chain
void link_to_head_and_tail() {
    struct String_vector children;
    if (zoo_wget_children(zh, "/chain", my_watcher_fn, "Zookeeper Data Watcher", &children) != ZOK) {
        fprintf(stderr, "Erro ao obter filhos de /chain\n");
        return;
    }
    // Ordenar os filhos e identificar head e tail
    qsort(children.data, children.count, sizeof(char *), compare_strings);
    if (children.count > 0) {
		snprintf(head_path, sizeof(head_path), "/chain/%s", children.data[0]);
		snprintf(tail_path, sizeof(tail_path), "/chain/%s", children.data[children.count - 1]);

        // Obter dados do head e tail
        char head_data[64], tail_data[64];
        int len = sizeof(head_data);
        if (zoo_get(zh, head_path, 0, head_data, &len, NULL) != ZOK) {
            printf("Erro ao obter dados do head\n");
			deallocate_String_vector(&children);
			return;
        }
		head = rtable_connect(head_data);

		if (head == NULL) {
			printf("Erro ao conectar ao head\n");
			deallocate_String_vector(&children);
			return;
		}

        len = sizeof(tail_data);
        if (zoo_get(zh, tail_path, 0, tail_data, &len, NULL) != ZOK) {
			printf("Erro ao obter dados do tail\n");
			deallocate_String_vector(&children);
			return;
        }
		tail = rtable_connect(tail_data);

		if (tail == NULL) {
			printf("Erro ao conectar ao tail\n");
			deallocate_String_vector(&children);
			return;
		}
    }
    deallocate_String_vector(&children);
}

//----------------------------------------------------------------------

int main(int argc, char **argv) {

	// Mutexes already initialized with PTHREAD_MUTEX_INITIALIZER

	if (argc != 2) {
		fprintf(stderr, "Usage: %s ZK <server>:<port>\n", argv[0]);
		return -1;
	}

	char* zookeeper_ip = argv[1];
	connect_to_zookeeper(zookeeper_ip);

    link_to_head_and_tail();

	char command[MAX_COMMAND_LEN];
	char *tokens[MAX_TOKENS]; // Array para guardar os pointers
	int token_count;

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
			rtable_disconnect(head);
			rtable_disconnect(tail);	
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
				if (head->sockfd < 0) {
					block_destroy(block);
					entry_destroy(entry);
					//pthread_mutex_unlock(&table_mutex);
					break;
				}

				rtable_put(head,entry);
				//pthread_mutex_unlock(&table_mutex);

			} else if (( strcmp(tokens[0], "get") == 0 || (strcmp(tokens[0], "g") == 0) ) ) {

				// Verifica se o comando get tem os argumentos necessários
				if (token_count < 2 || !tokens[1]  ) {
					printf("Invalid arguments. Usage: get <key>\n");
					continue;
				}

				//pthread_mutex_lock(&table_mutex);

				// Verifica se a conexão com o servidor foi bem sucedida
				if (tail->sockfd < 0) {
					//pthread_mutex_unlock(&table_mutex);
					break;
				}

				struct block_t *block_received = rtable_get(tail, tokens[1]);
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
				if (head->sockfd < 0) {
					//pthread_mutex_unlock(&table_mutex);
					break;
				}

				int del_result = rtable_del(head,tokens[1]);
				//pthread_mutex_unlock(&table_mutex);

				// Verifica se a operação del foi bem sucedida
				if (del_result != 0) {
					printf("Error in rtable_del or key not found! \n");
					continue;
				}


			} else if (( strcmp(tokens[0], "size") == 0 || (strcmp(tokens[0], "s") == 0))) {

				//pthread_mutex_lock(&table_mutex);

				// Verifica se a conexão com o servidor foi bem sucedida
				if (tail->sockfd < 0) {
					//pthread_mutex_unlock(&table_mutex);
					break;
				}

				int size = rtable_size(tail);
				//pthread_mutex_unlock(&table_mutex);

				if (size == -1) {
					printf("Error in rtable_size! \n");
					continue;
				}

				printf("Table size: %d\n",size);

			} else if (( strcmp(tokens[0], "getkeys") == 0 || (strcmp(tokens[0], "k") == 0) )) {

				//pthread_mutex_lock(&table_mutex);

				// Verifica se a conexão com o servidor foi bem sucedida
				if (tail->sockfd < 0) {
					//pthread_mutex_unlock(&table_mutex);
					break;
				}

				size_t num_keys = rtable_size(tail);
				char **keys = rtable_get_keys(tail);
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
				if (tail->sockfd < 0) {
					//pthread_mutex_unlock(&table_mutex);
					break;
				}

				rtable_get_table(tail);	// TODO não estamos a usar o retorno do get_table
				//pthread_mutex_unlock(&table_mutex);


			} else if ((strcmp(tokens[0], "stats") == 0) || (strcmp(tokens[0], "st") == 0)) {

				//pthread_mutex_lock(&stats_mutex);

				// Verifica se a conexão com o servidor foi bem sucedida
				if (tail->sockfd < 0) {
					//pthread_mutex_unlock(&stats_mutex);
					break;
				}
				
				struct statistics_t *stats_result = rtable_stats(tail);
				//pthread_mutex_unlock(&stats_mutex);

				// Verifica se a operação del foi bem sucedida
				if (stats_result == NULL) {
					printf("Error in rtable_stats! \n");
					continue;
				}

			} else {
				printf(USAGE_MESSAGE);
			}

		}
	}

	zookeeper_close(zh);

	return 0;

}*/
