all: build

BUILD_INPATH = ./src
BUILD_FLAGS = -o:speed -out:./bin/luna

STRESSTEST_INPATH = ./test/twomil.aphel
TEST_INPATH = ./test/test.aphel
TEST_FLAGS = -out:test/out.bin -debug -no-color # apparently make has trouble displaying ansi codes

build:
	@odin build $(BUILD_INPATH) $(BUILD_FLAGS)

test: build
	@./bin/luna $(TEST_INPATH) $(TEST_FLAGS)

stresstest: build
	@./bin/luna $(STRESSTEST_INPATH) $(TEST_FLAGS)

	T:\vscode\comet\test\fibonacci.aphel