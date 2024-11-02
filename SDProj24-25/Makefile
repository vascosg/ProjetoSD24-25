SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = binary
INC_DIR = include

TEST_SRC_FILES = client_hashtable server_hashtable

MODULES = block entry list serialization table table-private client_hashtable client_network client_stub server_hashtable server_network server_skeleton htmessages.pb-c message-private

TEST_EXECUTABLES = $(addprefix $(BIN_DIR)/, $(TEST_SRC_FILES))

OBJ_FILES = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(MODULES)))
TEST_OBJ_FILES = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(TEST_SRC_FILES)))

CC = gcc
CFLAGS = -Wall -g -I$(INC_DIR) 

all: $(TEST_EXECUTABLES)

$(BIN_DIR)/%: $(OBJ_FILES) $(OBJ_DIR)/%.o
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -lprotobuf-c -o $@

$(OBJ_FILES): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -lprotobuf-c -o $@

$(TEST_OBJ_FILES): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)/*.o $(BIN_DIR)/*

.PHONY: all clean
