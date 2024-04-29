CFLAGS=-Wall -Wextra -std=c17 -pedantic -ggdb `pkg-config --cflags sdl2` -Wno-gnu-zero-variadic-macro-arguments
LIBS=`pkg-config --libs sdl2` -lm

SRCS=main.c

znes: Makefile $(SRCS)
	clang $(CFLAGS) -o znes $(SRCS) $(LIBS)

format:
	clang-format -i $(SRCS)

clean:
	rm -f znes

run: znes
	./znes ./rom/smb.nes