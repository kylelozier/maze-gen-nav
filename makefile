# Variables
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
SRC = $(wildcard $(SRC_DIR)/*.c)  # Get all .c files in src directory
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))  # Replace .c with .o in obj directory

# Compiler
CC = gcc
CFLAGS = -Wall -g
INCLUDE_FLAGS := -Iraylib -I/usr/local/lib
LINKER_FLAGS := -g -L/usr/local/lib -lraylib
DEFINES := -D_DEBUG #-DCEXPORT -D_CRT_SECURE_NO_WARNINGS

# Rule to build the object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)  # Ensure the obj directory exists
	@$(CC) $(CFLAGS) -c $< -o $@ $(DEFINES) $(INCLUDE_FLAGS)

 # Rule to build the executable

TARGET = my_program.sh

$(BIN_DIR)/$(TARGET): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJ) -o $(BIN_DIR)/$(TARGET) $(LINKER_FLAGS)

# Clean rule
.PHONY: clean
clean:
	@rm -rf $(OBJ_DIR)/*.o
	@rm -rf $(BIN_DIR)/$(TARGET)
	@rm -rf $(BIN_DIR)/*.ilk
	@rm -rf $(BIN_DIR)/*.pdb

.PHONY: run
run:
	./bin/$(TARGET)
