CC=/usr/local/opt/llvm/bin/clang++
CFLAGS=--std=c++17 -O3 -Werror -Wall -I ~/Downloads/boost_1_65_1 -stdlib=libc++

all: json.cc
	$(CC) $(CFLAGS) json.cc -o json
