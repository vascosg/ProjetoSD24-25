#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/client_network.h"
#include "../include/entry.h"
#include "../include/block.h"
#include "../include/client_stub-private.h"
#include <stdio.h>
#include <string.h>
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

	printf("Informacoes da rtable: %s\n", argv[1]);
	fflush(stdout);  // Garantir que a mensagem seja exibida imediatamente

	char command[MAX_COMMAND_LEN];
	char *tokens[MAX_TOKENS]; // Array para guardar os pointers
	int token_count;
	struct rtable_t *rt = rtable_connect(argv[1]);

	if (!rt) {
		printf("Erro ao conectar ao servidor. A terminar o programa...\n");
		return -1;
	}

	// Loop principal
	while (1) {
		printf("Insira um novo comando: ");

		// Lê o comando do utilizador
		if (fgets(command, MAX_COMMAND_LEN, stdin) == NULL) {
			printf("Erro ao ler o comando.\n");
			continue;
		}

		// Remover o newline no final da string
		command[strcspn(command, "\n")] = '\0';

		if (strspn(command, " ") == strlen(command)) {
			printf("Comando vazio ou apenas espaços. Por favor, insira um comando válido.\n");
			continue;  // Volta para o início do loop
		}

		char *token;
		token_count = 0;

		// Primeiro token é a string até o primeiro espaço
		token = strtok(command, " ");

		// Verifica se o comando é 'quit'
		if (token != NULL && strcmp(token, "quit") == 0) { //TODO switch case ?
			printf("A terminar o programa...\n");
			break;
		}

		// Guarda os tokens que vao ate cada espaco
		while (token != NULL && token_count < MAX_TOKENS) {
			// Aloca memória para cada token e copia o conteúdo
			tokens[token_count] = strdup(token); // Usar strdup para duplicar a string
			token_count++;
			token = strtok(NULL, " "); // Próximo token
		}

		if(token_count > 0 && strcmp(tokens[0], "put") == 0){ //TODO checkar a quantidade de argumentos

			struct block_t *block = block_create(sizeof(tokens[2]),tokens[2]);
			struct entry_t *entry = entry_create(tokens[1],block);

			if (rt->sockfd < 0) {
				perror("Socket invalido\n");
				break;
			}
			rtable_put(rt,entry);

		} else if (token_count > 0 && strcmp(tokens[0], "get") == 0) { //TODO preparar para o caso de n encontrar a entrada

			if (rt->sockfd < 0) {
				perror("Socket invalido\n");
				break;
			}

			struct block_t *block_received = rtable_get(rt, tokens[1]);
			printf("Block size: %d\n", block_received->datasize);
			printf("Block data: ");
			for (int i = 0; i < block_received->datasize; i++) {
				printf("%02x ", ((unsigned char *)block_received->data)[i]); // Imprime como hex
			}
			printf("\n");
		} else if (token_count > 0 && strcmp(tokens[0], "del") == 0) { // TODO Prepatar para quando n tem a entry para deletar

			if (rt->sockfd < 0) {
				perror("Socket invalido\n");
				break;
			}

			if (rtable_del(rt,tokens[1]) != 0) {
				printf("Erro ao eliminar entrada na tabela \n");
			}

		} else if (token_count > 0 && strcmp(tokens[0], "size") == 0) {

			if (rt->sockfd < 0) {
				perror("Socket invalido\n");
				break;
			}

			int size = rtable_size(rt);

			if (size == -1) {
				printf("Erro ao obter o numero de elementos na tabela \n");
			}
			printf("Numero de elementos na tabela: %d\n",size);

		} else if (token_count > 0 && strcmp(tokens[0], "getkeys") == 0) { //TODO da erro no list free keys tiver 5 elementos...

			if (rt->sockfd < 0) {
				perror("Socket invalido\n");
				break;
			}

			size_t num_keys = rtable_size(rt);
			char **keys = rtable_get_keys(rt);

			if (!keys) {
				printf("Erro ao obter as chaves\n");
				break;
			} else {
				for (size_t i = 0; i < num_keys; i++) {
					printf("Key[%zu]: %s\n", i, keys[i]);
				}

				rtable_free_keys(keys);
			}


		} else if (token_count > 0 && strcmp(tokens[0], "gettable") == 0) {

			if (rt->sockfd < 0) {
				perror("Socket invalido\n");
				break;
			}

			struct entry_t ** table_entries = rtable_get_table(rt);

			if(!table_entries) {
				printf("Erro ao obter as entradas\n");
			} else {
				printf("A imprimir as entradas ...\n");

				size_t num_entries = rtable_size(rt);

				for (size_t i = 0; i < num_entries; i++) {
					struct entry_t *entry = table_entries[i];
					if (entry) {
						printf("Entry %zu:\n", i);
						printf("  Key: %s\n", entry->key);
					} else {
						printf("Entry %zu is NULL\n", i);
					}
				}
			}

		}

	}

	return 0;
}
