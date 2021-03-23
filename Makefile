
NAME = chip-8

SRC_DIR   = src
BUILD_DIR = ./

# compiler
CC     = gcc
LIBS   = -lSDL2 -lpthread -lSDL2
DEPS   = src/cpu_instructions.h

CFLAGS = -std=c11 -Wall -Wextra -Werror -Wshadow -Wunreachable-code -fno-omit-frame-pointer -pedantic -O2
RFLAGS = -fno-stack-protector -s -ffunction-sections -fdata-sections -Wl,--gc-sections -fno-unwind-tables -fno-asynchronous-unwind-tables
DFLAGS = -DDEBUG -g -pg -fsanitize=address -fsanitize=undefined -fsanitize=leak

CFLAGS += $(RFLAGS)

LINK   = $(CC) -o $@ $^ $(CFLAGS) $(LIBS)

STRIP  = strip -S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag
# ----

# compute variables
SRC    = $(foreach sdir, $(SRC_DIR), $(wildcard $(sdir)/*.c))
OBJ    = $(patsubst %.c, %.o, $(SRC))
TARGET = $(BUILD_DIR)/$(NAME)

ifdef OS
LIBS += -mwindows
endif
# ----

.PHONY: clean

%.o: %.c $(DEPS)
	@echo "cc    $<"
	@$(CC) -o $@ -c $< $(CFLAGS)

$(TARGET): $(OBJ)
	@echo "link  $@"
	@$(LINK)
	@echo "strip $@"
	@$(STRIP) $@

clean:
	@rm -f $(OBJ) $(TARGET)
