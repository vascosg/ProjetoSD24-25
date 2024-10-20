#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include "client_network.h"
#include "client_stub_private.h"

// Função para conectar ao servidor
int network_connect(struct rtable_t *rtable) {

    if (!rtable || !rtable->server_address || rtable->server_port <= 0) {
        return -1; // Verifica se os parâmetros são válidos
    }

    // Cria o socket
    rtable->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (rtable->sockfd < 0) {
        perror("Erro ao criar socket");
        return -1;
    }

    // Configura o endereço do servidor
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(rtable->server_port);

    // Converte o endereço IP de string para binário
    if (inet_pton(AF_INET, rtable->server_address, &server.sin_addr) <= 0) {
        perror("Erro ao converter endereço IP");
        close(rtable->sockfd);
        return -1;
    }

    // Tenta conectar ao servidor
    if (connect(rtable->sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Erro ao conectar ao servidor");
        close(rtable->sockfd);
        return -1;
    }

    return 0; // Conexão estabelecida com sucesso
}

// Função para enviar uma mensagem e receber a resposta
MessageT *network_send_receive(struct rtable_t *rtable, MessageT *msg) {
    if (!rtable || !msg) return NULL;

    // 1. Serializar a mensagem
    unsigned int len = message_t__get_packed_size(msg); // Tamanho da mensagem serializada
    void* buf = malloc(len);
    if (!len) return NULL;

    message_t__pack(msg, buf); // Serializa a mensagem para o buffer

    // 2. Enviar o tamanho da mensagem (2 bytes - short)
    uint16_t net_msg_size = htons(len); // Converte para network byte order
    if (write_all(rtable->sockfd, &net_msg_size, sizeof(net_msg_size)) != sizeof(net_msg_size)) {
        perror("Erro ao enviar o tamanho da mensagem");
        free(buf);
        return NULL;
    }

    // 3. Enviar a mensagem serializada
    if (write_all(rtable->sockfd, buf, len) != len) {
        perror("Erro ao enviar a mensagem");
        free(buf);
        return NULL;
    }

    free(buf); // Liberta o buffer após o envio

    // 4. Receber o tamanho da resposta (2 bytes - short)
    uint16_t net_response_size;
    if (read_all(rtable->sockfd, &net_response_size, sizeof(net_response_size)) != sizeof(net_response_size)) {
        perror("Erro ao receber o tamanho da resposta");
        return NULL;
    }
    uint16_t response_size = ntohs(net_response_size); // Converte para host byte order

    // 5. Receber a resposta serializada
    uint8_t *response_buffer = malloc(response_size);
    if (!response_buffer) {
        perror("Erro ao alocar memória para a resposta");
        return NULL;
    }

    if (read_all(rtable->sockfd, response_buffer, response_size) != response_size) {
        perror("Erro ao receber a resposta");
        free(response_buffer);
        return NULL;
    }

    // 6. Deserializar a resposta
    MessageT *response_msg = message_t__unpack(NULL, response_size, response_buffer);
    free(response_buffer); // Liberta o buffer após a deserialização

    if (!response_msg) {
        perror("Erro ao deserializar a resposta");
        return NULL;
    }

    // 7. Retorna a mensagem de resposta
    return response_msg;
}

// Função para fechar a ligação
int network_close(struct rtable_t *rtable) {
    if (!rtable) return -1;

    if (close(rtable->sockfd) < 0) {
        perror("Erro ao fechar o socket");
        return -1;
    }

    return 0;
}


