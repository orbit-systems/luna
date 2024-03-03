SRCPATHS = src/*.c
SRC = $(wildcard $(SRCPATHS))
OBJECTS = $(SRC:src/%.c=build/%.o)

NAME = luna
CC = gcc
LD = gcc

ifeq ($(OS),Windows_NT)
	EXECUTABLE_NAME = $(NAME).exe
else
	EXECUTABLE_NAME = $(NAME)
endif

DEBUGFLAGS = -g -rdynamic -pg
ASANFLAGS = -fsanitize=undefined -fsanitize=address
DONTBEAFUCKINGIDIOT = -Werror -Wall -Wextra -pedantic -Wno-missing-field-initializers -Wno-unused-result
CFLAGS = -O3 -flto
SHUTTHEFUCKUP = -Wno-unknown-warning-option -Wno-incompatible-pointer-types-discards-qualifiers -Wno-initializer-overrides -Wno-discarded-qualifiers

#MD adds a dependency file, .d to the directory. the line at the bottom
#forces make to rebuild, if any dependences need it.
#e.g if comet.h changes, it forces a rebuild
#if core.c changes, it only rebuilds.

all: build

build/%.o: src/%.c
	@echo compiling $<
	@$(CC) -c -o $@ $< $(CFLAGS) -MD

build: $(OBJECTS)
	@echo linking with $(LD)
	@$(CC) $(OBJECTS) -o $(EXECUTABLE_NAME) -lm -flto
	@echo $(EXECUTABLE_NAME) built
	
clean:
	@rm -rf build
	@mkdir build

printbuildinfo:
	@echo using $(CC) with flags $(CFLAGS)

new: clean printbuildinfo build

-include $(OBJECTS:.o=.d)