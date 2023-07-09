all: build

BUILD_INPATH = ./src
BUILD_OUTPATH = ./bin/luna
ifeq ($(OS),Windows_NT)
	BUILD_OUTPATH = ./bin/luna.exe
endif

BUILD_FLAGS = -o:speed -out:$(BUILD_OUTPATH)

STRESSTEST_INPATH = ./test/twomil.aphel
TEST_INPATH = ./test/test.aphel
TEST_FLAGS = -out:test/out.bin -debug -no-color # apparently make has trouble displaying ansi codes

build:
	@odin build $(BUILD_INPATH) $(BUILD_FLAGS)

test: build
	@$(BUILD_OUTPATH) $(TEST_INPATH) $(TEST_FLAGS)

stresstest: build
	@$(BUILD_OUTPATH) $(STRESSTEST_INPATH) $(TEST_FLAGS)