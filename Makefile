SRCPATHS = src/*.c
SRC = $(wildcard $(SRCPATHS))
OBJECTS = $(SRC:src/%.c=build/%.o)

NAME = luna
CC = clang

ifeq ($(OS),Windows_NT)
	EXECUTABLE_NAME = $(NAME).exe
else
	EXECUTABLE_NAME = $(NAME)
endif

DEBUGFLAGS = -g -rdynamic -pg
ASANFLAGS = -fsanitize=undefined -fsanitize=address
DONTBEAFUCKINGIDIOT = -Werror -Wall -Wextra -pedantic -Wno-missing-field-initializers -Wno-unused-result
CFLAGS = -O3
SHUTTHEFUCKUP = -Wno-unknown-warning-option -Wno-incompatible-pointer-types-discards-qualifiers -Wno-initializer-overrides -Wno-discarded-qualifiers

#MD adds a dependency file, .d to the directory. the line at the bottom
#forces make to rebuild, if any dependences need it.
#e.g if comet.h changes, it forces a rebuild
#if core.c changes, it only rebuilds.

build/%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS) -MD $(SHUTTHEFUCKUP)

build: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXECUTABLE_NAME) $(CFLAGS) -MD
	@echo ""

test: build
	./$(EXECUTABLE_NAME) test/test.aphel -o:test/out.apo

debug:
	$(DEBUGFLAGS) $(DONTBEAFUCKINGIDIOT)

clean:
	rm -f build/*

-include $(OBJECTS:.o=.d)