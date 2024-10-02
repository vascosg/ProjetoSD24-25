# Directories
SRC_DIR = source
OBJ_DIR = object
BIN_DIR = binary
INC_DIR = include

# Test source files (with main functions)
TEST_SRC_FILES = test_block test_entry test_list test_serialization test_table

# Object files for each module (without main)
MODULES = block entry list serialization table table-private

# Executable names (binary)
TEST_EXECUTABLES = $(addprefix $(BIN_DIR)/, $(TEST_SRC_FILES))

# Object files for modules
OBJ_FILES = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(MODULES)))

# Compiler and flags
CC = gcc
CFLAGS = -Wall -I$(INC_DIR) # Include header files

# Targets
all: $(TEST_EXECUTABLES)

# Rule to create each test executable
$(BIN_DIR)/%: $(OBJ_FILES) $(OBJ_DIR)/%.o
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Compile object files for each source file in the module
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -rf $(OBJ_DIR)/*.o $(BIN_DIR)/*

# Prevent make from considering these as file targets
.PHONY: all clean

