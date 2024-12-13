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

#include "../include/server_skeleton.h"
#include "../include/server_network.h"
#include "../include/table.h"
#include "../include/htmessages.pb-c.h"

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s ZK<IP:Porto> <porta> <número de listas>\n", argv[0]);
        return -1;
    }

    // Obtenção dos parâmetros passados na linha de comando
    char* zk_port = argv[1];
    short port = atoi(argv[2]);
    int n_lists = atoi(argv[3]);

    // Validação dos parâmetros
    if (port <= 0 || n_lists <= 0) {
        fprintf(stderr, "Erro: parâmetros inválidos.\n");
        return -1;
    }

    // 1. Inicializar o servidor e configurar o socket de escuta
    int server_socket = server_network_init(zk_port, port);
    if (server_socket < 0) {
        fprintf(stderr, "Erro ao inicializar o socket do servidor.\n");
        return -1;
    }

    // 2. Inicializar a tabela
    struct table_t *table = server_skeleton_init(n_lists);
    if (!table) {
        fprintf(stderr, "Erro ao inicializar a tabela.\n");
        server_network_close(server_socket);
        return -1;
    }

    if(set_table(table) < 0){
        fprintf(stderr, "Erro a preencher a tabela.\n");
        server_network_close(server_socket);
        return -1;
    }

    // 3. Loop principal para aceitar conexões de clientes e processar operações
    while (1) {
        printf("Server ready, waiting for connections\n");

        if (network_main_loop(server_socket, table) < 0) {
            printf("Erro no loop de atendimento ao cliente.\n");
            break;
        }

    }

    // Limpeza dos recursos
    server_skeleton_destroy(table);
    server_network_close(server_socket);
    printf("Servidor finalizado.\n");

    return 0;
}
