# Makefile do solucionador de Cubo Magico por arvore de estados.
#
#   make          -> compila o executavel 'cubo'
#   make run      -> compila e executa com entrada.txt (BFS)
#   make astar    -> compila e executa com entrada.txt (A*)
#   make clean    -> remove binarios e arquivos gerados

CC     = gcc
CFLAGS = -Wall -Wextra -O2
OBJ    = main.c cube.c solver.c graphviz.c

cubo: $(OBJ) cube.h solver.h graphviz.h
	$(CC) $(CFLAGS) $(OBJ) -o cubo

run: cubo
	./cubo entrada.txt

astar: cubo
	./cubo entrada.txt astar

clean:
	rm -f cubo test_moves solution.dot saida.png
