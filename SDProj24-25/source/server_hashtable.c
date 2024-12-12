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
#include <zookeeper/zookeeper.h>
#include "../include/server_skeleton.h"
#include "../include/server_network.h"
#include "../include/table.h"
#include "../include/htmessages.pb-c.h"

//Variáveis globais do ZooKeeper
static zhandle_t *zh;
static char znode_path[256];
static char successor_path[256];
static int is_tail = 0;

int compare_strings(const void *a, const void *b) {
    const char *str_a = *(const char **)a;
    const char *str_b = *(const char **)b;
    return strcmp(str_a, str_b);
}

void ensure_chain_node_exists() {
    // Check if the /chain node exists
    struct Stat stat;
    int ret = zoo_exists(zh, "/chain", 0, &stat);
    if (ret == ZNONODE) {
        // Create the /chain node if it does not exist
        ret = zoo_create(zh, "/chain", "", 0, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0);
        if (ret != ZOK) {
            fprintf(stderr, "Erro ao criar o nó /chain: %s\n", zerror(ret));
            exit(EXIT_FAILURE);
        }
        printf("Nó /chain criado com sucesso\n");
    } else if (ret != ZOK) {
        fprintf(stderr, "Erro ao verificar a existência do nó /chain: %s\n", zerror(ret));
        exit(EXIT_FAILURE);
    }
}

void connect_to_zookeeper(int port) {
    const char* zookeeper_host = "127.0.0.1:2181";

    zh = zookeeper_init(zookeeper_host, NULL, 2000, 0, NULL, 0);
    if (!zh) {
        fprintf(stderr, "Erro ao conectar ao ZooKeeper\n");
        exit(EXIT_FAILURE);
    }

    // Ensure the /chain node exists
    ensure_chain_node_exists();

    // Create the ZNode with the actual IP and port
    char server_data[64];
    snprintf(server_data, sizeof(server_data), "%s:%d", "127.0.0.1", port);

    // Criar ZNode efémero sequencial
    int ret = zoo_create(zh, "/chain/node-", server_data, strlen(server_data), &ZOO_OPEN_ACL_UNSAFE,
                         ZOO_EPHEMERAL | ZOO_SEQUENCE, znode_path, sizeof(znode_path));
    if (ret != ZOK) {
        fprintf(stderr, "Erro ao criar ZNode: %s\n", zerror(ret));
        exit(EXIT_FAILURE);
    }

    printf("Servidor registado no ZooKeeper com ZNode: %s\n", znode_path);
}

// Atualiza o sucessor com base nos filhos de /chain
void update_successor() {
    struct String_vector children;
    int ret = zoo_get_children(zh, "/chain", 1, &children);
    if (ret != ZOK) {
        fprintf(stderr, "Erro ao obter filhos de /chain: %s\n", zerror(ret));
        return;
    }

    // Ordenar os filhos e identificar o sucessor
    qsort(children.data, children.count, sizeof(char *), compare_strings);
    successor_path[0] = '\0';
    for (int i = 0; i < children.count; i++) {
        if (strcmp(znode_path, children.data[i]) < 0) {
            snprintf(successor_path, sizeof(successor_path), "/chain/%s", children.data[i]);
            break;
        }
    }
    is_tail = (successor_path[0] == '\0');

    // Se houver sucessor, conectar-se a ele
    if (!is_tail) {
        char successor_data[64];
        int len = sizeof(successor_data);
        ret = zoo_get(zh, successor_path, 0, successor_data, &len, NULL);
        if (ret != ZOK) {
            fprintf(stderr, "Erro ao obter dados do sucessor: %s\n", zerror(ret));
            fprintf(stderr, "Successor path: %s\n", successor_path);
        } else {
            printf("Conectado ao sucessor: %s\n", successor_data);
            connect_to_next_server(successor_data); // Função implementada em outro local
        }
    } else {
        printf("Este servidor é a cauda da cadeia.\n");
    }

    deallocate_String_vector(&children);
}

// Callback do watcher
void my_watcher_fn(zhandle_t *zzh, int type, int state, const char *path, void *watcherCtx) {
    if (type == ZOO_CHILD_EVENT) {
        printf("Alteração detectada nos filhos de /chain\n");
        update_successor();
    }
}

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

    // 2. Inicializar a tabela
    struct table_t *table = server_skeleton_init(n_lists);
    if (!table ) {
        fprintf(stderr, "Erro ao inicializar a tabela.\n");
        server_network_close(server_socket);
        return -1;
    }

    connect_to_zookeeper(port);

    update_successor();

    // 3. Loop principal para aceitar conexões de clientes e processar operações
    while (1) {
        printf("Server ready, waiting for connections\n");

        if (network_main_loop(server_socket, table) < 0) {
            printf("Erro no loop de atendimento ao cliente.\n");
            break;
        }

    }

    // Limpeza dos recursos
    zookeeper_close(zh);
    server_skeleton_destroy(table);
    server_network_close(server_socket);
    printf("Servidor finalizado.\n");

    return 0;
}
