#
#	Emulator Makefile
#

OUT = vm
SOURCE_DIR = src
BIN_DIR = bin
BUILD_DIR = build

CC = gcc
AS = nasm
CFLAGS = -g -O0 -I$(SOURCE_DIR)/include -lSDL2

HEADERS = $(wildcard $(SOURCE_DIR)/include/*.h)
SOURCES = $(shell find $(SOURCE_DIR) -type f -name '*.c')
ASMSOURCES = $(shell find $(SOURCE_DIR) -type f -name '*.asm')
OBJECTS = $(addprefix $(BUILD_DIR)/, $(notdir $(SOURCES:.c=.o))) $(addprefix $(BUILD_DIR)/, $(notdir $(ASMSOURCES:.asm=.asm.o)))

.PHONY: clean

all: $(BIN_DIR)/$(OUT)

$(BIN_DIR)/$(OUT): $(OBJECTS)
	@printf "%8s %-40s %s\n" $(CC) $@ "$(CFLAGS)"
	@mkdir -p $(BIN_DIR)
	@$(CC) $(CFLAGS) $^ -o $@
	@chmod +x $(BIN_DIR)/$(OUT)

$(BUILD_DIR)/%.asm.o: $(SOURCE_DIR)/%.asm
	@printf "%9s %-39s %s\n" $(AS) $< "-I$(SOURCE_DIR)/include -f elf64 -o $@"
	@mkdir -p $(BUILD_DIR)/
	@$(AS) $< -I$(SOURCE_DIR)/include -f elf64 -o $@	

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c $(HEADERS)
	@printf "%8s %-40s %s\n" $(CC) $< "$(CFLAGS)"
	@mkdir -p $(BUILD_DIR)/
	@$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -r bin
	rm -r build
