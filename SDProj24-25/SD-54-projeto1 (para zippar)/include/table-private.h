#ifndef INCLUDE_TABLE_PRIVATE_H_
#define INCLUDE_TABLE_PRIVATE_H_

#include "list.h"

struct table_t {
    int size;
    struct list_t **lists;
};

/*Funcao para decidir qual a lista dentro da tabela no qual a entry
 * vai ser colocada.Multiplica o valor ASCII  dos digitos na key e devolve o
 * resto da divisao por n onde n e o numero de listas na table
 */
int hash_function(char *key, int n);

#endif
