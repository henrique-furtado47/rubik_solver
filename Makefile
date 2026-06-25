# Makefile do solucionador de Cubo Magico por arvore de estados.
#
#   make          -> compila o executavel 'cubo'
#   make run      -> compila e executa com entrada.txt
#   make clean    -> remove o binario
#   make scramble -> gera um cubo embaralhado de exemplo (5 giros)

CC     = gcc
CFLAGS = -Wall -Wextra -O2
OBJ    = main.c cube.c solver.c

cubo: $(OBJ) cube.h solver.h
	$(CC) $(CFLAGS) $(OBJ) -o cubo

run: cubo
	./cubo entrada.txt

scramble: cubo
	./cubo scramble 5

clean:
	rm -f cubo
