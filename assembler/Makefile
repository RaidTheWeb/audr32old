#
#	Assembler Makefile
#

OUT = assemble
SOURCE_DIR = src
BIN_DIR = bin
BUILD_DIR = build
CC = gcc
CFLAGS = -g -O0 -I$(SOURCE_DIR)/include

HEADERS = $(wildcard $(SOURCE_DIR)/include/*.h)
SOURCES = $(wildcard $(SOURCE_DIR)/*.c)
OBJECTS = $(addprefix $(BUILD_DIR)/, $(notdir $(SOURCES:.c=.o)))

.PHONY: clean

all: $(BIN_DIR)/$(OUT)

$(BIN_DIR)/$(OUT): $(OBJECTS)
	@printf "%8s %-40s %s\n" $(CC) $@ "$(CFLAGS)"
	@mkdir -p $(BIN_DIR)
	@$(CC) $(CFLAGS) $^ -o $@
	@chmod +x $(BIN_DIR)/$(OUT)

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c $(HEADERS)
	@printf "%8s %-40s %s\n" $(CC) $< "$(CFLAGS)"
	@mkdir -p $(BUILD_DIR)/
	@$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -r bin
	rm -r build
