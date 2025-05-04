# --------------------------------------------------
# Makefile – ZX-81 + novo core Z80
# --------------------------------------------------

SRCS   = zx81.c z80.c
CFLAGS = -std=c99 -Wall -O2 $(shell sdl-config --cflags)
LDLIBS = $(shell sdl-config --libs)

# alvo principal: compila o emulador
zx81: $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LDLIBS)

.PHONY: run clean

# compila (se necessário) e executa o emulador
run: zx81
	./zx81

# apaga o executável e ficheiros temporários
clean:
	rm -f zx81
