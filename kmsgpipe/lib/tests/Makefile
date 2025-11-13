ROOT_DIR := ../..
SRC_DIR  := $(ROOT_DIR)/lib/src
UNITY_DIR := $(ROOT_DIR)/third_party/unity/src

# Files
TARGET := test_kmsgpipe
SRC := $(SRC_DIR)/kmsgpipe.c
TEST_SRC := test_kmsgpipe.c
UNITY_SRC := $(UNITY_DIR)/unity.c
INCLUDES := -I$(ROOT_DIR)/include

# Compiler flags
CC := gcc
CFLAGS := -Wall -Wextra -I$(SRC_DIR) -I$(UNITY_DIR) $(INCLUDES) -g
LDFLAGS :=

# Default target
all: $(TARGET)

$(TARGET): $(SRC) $(TEST_SRC) $(UNITY_SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test: $(TARGET)
	@echo "Running tests..."
	@./$(TARGET)

clean:
	rm -f $(TARGET)