#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "../include/serialization.h"

/*
 * Serializes the array of strings into a buffer.
 * Format:
 * | int (nkeys) | string1 | string2 | string3 |
 * Return: size of buffer or -1 on error.
 */
int keyArray_to_buffer(char **keys, char **keys_buf) {
    if (keys == NULL || keys_buf == NULL) {
        return -1;
    }

    int nkeys = 0;
    // Calculate number of keys
    while (keys[nkeys] != NULL) {
        nkeys++;
    }

    // Calculate required buffer size
    int buffer_size = sizeof(int); // For nkeys (int)
    for (int i = 0; i < nkeys; i++) {
        buffer_size += strlen(keys[i]) + 1; // Include null terminator for each key
    }

    // Allocate memory for the buffer
    *keys_buf = (char *)malloc(buffer_size);
    if (*keys_buf == NULL) {
        return -1; // Memory allocation failed
    }

    char *ptr = *keys_buf;

    // First, write nkeys in network byte order (htonl)
    int nkeys_net = htonl(nkeys);
    memcpy(ptr, &nkeys_net, sizeof(int));
    ptr += sizeof(int);

    // Then, serialize each string
    for (int i = 0; i < nkeys; i++) {
        int len = strlen(keys[i]) + 1; // Include null terminator
        memcpy(ptr, keys[i], len);
        ptr += len;
    }

    return buffer_size; // Return the size of the buffer
}

/*
 * Deserializes the buffer into an array of strings.
 * Format:
 * | int (nkeys) | string1 | string2 | string3 |
 * Return: array of strings or NULL on error.
 */
char **buffer_to_keyArray(char *keys_buf) {
    if (keys_buf == NULL) {
        return NULL;
    }

    // Read nkeys in network byte order (ntohl)
    int nkeys;
    memcpy(&nkeys, keys_buf, sizeof(int));
    nkeys = ntohl(nkeys); // Convert from network byte order to host byte order
    keys_buf += sizeof(int);

    // Allocate memory for the array of strings
    char **keys = (char **)malloc((nkeys + 1) * sizeof(char *)); // +1 for NULL terminator
    if (keys == NULL) {
        return NULL; // Memory allocation failed
    }

    // Deserialize each string
    for (int i = 0; i < nkeys; i++) {
        int len = strlen(keys_buf) + 1; // Include null terminator
        keys[i] = (char *)malloc(len);
        if (keys[i] == NULL) {
            // Free previously allocated memory in case of failure
            for (int j = 0; j < i; j++) {
                free(keys[j]);
            }
            free(keys);
            return NULL;
        }
        memcpy(keys[i], keys_buf, len); // Copy string
        keys_buf += len; // Move to the next string
    }

    keys[nkeys] = NULL; // Null-terminate the array of strings

    return keys;
}
