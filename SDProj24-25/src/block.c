#include <stdlib.h> // Para malloc, free
#include <string.h> // Para memcpy
#include "../include/block.h"  // Incluir o cabeçalho do bloco

/* Função que cria um novo bloco de dados block_t e que inicializa
* os dados de acordo com os argumentos recebidos, sem necessidade de
* reservar memória para os dados.
* Retorna a nova estrutura ou NULL em caso de erro.
*/
struct block_t *block_create(int size, void *data) {

	if ( size <= 0 || !data ) { // Nao deve criar blocos se o tamanho for 0 ou o ponteiro para data for NULL
	        return NULL; // Retorna NULL se a verificação falhar
	    }

    struct block_t *new_block = malloc(sizeof(struct block_t)); //Tenta criar um novo bloco

    if (!new_block){
        free(new_block);
        return NULL; // Falha ao alocar memoria
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

    if (!dup_block) return NULL; // Falha ao alocar memoria

    dup_block->datasize = b->datasize;
    dup_block->data = malloc(b->datasize); // Aloca memória para os dados para a nova instancia ser completamente independente

    if (!dup_block->data) {
        free(dup_block); // Liberta a estrutura se falhar
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

    if (!b || new_size <= 0|| new_data == NULL) {
    	return -1; // Verifica se o bloco existe e se nao e vazio
    }

    block_destroy(b); // Liberta o bloco antigo

    block_create(new_size, new_data); // Cria um novo bloco com os novos dados

    return 0; // Sucesso
}

/* Função que elimina um bloco de dados, apontado pelo parâmetro b,
* libertando toda a memória por ele ocupada.
* Retorna 0 (OK) ou -1 em caso de erro.
*/
int block_destroy(struct block_t *b) {

    if (!b) return -1; // Verifica se o bloco existe
    free(b->data); // Liberta os dados
    free(b); // Liberta a estrutura

    return 0; // Sucesso
}
