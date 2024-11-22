/* ------------------------------------
---------------  SD-054 ---------------
    Filipa Inácio       fc59788
    Tomás Canilhas      fc59794
    Vasco Baldé         fc58174
---------------------------------------
------------------------------------ */

#include "../include/stats.h"
#include <stdlib.h>

/* Função que cria uma nova estrutura statistics_t e inicializa os seus
 * campos.
 * Retorna a nova estrutura ou NULL em caso de erro.
 */
struct statistics_t *statistics_create() {
    struct statistics_t *stats = (struct statistics_t *) malloc(sizeof(struct statistics_t));
    if (!stats) return NULL; // verifica se a alocação foi bem sucedida

    stats->n_ops = 0;
    stats->time_spent = 0;
    stats->n_clients = 0;

    return stats;
};

/* Função que destrói a estrutura statistics_t, libertando toda a memória
 * previamente alocada.
    * Retorna 0 (OK) ou -1 em caso de erro.
 */
void statistics_destroy(struct statistics_t *stats) {
    if (!stats) {
        return -1;
    };

    free(stats);

    return 0;
};

/* Função que atualiza as estatísticas de uma tabela de acordo com o tempo e
 * o número de clientes dados.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
void statistics_update(struct statistics_t *stats, int time, int clients) {
    if (!stats) return -1;

    stats->time_spent += time;
    stats->n_clients = clients;

    return 0;
};
