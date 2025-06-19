SRC_DIR = src
OBJ_DIR = obj
INCLUDE_DIR = include
BUILD_DIR = build

CC = gcc
CFLAGS = -Wall -Wextra -I$(INCLUDE_DIR)

SRCS = $(wildcard src/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
TARGET = $(BUILD_DIR)/yash

all: $(TARGET)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CC) $(OBJS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(OBJ_DIR) $(BUILD_DIR)

run: $(TARGET)
	@./$(TARGET)

.PHONY: all clean