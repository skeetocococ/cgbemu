# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -O2
# Use SDL2 if available
SDL_CFLAGS := $(shell sdl2-config --cflags 2>/dev/null)
SDL_LDFLAGS := $(shell sdl2-config --libs 2>/dev/null)
CFLAGS += $(SDL_CFLAGS)
LDFLAGS = $(SDL_LDFLAGS)

# Directories
CPU_DIR = cpu
IO_DIR = io
DEBUG_DIR = debug
OBJ_DIR = obj

# Source files
SRCS = main.c \
       memory.c \
       $(CPU_DIR)/cpu.c \
       $(CPU_DIR)/instructions.c \
       $(CPU_DIR)/opcodes.c \
       $(IO_DIR)/ppu.c \
       $(IO_DIR)/joypad.c \
       $(DEBUG_DIR)/debug.c

# Object files (optional)
OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS))

# Executable
TARGET = gbemu

# Default: compile & link in one step (no .o files left)
all: $(TARGET)

$(TARGET):
	$(CC) $(CFLAGS) $(SRCS) -o $@ $(LDFLAGS)

DEPFLAGS = -MMD -MP

# Object files
OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS))

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

objects: $(OBJS)
	@echo "Object files created in $(OBJ_DIR)/"

# Optional link from object files
link: $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Clean
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean objects link
