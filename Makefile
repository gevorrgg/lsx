# Build configuration for lsx.
# Targets: all (default), clean, re (clean + all).

CC = gcc
# -Ivector/include and -I./include let sources use <file_data.h> and <ls/...>.
CFLAGS = -g -Wall -Wextra -std=gnu11 -Ivector/include -I./include

LDLIBS = -lm          # round() in print_utils.c

SRC = main.c \
      vector/src/file_vector.c \
      vector/src/file_data.c \
      src/flags/flag_parser.c \
      src/flags/flag_setters.c \
      src/print/print.c \
      src/sorting.c \
      src/file_scan.c \
      src/print/format_info.c \
      src/ls.c \
      src/flags/flags.c \
      src/print/print_utils.c \
	  src/print/print_long.c \
	  src/print/print_columns.c \
	  src/print/print_horizontal.c \
	  src/print/print_one_per_line.c \
	  src/print/print_standard.c \
	  src/print/print_commas.c 

# Objects are placed under build/ mirroring the source tree.
OBJ = $(patsubst %.c,build/%.o,$(SRC))

TARGET = lsx

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDLIBS)

# Compile one source; create the build subdirectory on demand.
build/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build $(TARGET)

re: clean all

# Generate HTML documentation into docs/html (requires doxygen).
docs:
	doxygen Doxyfile

.PHONY: all clean re docs
