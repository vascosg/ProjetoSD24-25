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

/*void test_write_read_all()
{
    // Create a pair of connected sockets
    int sv[2];  // sv[0] and sv[1] are the file descriptors for the socket pair
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
        perror("socketpair");
        return;
    }

    const char *test_message = "Hello, world!";
    char read_buffer[100] = {0};  // Buffer to store the read message

    // Test write_all
    size_t write_len = write_all(sv[0], test_message, strlen(test_message));
    if (write_len != strlen(test_message)) {
        fprintf(stderr, "write_all failed, wrote %zu bytes, expected %zu\n", write_len, strlen(test_message));
        close(sv[0]);
        close(sv[1]);
        return;
    } else {
        printf("write_all succeeded, wrote %zu bytes\n", write_len);
    }

    // Test read_all
    size_t read_len = read_all(sv[1], read_buffer, strlen(test_message));
    if (read_len != strlen(test_message)) {
        fprintf(stderr, "read_all failed, read %zu bytes, expected %zu\n", read_len, strlen(test_message));
    } else {
        printf("read_all succeeded, read %zu bytes\n", read_len);
    }

    // Verify that the message was received correctly
    if (strncmp(test_message, read_buffer, strlen(test_message)) == 0) {
        printf("Message received successfully: %s\n", read_buffer);
    } else {
        fprintf(stderr, "Message received incorrectly, got: %s\n", read_buffer);
    }

    // Close the sockets
    close(sv[0]);
    close(sv[1]);
}*/

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

			if (rt->sockfd < 0) {
					    perror("Socket inválido");
					    return NULL;
				}
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
