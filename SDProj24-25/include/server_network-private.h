

#ifndef INCLUDE_SERVER_NETWORK_PRIVATE_H_
#define INCLUDE_SERVER_NETWORK_PRIVATE_H_

#include "htmessages.pb-c.h"

struct client_thread_args {
    int client_socket;
    struct table_t *table;
    struct statistics_t *stats;
};

/* Esta funçao e utilizada na criação de cada thread
 * Processa todos os pedidos de um cliente
 */
void *client_handler(void *args);

#endif
