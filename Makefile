CC=/usr/local/opt/llvm/bin/clang++
CFLAGS=--std=c++17 -O3 -Werror -Wall

all: json.cc
	$(CC) $(CFLAGS) json.cc -o example.exe
