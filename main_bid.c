/* ==========================================================================
 *  main_bid.c  -  Codigo 2: resolve o cubo por BUSCA BIDIRECIONAL + BST
 * ==========================================================================
 *
 *  Diferenca para o Codigo 1 (main.c): la usamos UMA arvore descendo ate ~7-8
 *  movimentos (e uma gulosa de reserva). Aqui usamos DUAS arvores que se
 *  encontram no meio, com uma Arvore Binaria de Busca (BST) casando os estados.
 *  Isso resolve cubos bem mais embaralhados (ate ~12 movimentos) em segundos e
 *  com solucao MINIMA. Veja solver_bid.h / bst.h.
 *
 *  Compilacao (junto com o cubo do Codigo 1):
 *      make            -> gera 'cubo' (Codigo 1) e 'cubo_bid' (Codigo 2)
 *
 *  Execucao:
 *      ./cubo_bid entrada.txt                 frente=6, tras=6  (ate 12 mov.)
 *      ./cubo_bid entrada.txt 7 5             frente=7, tras=5  (menos memoria)
 * ========================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cube.h"
#include "solver_bid.h"

/* lerEntrada: igual a do Codigo 1. Le o arquivo e extrai ate 54 cores,
 * ignorando espacos/virgulas/quebras de linha e linhas comecadas por '#'.     */
static int lerEntrada(const char *caminho, char *dest)
{
    FILE *fp = fopen(caminho, "r");
    int   n = 0, ch, inicioLinha = 1;
    const char *cores = "YWGBRO";

    if (!fp) {
        printf("Erro: nao foi possivel abrir o arquivo '%s'.\n", caminho);
        return -1;
    }
    while ((ch = fgetc(fp)) != EOF && n < NUM_FACELETS) {
        if (inicioLinha && ch == '#') {
            while ((ch = fgetc(fp)) != EOF && ch != '\n')
                ;
            continue;
        }
        if (ch == '\n') { inicioLinha = 1; continue; }
        inicioLinha = 0;

        ch = toupper(ch);
        if (strchr(cores, ch))
            dest[n++] = (char) ch;
    }
    fclose(fp);
    dest[n] = '\0';
    return n;
}

int main(int argc, char *argv[])
{
    char       entrada[128];
    Cube       inicial;
    SolucaoBid sol;
    int        profFrente = 6, profTras = 6;   /* metade para cada lado         */

    if (argc < 2) {
        printf("Uso:\n");
        printf("  %s <arquivo_entrada> [profFrente] [profTras]\n", argv[0]);
        printf("  (padrao: frente=6, tras=6 -> resolve ate 12 movimentos)\n");
        return 1;
    }

    initMoves();
    cubeInitCores();

    if (argc >= 3) { int p = atoi(argv[2]); if (p > 0) profFrente = p; }
    if (argc >= 4) { int p = atoi(argv[3]); if (p > 0) profTras   = p; }

    if (lerEntrada(argv[1], entrada) < 0)
        return 1;

    if (!initCube(&inicial, entrada)) {
        printf("Estado invalido do cubo.\n");
        printf("(Esperado: 54 cores entre Y W G B R O, 9 de cada, centros diferentes.)\n");
        return 1;
    }

    printf("%s%s== Cubo Magico 3x3 - Solucionador BIDIRECIONAL (com BST) ==%s\n\n",
           ansi(A_BOLD), ansi(A_CYAN), ansi(A_RESET));
    printf("%sEstado inicial informado:%s\n\n", ansi(A_BOLD), ansi(A_RESET));
    printCube(&inicial);
    printf("\n%sString:%s %s\n\n", ansi(A_DIM), ansi(A_RESET), inicial.f);

    printf("%sBusca bidirecional: frente ate %d e tras ate %d "
           "(solucao de ate %d movimentos)%s\n",
           ansi(A_BOLD), profFrente, profTras, profFrente + profTras, ansi(A_RESET));
    printf("  construindo a arvore de tras na BST e procurando o encontro...\n");

    resolverBidirecional(&inicial, profFrente, profTras, &sol);

    imprimirSolucaoBid(&inicial, &sol);
    return 0;
}
