#ifndef _STATS_H
#define _STATS_H /* Módulo STATS */

/* Estrutura que armazena as estatísticas de uma tabela.
 */
struct statistics_t {
    int n_ops;      /* Número de operações realizadas na tabela (exceto a operação stats) */
    double time_spent; /* Tempo total gasto nas operações */
    int n_clients;  /* Número de clientes que acederam à tabela */
};

/* Função que cria uma nova estrutura statistics_t e inicializa os seus
 * campos.
 * Retorna a nova estrutura ou NULL em caso de erro.
 */
struct statistics_t *statistics_create();

/* Função que destrói a estrutura statistics_t, libertando toda a memória
 * previamente alocada.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int statistics_destroy(struct statistics_t *stats);

/* Função que atualiza as estatísticas de uma tabela de acordo com o tempo
 * e o número de clientes dados.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int statistics_update(struct statistics_t *stats, int time, int clients, int ops);

#endif
