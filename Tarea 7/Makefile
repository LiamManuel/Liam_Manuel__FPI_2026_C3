# Gestion de prestamos de vehiculos
# Uso con MinGW / Code::Blocks: mingw32-make
# Uso con GNU Make en Linux/macOS: make

CC      = gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -Wpedantic -O2

ifeq ($(OS),Windows_NT)
EXE := .exe
RM  := del /Q
RUN :=
else
EXE :=
RM  := rm -f
RUN := ./
endif

BIN     := gestion$(EXE)
PRUEBA  := checks$(EXE)
FUENTES := $(wildcard src/*.c)
NUCLEO  := src/archivos.c src/clientes.c src/prestamos.c src/reportes.c \
           src/tui.c src/util.c
CABEZAS := src/model.h src/archivos.h src/clientes.h src/prestamos.h \
           src/reportes.h src/tui.h src/util.h

all: $(BIN)

$(BIN): $(FUENTES) $(CABEZAS)
	$(CC) $(CFLAGS) $(FUENTES) -o $(BIN)

$(PRUEBA): tests/checks.c $(NUCLEO) $(CABEZAS)
	$(CC) $(CFLAGS) -Isrc tests/checks.c $(NUCLEO) -o $(PRUEBA)

test: $(PRUEBA)
	$(RUN)$(PRUEBA)

clean:
	-$(RM) $(BIN)
	-$(RM) $(PRUEBA)

.PHONY: all test clean
