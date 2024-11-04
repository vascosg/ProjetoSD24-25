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
#include <arpa/inet.h>
#include "../include/serialization.h"

/* Serializa todas as chaves presentes no array de strings keys para o
* buffer keys_buf, que será alocado dentro da função. A serialização
* deve ser feita de acordo com o seguinte formato:
*
| int
| string | string | string |
*
| nkeys | key1
| key2
| key3
|
* Retorna o tamanho do buffer alocado ou -1 em caso de erro.
*/
int keyArray_to_buffer(char **keys, char **keys_buf) {
    if (keys == NULL || keys_buf == NULL) {
        return -1;
    }

    int nkeys = 0;
    // Calcula o número de chaves
    while (keys[nkeys] != NULL) {
        nkeys++;
    }

    // Calcula o espaço requerido do buffer
    int buffer_size = sizeof(int); // Para nkeys (int)
    for (int i = 0; i < nkeys; i++) {
        buffer_size += strlen(keys[i]) + 1; // +1 para incluir o null no final
    }


    *keys_buf = (char *)malloc(buffer_size);
    if (!*keys_buf) return -1; // Falha ao alocar memoria para o buffer

    char *ptr = *keys_buf;

    // primeiro escreve nkeys em ordem network byte  (htonl)
    int nkeys_net = htonl(nkeys);
    memcpy(ptr, &nkeys_net, sizeof(int));
    ptr += sizeof(int);

    // Serializar as strings
    for (int i = 0; i < nkeys; i++) {
        int len = strlen(keys[i]) + 1; // Novamente incluir no final
        memcpy(ptr, keys[i], len);
        ptr += len;
    }

    return buffer_size;
}

/* De-serializa a mensagem contida em keys_buf, colocando-a num array de
* strings cujo espaco em memória deve ser reservado. A mensagem contida
* em keys_buf deverá ter o seguinte formato:
*
| int
| string | string | string |
*
| nkeys | key1
| key2
| key3
|
* Retorna o array de strings ou NULL em caso de erro.
*/
char **buffer_to_keyArray(char *keys_buf) {
    if (keys_buf == NULL) {
        return NULL;
    }

    // lw nkeys em ordem network byte (ntohl)
    int nkeys;
    memcpy(&nkeys, keys_buf, sizeof(int));
    nkeys = ntohl(nkeys); // Converter de ordem network byte para ordem de bytes
    keys_buf += sizeof(int);

    // Alocar memoria para o array de strings
    char **keys = (char **)malloc((nkeys + 1) * sizeof(char *)); // +1 termina null
    if (!keys) return NULL; // Falha ao alocar memoria para a lista de chaves

    // Deserializar cada string
    for (int i = 0; i < nkeys; i++) {
        int len = strlen(keys_buf) + 1; // Incluir o null no fim
        keys[i] = (char *)malloc(len);
        if (keys[i] == NULL) {
            // Libertar memoria anteriormente alocada em caso de erro
            for (int j = 0; j < i; j++) {
                free(keys[j]);
            }
            free(keys);
            return NULL;
        }
        memcpy(keys[i], keys_buf, len); // Copia string
        keys_buf += len; // Passa a próxima string
    }

    keys[nkeys] = NULL; // terminar as strings com null
    return keys;
}
