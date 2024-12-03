SRC_DIR = source
OBJ_DIR = object
BIN_DIR = binary
INC_DIR = include

TEST_SRC_FILES = client_hashtable server_hashtable

MODULES = block entry list serialization table table-private htmessages.pb-c client_network client_stub server_network server_skeleton message-private stats server_network-private

TEST_EXECUTABLES = $(addprefix $(BIN_DIR)/, $(TEST_SRC_FILES))

OBJ_FILES = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(MODULES)))
TEST_OBJ_FILES = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(TEST_SRC_FILES)))

CC = gcc
CFLAGS = -Wall -g -I$(INC_DIR)

PROTOC = protoc-c

all: initialize_proto $(TEST_EXECUTABLES)

# Rule to generate .c and .h from .proto
initialize_proto: $(INC_DIR)/htmessages.proto
	@mkdir -p $(SRC_DIR) $(INC_DIR)
	$(PROTOC) --proto_path=$(INC_DIR) --c_out=$(SRC_DIR) $<
	mv $(SRC_DIR)/htmessages.pb-c.h $(INC_DIR)/htmessages.pb-c.h

# Rule to build the test executables
$(BIN_DIR)/%: $(OBJ_FILES) $(OBJ_DIR)/%.o
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ -lprotobuf-c

# Rule to compile .c files into .o files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure the .pb-c.c and .pb-c.h files are generated before compiling
$(OBJ_DIR)/client_network.o: $(SRC_DIR)/htmessages.pb-c.c $(INC_DIR)/htmessages.pb-c.h
$(OBJ_DIR)/server_network.o: $(SRC_DIR)/htmessages.pb-c.c $(INC_DIR)/htmessages.pb-c.h
$(OBJ_DIR)/client_stub.o: $(SRC_DIR)/htmessages.pb-c.c $(INC_DIR)/htmessages.pb-c.h
$(OBJ_DIR)/server_skeleton.o: $(SRC_DIR)/htmessages.pb-c.c $(INC_DIR)/htmessages.pb-c.h

# Rules to build the test executables
$(BIN_DIR)/client_hashtable: $(OBJ_FILES) $(OBJ_DIR)/client_hashtable.o
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ -lprotobuf-c

$(BIN_DIR)/server_hashtable: $(OBJ_FILES) $(OBJ_DIR)/server_hashtable.o
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ -lprotobuf-c

clean:
	rm -rf $(OBJ_DIR)/*.o $(BIN_DIR)/* $(Lib) $(SRC_DIR)/*.pb-c.c $(INC_DIR)/*.pb-c.h

.PHONY: all clean