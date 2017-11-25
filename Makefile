CC=/usr/local/opt/llvm/bin/clang++
CFLAGS=--std=c++17 -Werror -Wall
INCLUDES=-I include -I third_party

SOURCES=json_parser
LIB=build/json_parser.a

TESTS=simple acceptance

all: lib

lib: $(addprefix build/, $(addsuffix .o, $(SOURCES)))
	ar rvs $(LIB) build/*.o

test: $(addprefix run-, $(TESTS))

run-%: build/%.test
	$<

%.o: %.cc
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

build/%.o: src/%.cc
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

build/%.test: test/src/%.cc lib
	$(CC) $(CFLAGS) $(INCLUDES) $< $(LIB) -o $@

.PHONY: clean
clean:
	rm -rf build/*.o build/*.a build/*.test
