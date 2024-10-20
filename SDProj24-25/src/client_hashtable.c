#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

	//TODO criar a socket ?talvez a chamar do client stub

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

		char *token;
		token_count = 0;

		// Primeiro token é a string até o primeiro espaço
		token = strtok(command, " ");

		// Verifica se o comando é 'quit'
		if (token != NULL && strcmp(token, "quit") == 0) {
			printf("Exiting...\n");
			break;
		}

		// Guarda os tokens que vao ate cada espaco
		while (token != NULL && token_count < MAX_TOKENS) {
			// Aloca memória para cada token e copia o conteúdo
			tokens[token_count] = strdup(token); // Usar strdup para duplicar a string
			token_count++;
			token = strtok(NULL, " "); // Próximo token
		}

		// Print stored tokens
		printf("Stored tokens:\n");
		for (int i = 0; i < token_count; i++) {
			printf("Token[%d]: %s\n", i, tokens[i]);
			free(tokens[i]); // Liberta memória após uso
		}

	}

	return 0;
}


