#include <stdlib.h> // Para malloc, free
#include <string.h> // Para memcpy
#include "../include/block.h"  // Incluir o cabeçalho do bloco

/* Função que cria um novo bloco de dados block_t e que inicializa
* os dados de acordo com os argumentos recebidos, sem necessidade de
* reservar memória para os dados.
* Retorna a nova estrutura ou NULL em caso de erro.
*/
struct block_t *block_create(int size, void *data) {

	if (size <= 0 || data == NULL) { // Nao deve criar blocos se o tamanho for 0 ou o ponteiro para data for NULL
	        return NULL; // Retorna NULL se a verificação falhar
	    }

    struct block_t *new_block = malloc(sizeof(struct block_t)); //Tenta criar um novo bloco

    if (!new_block) {
        return NULL; // Falha ao alocar memória
    }

    new_block->datasize = size;
    new_block->data = data;

    return new_block;
}

/* Função que duplica uma estrutura block_t, reservando a memória
* necessária para a nova estrutura.
* Retorna a nova estrutura ou NULL em caso de erro.
*/
struct block_t *block_duplicate(struct block_t *b) {

    if (!b || b->datasize <= 0 || !b->data) {
    	return NULL; // Verifica se o bloco existe
    }

    struct block_t *dup_block = malloc(sizeof(struct block_t));

    if (!dup_block) {
        return NULL; // Falha ao alocar memória
    }

    // Copia os dados
    dup_block->datasize = b->datasize;
    dup_block->data = malloc(b->datasize); // Aloca memória para os dados para a nova instancia ser completamente independente

    if (!dup_block->data) {
        free(dup_block); // Libera a estrutura se falhar
        return NULL;
    }

    memcpy(dup_block->data, b->data, b->datasize); // Copia os dados

    return dup_block;
}

/* Função que substitui o conteúdo de um bloco de dados block_t.
* Deve assegurar que liberta o espaço ocupado pelo conteúdo antigo.
* Retorna 0 (OK) ou -1 em caso de erro.
*/
int block_replace(struct block_t *b, int new_size, void *new_data) {

    if (!b || new_size == 0|| new_data == NULL) {
    	return -1; // Verifica se o bloco existe e se nao e vazio
    }

    //free(b->data); // Liberta memoria antiga

    /*b->data = malloc(new_size);

    if (!b->data) {
        return -1; // Falha ao alocar memoria nova
    }*/

    b->data =new_data;
    //memcpy(b->data, new_data, new_size); // Copia os novos dados
    b->datasize = new_size; // Atualiza o tamanho

    return 0; // Sucesso
}

/* Função que elimina um bloco de dados, apontado pelo parâmetro b,
* libertando toda a memória por ele ocupada.
* Retorna 0 (OK) ou -1 em caso de erro.
*/
int block_destroy(struct block_t *b) {

    if (!b) return -1; // Verifica se o bloco existe
    free(b->data); // LIberta os dados
    free(b); // LIberta a estrutura

    return 0; // Sucesso
}
