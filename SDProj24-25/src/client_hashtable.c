#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/client_network.h"
#include "../include/entry.h"
#include "../include/block.h"
#include "../include/client_stub-private.h"
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#define MAX_COMMAND_LEN 1024
#define MAX_TOKENS 3

int main(int argc, char **argv) {

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
			printf("Usage: p[ut] <key> <value> | g[et] <key> | d[el] <key> | s[ize] | [get]k[eys] | [get]t[able] | q[uit]\n");
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
		if (token != NULL &&  ( strcmp(token, "quit") == 0 || strcmp(token,"q") == 0 ) ) { //TODO switch case ? yesss
			printf("Bye, bye!\n");
			break;
		}

		// Guarda os tokens que vao ate cada espaco
		while (token != NULL && token_count < MAX_TOKENS) {
			// Aloca memória para cada token e copia o conteúdo
			tokens[token_count] = strdup(token); // Usar strdup para duplicar a string
			token_count++;
			token = strtok(NULL, " "); // Próximo token
		}

		if(token_count > 0 &&  ( strcmp(tokens[0], "put") == 0 || (strcmp(tokens[0], "p") == 0) ) ){

			if (token_count < 3 || !tokens[1] || !tokens[2] ) { // Verifica se o comando put tem os argumentos necessários
				printf("Invalid arguments. Usage: put <key> <value>\n");
				continue;
			}

			struct block_t *block = block_create(sizeof(tokens[2]),tokens[2]);
			struct entry_t *entry = entry_create(tokens[1],block);

			if (rt->sockfd < 0) {
				//perror("Socket invalido\n");
				break;
			}
			rtable_put(rt,entry);

		} else if (token_count > 0 && ( strcmp(tokens[0], "get") == 0 || (strcmp(tokens[0], "g") == 0) ) ) { //TODO preparar para o caso de n encontrar a entrada

			if (token_count < 2 || !tokens[1]  ) { // Verifica se o comando put tem os argumentos necessários
				printf("Invalid arguments. Usage: get <key>\n");
				continue;
			}

			if (rt->sockfd < 0) {
				//perror("Socket invalido\n");
				break;
			}

			struct block_t *block_received = rtable_get(rt, tokens[1]);
			if (!block_received) {
				printf("Error in rtable_get or key not found!\n");
				continue;
			}

		} else if (token_count > 0 &&  ( strcmp(tokens[0], "del") == 0 || (strcmp(tokens[0], "d") == 0) )  ) { // TODO Prepatar para quando n tem a entry para deletar

			if (token_count < 2 || !tokens[1]  ) { // Verifica se o comando put tem os argumentos necessários
				printf("Invalid arguments. Usage: del <key>\n");
				continue;
			}

			if (rt->sockfd < 0) {
				//perror("Socket invalido\n");
				break;
			}

			int del_result = rtable_del(rt,tokens[1]);

			if (del_result != 0) {
				printf("Error in rtable_del or key not found! \n");
				continue;
			}


		} else if (token_count > 0 && ( strcmp(tokens[0], "size") == 0 || (strcmp(tokens[0], "s") == 0))) {

			if (rt->sockfd < 0) {
				//perror("Socket invalido\n");
				break;
			}

			int size = rtable_size(rt);

			if (size == -1) {
				printf("Erro ao obter o numero de elementos na tabela \n");
			}

			printf("Table size: %d\n",size);

		} else if (token_count > 0 &&  ( strcmp(tokens[0], "getkeys") == 0 || (strcmp(tokens[0], "k") == 0) )) { //TODO da erro no list free keys tiver 5 elementos...

			if (rt->sockfd < 0) {
				//perror("Socket invalido\n");
				break;
			}

			size_t num_keys = rtable_size(rt);
			char **keys = rtable_get_keys(rt);

			if (!keys) {
				printf("Erro ao obter as chaves\n");
				//break;
			} else {
				for (size_t i = 0; i < num_keys; i++) {
					printf("%s\n", keys[i]);
				}

			}


		} else if (token_count > 0 &&  ( strcmp(tokens[0], "gettable") == 0 || (strcmp(tokens[0], "t") == 0)) ) {

			if (rt->sockfd < 0) {
				//perror("Socket invalido\n");
				break;
			}

			struct entry_t ** table_entries = rtable_get_table(rt);

			if(!table_entries) {
				printf("Erro ao obter as entradas\n");
			}

		} else {
			printf("Usage: p[ut] <key> <value> | g[et] <key> | d[el] <key> | s[ize] | [get]k[eys] | [get]t[able] | q[uit]\n");
		}

	}

	return 0;
}
