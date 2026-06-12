/* ==========================================================================
 *  main.c  -  Programa principal: resolve um Cubo Magico 3x3 usando ARVORES
 * ==========================================================================
 *
 *  Fluxo do programa:
 *      1. Le o arquivo de entrada (54 cores em uma unica string).
 *      2. Valida o estado (cores e quantidades). Se invalido -> mensagem.
 *      3. Cria a ARVORE de estados com a raiz = estado informado.
 *      4. Executa a busca (BFS por padrao; A* opcional).
 *      5. Imprime: quantidade de movimentos, sequencia e arvore da solucao.
 *      6. Exporta o arquivo solution.dot (Graphviz).
 *
 *  Compilacao:
 *      gcc main.c cube.c solver.c graphviz.c -o cubo
 *
 *  Execucao:
 *      ./cubo entrada.txt           (usa BFS, o padrao)
 *      ./cubo entrada.txt astar     (usa a busca A*)
 * ========================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cube.h"
#include "solver.h"
#include "graphviz.h"

/* Le o arquivo e extrai ate 54 caracteres de cor (ignora espacos e quebras
 * de linha). Retorna o numero de caracteres de cor lidos.                    */
static int lerEntrada(const char *caminho, char *dest)
{
    FILE *fp = fopen(caminho, "r");
    int n = 0, ch;
    const char *cores = "YWGBRO";

    if (!fp) {
        printf("Erro: nao foi possivel abrir o arquivo '%s'.\n", caminho);
        return -1;
    }
    while ((ch = fgetc(fp)) != EOF && n < NUM_FACELETS) {
        ch = toupper(ch);
        if (strchr(cores, ch))          /* guarda apenas caracteres de cor    */
            dest[n++] = (char) ch;
    }
    fclose(fp);
    dest[n] = '\0';
    return n;
}

int main(int argc, char *argv[])
{
    char  entrada[128];
    Cube  inicial;
    Tree *arvore;
    Node *solucao;
    int   usarAstar = 0;

    /* --- 0. Argumentos da linha de comando ------------------------------- */
    if (argc < 2) {
        printf("Uso: %s <arquivo_entrada> [astar]\n", argv[0]);
        return 1;
    }
    if (argc >= 3 && strcmp(argv[2], "astar") == 0)
        usarAstar = 1;

    /* --- 1. Inicializa as tabelas de movimento --------------------------- */
    initMoves();

    /* --- 2. Le e valida a entrada ---------------------------------------- */
    if (lerEntrada(argv[1], entrada) < 0)
        return 1;

    if (!initCube(&inicial, entrada)) {
        /* Tamanho diferente de 54 ou contagem de cores incorreta.            */
        printf("Estado invalido do cubo.\n");
        return 1;
    }

    printf("=== Cubo Magico 3x3 - Solucionador por Arvore de Estados ===\n\n");
    printf("Estado inicial informado:\n");
    printCube(&inicial);
    printf("\nString: %s\n\n", inicial.f);

    /* --- 3. Cria a arvore e --- 4. busca a solucao ----------------------- */
    arvore = createTree(&inicial);

    if (usarAstar) {
        printf("Algoritmo de busca: A* (heuristica admissivel)\n\n");
        solucao = astarSolve(arvore, &inicial);
    } else {
        printf("Algoritmo de busca: BFS (busca em largura)\n\n");
        solucao = bfsSolve(arvore, &inicial);
    }

    printf("Nos gerados na arvore: %d\n\n", arvore->count);

    /* --- 5. Resultado ---------------------------------------------------- */
    if (solucao == NULL) {
        /* A entrada e valida em cores, mas a solucao nao foi achada dentro
         * dos limites de profundidade/memoria definidos no solver.           */
        printf("Nao foi possivel resolver dentro do limite de movimentos.\n");
        printf("(Aumente MAX_DEPTH em solver.c para embaralhamentos maiores.)\n");
        freeTree(arvore);
        return 0;
    }

    printSolution(solucao);

    /* --- 6. Exporta para Graphviz ---------------------------------------- */
    exportGraphviz(solucao, "solution.dot");

    freeTree(arvore);
    return 0;
}
