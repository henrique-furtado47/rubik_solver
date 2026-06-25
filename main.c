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
#include <time.h>
#include "cube.h"
#include "solver.h"
#include "graphviz.h"

/* String do cubo resolvido (centros: W O G R B Y). */
static const char *RESOLVIDO =
    "WWWWWWWWW" "OOOOOOOOO" "GGGGGGGGG" "RRRRRRRRR" "BBBBBBBBB" "YYYYYYYYY";

/* inverseMove: indice do movimento que desfaz 'm' (U<->U', etc.).            */
static int inverseMove(int m) { return (m % 2 == 0) ? m + 1 : m - 1; }

/* gerarScramble: parte do cubo resolvido e aplica 'n' movimentos aleatorios
 * (sem desfazer o anterior), garantindo um cubo SOLUVEL em ate 'n' giros.
 * Grava o resultado em entrada.csv e mostra a sequencia usada.               */
static void gerarScramble(int n)
{
    Cube c;
    int  i, ultimo = -1, face;
    FILE *fp;

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
    printf("\n\nString gerada:\n%s\n\n", c.f);

    /* grava em entrada.csv (uma face por linha)                              */
    fp = fopen("entrada.csv", "w");
    if (fp) {
        const char *nomes[6] = {"Cima","Esquerda","Frente","Direita","Tras","Baixo"};
        fprintf(fp, "# Embaralhamento gerado automaticamente (%d movimentos).\n", n);
        fprintf(fp, "# Ordem das faces: Cima, Esquerda, Frente, Direita, Tras, Baixo.\n");
        for (face = 0; face < 6; face++) {
            int k;
            fprintf(fp, "# %s\n", nomes[face]);
            for (k = 0; k < 9; k++)
                fprintf(fp, "%c%s", c.f[face*9 + k], (k < 8) ? "," : "\n");
        }
        fclose(fp);
        printf("Arquivo 'entrada.csv' gerado. Agora resolva com:\n");
        printf("  ./cubo entrada.csv\n");
    }
}

/* Le o arquivo e extrai ate 54 caracteres de cor. Aceita tanto a string unica
 * quanto o formato CSV (cores separadas por virgula e/ou quebras de linha):
 *   - virgulas, espacos e quebras de linha sao IGNORADOS (servem so de
 *     separadores);
 *   - linhas que comecam com '#' sao COMENTARIOS e tambem sao ignoradas
 *     (uteis para rotular as faces no proprio arquivo .csv).
 * Retorna o numero de caracteres de cor lidos.                               */
static int lerEntrada(const char *caminho, char *dest)
{
    FILE *fp = fopen(caminho, "r");
    int n = 0, ch;
    int inicioLinha = 1;            /* estamos no comeco de uma linha?         */
    const char *cores = "YWGBRO";

    if (!fp) {
        printf("Erro: nao foi possivel abrir o arquivo '%s'.\n", caminho);
        return -1;
    }
    while ((ch = fgetc(fp)) != EOF && n < NUM_FACELETS) {
        /* Linha de comentario: começa com '#'. Pula tudo ate a quebra.        */
        if (inicioLinha && ch == '#') {
            while ((ch = fgetc(fp)) != EOF && ch != '\n')
                ;                   /* descarta o resto da linha               */
            continue;               /* inicioLinha continua 1                  */
        }
        if (ch == '\n') { inicioLinha = 1; continue; }
        inicioLinha = 0;

        ch = toupper(ch);
        if (strchr(cores, ch))      /* guarda apenas caracteres de cor         */
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
    int   algoritmo = 0;   /* 0 = BFS, 1 = A*, 2 = BFS bidirecional           */

    /* --- 0. Argumentos da linha de comando ------------------------------- */
    if (argc < 2) {
        printf("Uso:\n");
        printf("  %s <arquivo_entrada> [astar]   resolve o cubo do arquivo\n", argv[0]);
        printf("  %s scramble <N>                gera um embaralhamento de N giros\n", argv[0]);
        return 1;
    }

    /* --- 1. Inicializa as tabelas de movimento --------------------------- */
    initMoves();

    /* Modo gerador: ./cubo scramble N  -> cria um cubo soluvel em ate N giros */
    if (strcmp(argv[1], "scramble") == 0) {
        int n = (argc >= 3) ? atoi(argv[2]) : 5;
        if (n < 1)  n = 1;
        if (n > 7)  printf("Aviso: N grande pode estourar o limite da busca (use N<=7).\n");
        gerarScramble(n);
        return 0;
    }

    if (argc >= 3 && strcmp(argv[2], "astar") == 0) algoritmo = 1;
    if (argc >= 3 && strcmp(argv[2], "bidir") == 0) algoritmo = 2;

    /* --- 2. Le e valida a entrada ---------------------------------------- */
    if (lerEntrada(argv[1], entrada) < 0)
        return 1;

    if (!initCube(&inicial, entrada)) {
        /* Tamanho diferente de 54 ou contagem de cores incorreta.            */
        printf("Estado invalido do cubo.\n");
        return 1;
    }

    /* Mesmo com 9 de cada cor, a montagem pode ser fisicamente impossivel.   */
    if (!isSolvable(&inicial)) {
        printf("Estado invalido do cubo.\n");
        return 1;
    }

    printf("=== Cubo Magico 3x3 - Solucionador por Arvore de Estados ===\n\n");
    printf("Estado inicial informado:\n");
    printCube(&inicial);
    printf("\nString: %s\n\n", inicial.f);

    /* --- 3. Cria a arvore e --- 4. busca a solucao ----------------------- */
    arvore = createTree(&inicial);

    if (algoritmo == 1) {
        printf("Algoritmo de busca: A* (heuristica admissivel)\n\n");
        solucao = astarSolve(arvore, &inicial);
    } else if (algoritmo == 2) {
        printf("Algoritmo de busca: BFS bidirecional\n\n");
        solucao = biBfsSolve(arvore, &inicial);
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
