# Makefile do solucionador de Cubo Magico por arvore de estados.
#
#   make          -> compila os dois executaveis: 'cubo' e 'cubo_bid'
#   make cubo     -> Codigo 1 (arvore unica + poda + gulosa; limite ~7-8 mov.)
#   make cubo_bid -> Codigo 2 (busca bidirecional + BST; resolve ate ~12 mov.)
#   make run      -> compila e executa o Codigo 1 com entrada.txt
#   make clean    -> remove os binarios
#   make scramble -> gera um cubo embaralhado de exemplo (5 giros)

CC      = gcc
CFLAGS  = -Wall -Wextra -O2

# Codigo 1: arvore unica (sem BST)
OBJ1    = main.c cube.c solver.c
# Codigo 2: busca bidirecional (com BST) -- compartilha o modelo do cubo
OBJ2    = main_bid.c cube.c solver_bid.c bst.c

all: cubo cubo_bid

cubo: $(OBJ1) cube.h solver.h
	$(CC) $(CFLAGS) $(OBJ1) -o cubo

cubo_bid: $(OBJ2) cube.h solver_bid.h bst.h
	$(CC) $(CFLAGS) $(OBJ2) -o cubo_bid

run: cubo
	./cubo entrada.txt

scramble: cubo
	./cubo scramble 5

clean:
	rm -f cubo cubo_bid
