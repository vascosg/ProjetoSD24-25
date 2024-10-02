#include <stdlib.h>
#include <string.h>
#include "../include/table-private.h"

/*Funcao para decidir qual a lista dentro da tabela no qual a entry
 * vai ser colocada.Multiplica o valor ASCII  dos digitos na key e devolve o
 * resto da divisao por n onde n e o numero de listas na table
 */
int hash_function(char *key, int n) {
    int multi = 0;
    for (int i = 0; key[i] != '\0'; i++) {
    	multi *= key[i];
    }
    return multi % n;
}
