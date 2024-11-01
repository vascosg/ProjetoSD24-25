#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "../include/server_skeleton.h"
#include "../include/server_network.h"
#include "../include/table.h"
#include "../include/htmessages.pb-c.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <porta> <número de listas>\n", argv[0]);
        return -1;
    }

    // Obtenção dos parâmetros passados na linha de comando
    int port = atoi(argv[1]);
    int n_lists = atoi(argv[2]);

    // Validação dos parâmetros
    if (port <= 0 || n_lists <= 0) {
        fprintf(stderr, "Erro: porta e número de listas devem ser maiores que zero.\n");
        return -1;
    }

    // 1. Inicializar o servidor e configurar o socket de escuta
    int server_socket = server_network_init(port);
    if (server_socket < 0) {
        fprintf(stderr, "Erro ao inicializar o socket do servidor.\n");
        return -1;
    }
    printf("Servidor inicializado e escutando na porta %d...\n", port);

    // 2. Inicializar a tabela
    struct table_t *table = server_skeleton_init(n_lists);
    if (!table ) {
        fprintf(stderr, "Erro ao inicializar a tabela.\n");
        server_network_close(server_socket);
        return -1;
    }
    printf("Tabela inicializada com %d listas.\n", n_lists);

    // 3. Loop principal para aceitar conexões de clientes e processar operações
    while (1) {
        printf("Aguardando conexão de um cliente...\n");

        if (network_main_loop(server_socket, table) < 0) {
            fprintf(stderr, "Erro no loop de atendimento ao cliente.\n");
            break;
        }

        printf("Conexão encerrada. Aguardando novo cliente...\n");
    }

    // Limpeza dos recursos
    server_skeleton_destroy(table);
    server_network_close(server_socket);
    printf("Servidor finalizado.\n");

    return 0;
}
