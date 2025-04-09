CFLAGS=-Wall -Wextra -std=c17 -pedantic -ggdb `pkg-config --cflags sdl2` -Wno-gnu-zero-variadic-macro-arguments
LIBS=`pkg-config --libs sdl2` -lm `pkg-config --libs raylib`

SRCS=main.c nes6502.c
HEADERS=nes6502.h

znes: Makefile $(SRCS) $(HEADERS)
	clang $(CFLAGS) -o znes $(SRCS) $(LIBS)

format:
	clang-format -i $(SRCS) $(HEADERS)

clean:
	rm -f znes

run: znes
	./znes ./rom/smb.nes