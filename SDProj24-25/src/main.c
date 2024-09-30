/*#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "block.h"
#include "entry.h"
#include "list.h"
#include "list-private.h"

// PRINTS
void print_block(struct block_t *block) {

	if (block == NULL) {
		printf("Block is NULL\n");
		return;
	}
	printf("Block size: %d\n", block->datasize);
	printf("Block data: ");
	for (int i = 0; i < block->datasize; i++) {
		printf("%02x ", ((unsigned char *)block->data)[i]); // Imprime como hex
	}
	printf("\n");
}

void print_entry(struct entry_t *entry) {
	if (entry == NULL) {
		printf("Entry is NULL\n");
		return;
	}
	printf("Entry key: %s\n", entry->key);
	print_block(entry->value);
}

void print_keys(struct list_t *list) {
	char **keys = list_get_keys(list); // Get keys from the list
	if (keys) {
		printf("Keys in the list:\n");
		for (int i = 0; keys[i] != NULL; i++) {
			printf("%s\n", keys[i]); // Print each key
		}
		list_free_keys(keys); // Free the keys array
	} else {
		printf("Failed to get keys from the list.\n");
	}
}

// TESTS
int test_block() {
	int data_size = 5;
	unsigned char *data = malloc(data_size); // Allocate memory for data
	if (!data) {
		printf("Failed to allocate memory for data.\n");
		return -1; // Handle memory allocation failure
	}
	data[0] = 1; data[1] = 2; data[2] = 3; data[3] = 4; data[4] = 5;

	struct block_t *block = block_create(data_size, data);

	if (block) {
		printf("Block created successfully:\n");
		print_block(block);
	} else {
		printf("Failed to create block.\n");
	}

	// Teste da função block_duplicate
	struct block_t *dup_block = block_duplicate(block);
	if (dup_block) {
		printf("Block duplicated successfully:\n");
		print_block(dup_block);
	} else {
		printf("Failed to duplicate block.\n");
	}

	// Teste da função block_replace
	unsigned char new_data[] = {6, 7, 8, 9, 10};
	if (block_replace(block, sizeof(new_data), new_data) == 0) {
		printf("Block replaced successfully:\n");
		print_block(block);
	} else {
		printf("Failed to replace block.\n");
	}

	// Teste da função block_destroy
	if (block_destroy(block) == 0) {
		printf("Block destroyed successfully.\n");
	} else {
		printf("Failed to destroy block.\n");
	}

	if (block_destroy(dup_block) == 0) {
		printf("Duplicate block destroyed successfully.\n");
	} else {
		printf("Failed to destroy duplicate block.\n");
	}

	return 0;
}

int test_entry() {
	// Teste da função entry_create
	int data_size = 5;
	unsigned char *data = malloc(data_size); // Aloca memoria para data

	if (!data) {
		printf("Failed to allocate memory for data.\n");
		return -1; // Verifica criação de data
	}
	data[0] = 1; data[1] = 2; data[2] = 3; data[3] = 4; data[4] = 5;

	struct block_t *block = block_create(data_size, data);

	if (!block) {
		printf("Failed to create block.\n");
		return -1; // Handle memory allocation failure
	}

	struct entry_t *entry = entry_create("test_key", block); // Cria uma entry

	if (entry) {
		printf("Entry created successfully:\n");
		print_entry(entry);
	} else {
		printf("Failed to create entry.\n");
	}

	// Teste da função entry_duplicate
	struct entry_t *dup_entry = entry_duplicate(entry);
	if (dup_entry) {
		printf("Entry duplicated successfully:\n");
		print_entry(dup_entry);
	} else {
		printf("Failed to duplicate entry.\n");
	}

	// Teste da função entry_replace
	unsigned char new_data[] = {6, 7, 8, 9, 10};
	struct block_t *new_block = block_create(5, new_data); // Cria um novo bloco
	if (entry_replace(entry, "new_key", new_block) == 0) {
		printf("Entry replaced successfully:\n");
		print_entry(entry);
	} else {
		printf("Failed to replace entry.\n");
	}

	// Teste da função entry_destroy
	if (entry_destroy(entry) == 0) {
		printf("Entry destroyed successfully.\n");
	} else {
		printf("Failed to destroy entry.\n");
	}

	if (dup_entry) {
		if (entry_destroy(dup_entry) == 0) {
			printf("Duplicate entry destroyed successfully.\n");
		} else {
			printf("Failed to destroy duplicate entry.\n");
		}
	}

	return 0;
}

int test_list() {

	struct list_t *list = list_create();
	if (!list) {
		printf("Failed to create list.\n");
		return -1;
	}

	// Add entries to the list
	int data_size = 5;
	unsigned char *data1 = malloc(data_size);
	if (!data1) return -1;
	data1[0] = 1; data1[1] = 2; data1[2] = 3; data1[3] = 4; data1[4] = 5;
	struct block_t *block1 = block_create(data_size, data1);
	struct entry_t *entry1 = entry_create("key1", block1);
	list_add(list, entry1);

	unsigned char *data2 = malloc(data_size);
	data2[0] = 6; data2[1] = 7; data2[2] = 8; data2[3] = 9; data2[4] = 10;
	struct block_t *block2 = block_create(data_size, data2);
	struct entry_t *entry2 = entry_create("key2", block2);
	list_add(list, entry2);


	printf("List size after adding two entries: %d\n", list_size(list));

	print_keys(list);

	// Obtme ENtry1
	struct entry_t *retrieved_entry = list_get(list, "key1");
	if (retrieved_entry) {
		printf("Retrieved entry:\n");
		print_entry(retrieved_entry);
	} else {
		printf("Failed to retrieve entry with key 'key1'.\n");
	}

	// Remove Entry1
	if (list_remove(list, "key1") == 0) {
		printf("Entry with key 'key1' removed successfully.\n");
	} else {
		printf("Failed to remove entry with key 'key1'.\n");
	}

	printf("List size after removing one entry: %d\n", list_size(list));

	print_keys(list);

	// Obtem Entry1
	struct entry_t *retrieved_entry2 = list_get(list, "key2");
	if (retrieved_entry2) {
		printf("Retrieved entry2:\n");
		print_entry(retrieved_entry2);
	} else {
		printf("Failed to retrieve entry with key 'key2'.\n");
	}

	unsigned char *data3 = malloc(data_size);
	data3[0] = 11; data3[1] = 12; data3[2] = 13; data3[3] = 14; data3[4] = 15;
	struct block_t *block3 = block_create(data_size, data3);
	struct entry_t *entry3 = entry_create("key3", block3);
	list_add(list, entry3);

	unsigned char *data4 = malloc(data_size);
		if (!data1) return -1;
		data4[0] = 1; data4[1] = 2; data4[2] = 3; data4[3] = 4; data4[4] = 5;
		struct block_t *block4 = block_create(data_size, data4);
		struct entry_t *entry4 = entry_create("key11", block4);
	list_add(list, entry4);

	print_keys(list);

	// Destroi lista
	if (list_destroy(list) == 0) {
		printf("List destroyed successfully.\n");
	} else {
		printf("Failed to destroy list.\n");
	}

	// A data ja e libertadano codigo
	//free(data1);
	//free(data2);

	return 0;
}

int main() {

	//test_block();
	//test_entry();
	test_list();

	return 0;
}
*/
