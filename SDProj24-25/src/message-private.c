#include <unistd.h>  // para read() e write()
#include <errno.h>   // para verificar erros
#include "../include/message-private.h"  // Certifique-se de incluir o cabeçalho correto

/**
 * Função que tenta enviar todo o buffer para o socket, assegurando que
 * todo o buffer foi enviado antes de retornar.
 *
 * @param sockfd O descritor do socket para onde os dados serão enviados.
 * @param buf O buffer que contém os dados a enviar.
 * @param len O tamanho do buffer.
 * @return O número total de bytes enviados, ou -1 em caso de erro.
 */
size_t write_all(int sockfd, const void *buf, size_t len) {
    size_t total_sent = 0;  // Bytes já enviados
    const char *ptr = (const char *) buf;

    while (total_sent < len) {
        ssize_t bytes_sent = write(sockfd, ptr + total_sent, len - total_sent);
        if (bytes_sent < 0) {
            if (errno == EINTR) {
                continue;  // Interrompido por sinal, tentar novamente
            }
            return -1;  // Outro erro ocorreu
        }
        total_sent += bytes_sent;
    }
    return total_sent;
}

/**
 * Função que tenta ler todo o buffer de um socket, assegurando que
 * todo o buffer foi lido antes de retornar.
 *
 * @param sockfd O descritor do socket de onde os dados serão lidos.
 * @param buf O buffer onde os dados lidos serão armazenados.
 * @param len O tamanho do buffer.
 * @return O número total de bytes lidos, ou -1 em caso de erro.
 */
size_t read_all(int sockfd, void *buf, size_t len) {
    size_t total_read = 0;  // Bytes já lidos
    char *ptr = (char *) buf;

    while (total_read < len) {
        ssize_t bytes_read = read(sockfd, ptr + total_read, len - total_read);
        if (bytes_read < 0) {
            if (errno == EINTR) {
                continue;  // Interrompido por sinal, tentar novamente
            }
            return -1;  // Outro erro ocorreu
        }
        if (bytes_read == 0) {
            break;  // Conexão fechada
        }
        total_read += bytes_read;
    }
    return total_read;
}
