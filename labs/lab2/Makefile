CC = g++

CFLAGS_COMMON = -pedantic -Wall -std=c++11
CFLAGS_RELEASE = $(CFLAGS_COMMON) -O2
CFLAGS_TEST = $(CFLAGS_COMMON) -O2

DIR_SRC = src
DIR_INCLUDE = include
DIR_TESTS = tests
DIR_EXTERNAL = external
DIR_DOCTEST = $(DIR_EXTERNAL)/doctest
DIR_BUILD = build

SOURCE_TESTS = $(wildcard $(DIR_TESTS)/*.cpp)
BIN_TESTS = $(DIR_BUILD)/test

all: test

$(DIR_BUILD):
	mkdir -p $(DIR_BUILD)

clean:
	rm -rf $(DIR_BUILD)

ser:
	$(CC) $(CFLAGS_RELEASE) $(DIR_SRC)/serialize.cpp -I$(DIR_INCLUDE) -I$(DIR_DOCTEST) -o $(DIR_BUILD)/serialize

deser:
	$(CC) $(CFLAGS_RELEASE) $(DIR_SRC)/deserialize.cpp -I$(DIR_INCLUDE) -I$(DIR_DOCTEST) -o $(DIR_BUILD)/deserialize

man: $(DIR_BUILD) ser deser

test: $(BIN_TESTS)
	./$(BIN_TESTS)

$(BIN_TESTS): $(DIR_BUILD) $(SOURCE_TESTS)
	$(CC) $(CFLAGS_TEST) $(SOURCE_TESTS) -I$(DIR_INCLUDE) -I$(DIR_DOCTEST) -o $@

.PHONY: all test clean