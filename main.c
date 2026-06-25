/* ==========================================================================
 *  main.c  -  Resolve um Cubo Magico 3x3 usando uma ARVORE de estados
 * ==========================================================================
 *
 *  Fluxo do programa:
 *      1. Le o arquivo de entrada (54 cores, na ordem do enunciado:
 *         Cima, Esquerda, Frente, Direita, Tras, Baixo).
 *      2. Valida o estado (54 cores, 9 de cada, centros distintos).
 *      3. Monta a ARVORE de estados (raiz = cubo embaralhado) e faz a busca
 *         em profundidade com aprofundamento iterativo.
 *      4. Imprime a quantidade de movimentos e a sequencia da solucao.
 *
 *  Compilacao:
 *      gcc main.c cube.c solver.c -o cubo
 *
 *  Execucao:
 *      ./cubo entrada.txt          resolve o cubo do arquivo
 *      ./cubo entrada.txt 8        resolve permitindo ate 8 movimentos
 *      ./cubo scramble 5           gera um cubo embaralhado com 5 giros
 *
 *  Tambem aceita a entrada digitada e redirecionada de um arquivo:
 *      ./cubo /dev/stdin   (e cole a string, ou use < arquivo)
 * ========================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "cube.h"
#include "solver.h"

/* String do cubo resolvido (centros: W O G R B Y). Usada so para gerar
 * embaralhamentos de demonstracao.                                           */
static const char *RESOLVIDO =
    "WWWWWWWWW" "OOOOOOOOO" "GGGGGGGGG" "RRRRRRRRR" "BBBBBBBBB" "YYYYYYYYY";

/* inverseMove: indice do movimento que desfaz 'm' (U<->U', etc.).            */
static int inverseMove(int m) { return (m % 2 == 0) ? m + 1 : m - 1; }

/* gerarScramble: parte do cubo resolvido e aplica 'n' movimentos aleatorios
 * (sem desfazer o anterior), gerando um cubo soluvel em ate 'n' giros. Mostra
 * a sequencia usada e imprime a string final, pronta para ser resolvida.     */
static void gerarScramble(int n)
{
    Cube c;
    int  i, ultimo = -1;

    initCube(&c, RESOLVIDO);
    srand((unsigned) time(NULL));

    printf("Embaralhando o cubo resolvido com %d movimento(s):\n  ", n);
    for (i = 0; i < n; i++) {
        int m;
        do {
            m = rand() % NUM_MOVES;
        } while (ultimo != -1 && m == inverseMove(ultimo));  /* nao desfaz     */
        applyMove(&c, &c, m);
        printf("%s ", MOVE_NAMES[m]);
        ultimo = m;
    }
    printf("\n\nString gerada (cole em um arquivo e resolva com ./cubo):\n%s\n", c.f);
}

/* lerEntrada: le o arquivo e extrai ate 54 caracteres de cor. Ignora espacos,
 * virgulas e quebras de linha (servem so de separadores) e linhas iniciadas
 * por '#' (comentarios). Assim aceita tanto a string unica do enunciado
 * quanto uma face por linha. Retorna o numero de cores lidas (ou -1 em erro). */
static int lerEntrada(const char *caminho, char *dest)
{
    FILE *fp = fopen(caminho, "r");
    int n = 0, ch;
    int inicioLinha = 1;
    const char *cores = "YWGBRO";

    if (!fp) {
        printf("Erro: nao foi possivel abrir o arquivo '%s'.\n", caminho);
        return -1;
    }
    while ((ch = fgetc(fp)) != EOF && n < NUM_FACELETS) {
        if (inicioLinha && ch == '#') {            /* linha de comentario      */
            while ((ch = fgetc(fp)) != EOF && ch != '\n')
                ;
            continue;
        }
        if (ch == '\n') { inicioLinha = 1; continue; }
        inicioLinha = 0;

        ch = toupper(ch);
        if (strchr(cores, ch))                     /* guarda so as cores       */
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
    No   *solucao;
    long  nosVisitados = 0;
    int   profMax = PROF_MAXIMA;

    /* --- 0. Argumentos da linha de comando ------------------------------- */
    if (argc < 2) {
        printf("Uso:\n");
        printf("  %s <arquivo_entrada> [prof_max]   resolve o cubo do arquivo\n", argv[0]);
        printf("  %s scramble <N>                   gera um embaralhamento de N giros\n", argv[0]);
        return 1;
    }

    /* --- 1. Inicializa movimentos e cores -------------------------------- */
    initMoves();
    cubeInitCores();   /* liga as cores ANSI se a saida for um terminal        */

    /* Modo gerador: ./cubo scramble N  -> cria um cubo soluvel em ate N giros */
    if (strcmp(argv[1], "scramble") == 0) {
        int n = (argc >= 3) ? atoi(argv[2]) : 5;
        if (n < 1) n = 1;
        gerarScramble(n);
        return 0;
    }

    /* Profundidade maxima opcional (segundo argumento).                       */
    if (argc >= 3) {
        int p = atoi(argv[2]);
        if (p > 0) profMax = p;
    }

    /* --- 2. Le e valida a entrada ---------------------------------------- */
    if (lerEntrada(argv[1], entrada) < 0)
        return 1;

    if (!initCube(&inicial, entrada)) {
        printf("Estado invalido do cubo.\n");
        printf("(Esperado: 54 cores entre Y W G B R O, 9 de cada, com os 6 centros diferentes.)\n");
        return 1;
    }

    printf("%s%s== Cubo Magico 3x3 - Solucionador por Arvore de Estados ==%s\n\n",
           ansi(A_BOLD), ansi(A_CYAN), ansi(A_RESET));
    printf("%sEstado inicial informado:%s\n\n", ansi(A_BOLD), ansi(A_RESET));
    printCube(&inicial);
    printf("\n%sString:%s %s\n\n", ansi(A_DIM), ansi(A_RESET), inicial.f);

    /* --- 3./4. Monta a arvore e busca a solucao -------------------------- */
    printf("%sBusca em profundidade (aprofundamento iterativo), ate %d movimentos:%s\n",
           ansi(A_BOLD), profMax, ansi(A_RESET));
    solucao = resolver(&inicial, profMax, &nosVisitados);

    printf("\nNos gerados na arvore: %s%ld%s\n", ansi(A_BOLD), nosVisitados, ansi(A_RESET));

    /* --- 5. Resultado ---------------------------------------------------- */
    if (solucao == NULL) {
        printf("\nNao foi possivel resolver em ate %d movimentos.\n", profMax);
        printf("A arvore cresce cerca de 12x por nivel, entao a busca em\n");
        printf("profundidade so alcanca cubos pouco embaralhados. Tente um\n");
        printf("limite maior (ex.: ./cubo %s %d) ou um embaralhamento menor.\n",
               argv[1], profMax + 1);
        return 0;
    }

    imprimirSolucao(solucao);
    liberarCaminho(solucao);
    return 0;
}
