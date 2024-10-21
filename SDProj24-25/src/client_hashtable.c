#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/client_network.h"
#include "../include/entry.h"
#include "../include/block.h"
#include "../include/client_stub-private.h"

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
	char *tokens[MAX_TOKENS]; // Array to store pointers to tokens
	int token_count;
	struct rtable *rt = rtable_connect(argv[1]);

	if (!rt) {
		printf("Erro ao conectar ao servidor. A terminar o programa...");
	}

	// Loop principal
	while (1) {
		printf("Insira um novo comando: ");

		// Lê o comando do utilizador
		if (fgets(command, MAX_COMMAND_LEN, stdin) == NULL) {
			printf("Erro ao let o comando.\n");
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
		if (token != NULL && strcmp(token, "quit") == 0) {
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

		if(token_count > 0 && strcmp(tokens[0], "put") == 0){
			struct block_t *block = block_create(sizeof(tokens[2]),tokens[2]);
			struct entry_t *entry = entry_create(tokens[1],block);
			rtable_put(rt,entry);
		}

		// Print stored tokens
		printf("Tokens Guardados:\n");
		for (int i = 0; i < token_count; i++) {
			printf("Token[%d]: %s\n", i, tokens[i]);
			free(tokens[i]); // Liberta memória após uso
		}

	}

	return 0;
}


