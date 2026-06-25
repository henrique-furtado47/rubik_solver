/* ==========================================================================
 *  cube.c  -  Implementacao das operacoes do Cubo Magico 3x3
 * ==========================================================================
 *
 *  O ponto delicado deste arquivo e descrever CORRETAMENTE como cada um dos
 *  6 movimentos basicos (giro de 90 graus no sentido horario) embaralha os
 *  54 adesivos. Fazemos isso atraves de "ciclos de permutacao".
 *
 *  Um ciclo (a b c d) significa: o conteudo do adesivo 'a' vai para 'b',
 *  o de 'b' vai para 'c', o de 'c' vai para 'd' e o de 'd' volta para 'a'.
 *
 *  Cada movimento e composto por:
 *    - 2 ciclos que giram os 8 adesivos da PROPRIA face (4 cantos + 4 meios);
 *    - 3 ciclos que movem os adesivos das faces VIZINHAS.
 *
 *  A partir desses ciclos montamos, em initMoves(), uma tabela de permutacao
 *  perm[54] para cada movimento, onde:  novo[i] = antigo[ perm[i] ].
 *
 *  Os movimentos "linha" (U', D', ...) sao simplesmente a permutacao inversa
 *  do movimento horario correspondente.
 * ========================================================================== */

#include <stdio.h>
#include <string.h>
#include "cube.h"

const char *MOVE_NAMES[NUM_MOVES] = {
    "U", "U'", "D", "D'", "L", "L'", "R", "R'", "F", "F'", "B", "B'"
};

/* Tabela de permutacao de cada um dos 12 movimentos.
 * gMovePerm[m][i] = indice de onde vem a cor da posicao i apos o movimento m. */
static int gMovePerm[NUM_MOVES][NUM_FACELETS];

/* --------------------------------------------------------------------------
 *  Definicao dos ciclos dos 6 movimentos basicos (sentido horario).
 *  Cada movimento tem 5 ciclos de 4 elementos:
 *      ciclos[0], ciclos[1] -> giro dos adesivos da propria face;
 *      ciclos[2], ciclos[3], ciclos[4] -> adesivos das faces vizinhas.
 *
 *  Estes numeros foram derivados a partir de um modelo geometrico do cubo
 *  (coordenadas 3D de cada adesivo) e validados por testes automaticos
 *  (ver verificacao: aplicar um movimento 4x deve voltar ao estado original).
 * -------------------------------------------------------------------------- */
#define NUM_CYCLES 5
static const int BASE_CYCLES[6][NUM_CYCLES][4] = {
    /* U (Cima, horario) */
    { {0,2,8,6}, {1,5,7,3}, {18,9,36,27}, {19,10,37,28}, {20,11,38,29} },
    /* D (Baixo, horario) */
    { {45,47,53,51}, {46,50,52,48}, {24,33,42,15}, {25,34,43,16}, {26,35,44,17} },
    /* L (Esquerda, horario) */
    { {9,11,17,15}, {10,14,16,12}, {0,18,45,44}, {3,21,48,41}, {6,24,51,38} },
    /* R (Direita, horario) */
    { {27,29,35,33}, {28,32,34,30}, {20,2,42,47}, {23,5,39,50}, {26,8,36,53} },
    /* F (Frente, horario) */
    { {18,20,26,24}, {19,23,25,21}, {6,27,47,17}, {7,30,46,14}, {8,33,45,11} },
    /* B (Tras, horario) */
    { {36,38,44,42}, {37,41,43,39}, {0,15,53,29}, {1,12,52,32}, {2,9,51,35} }
};

/* Para cada movimento basico (0=U,1=D,2=L,3=R,4=F,5=B), o indice horario
 * e o indice "linha" (anti-horario) dentro do vetor MOVE_NAMES.              */
static const int CW_INDEX[6]    = { 0, 2, 4, 6, 8, 10 };  /* U  D  L  R  F  B  */
static const int PRIME_INDEX[6] = { 1, 3, 5, 7, 9, 11 };  /* U' D' L' R' F' B' */

/* --------------------------------------------------------------------------
 *  initMoves: constroi as 12 tabelas de permutacao a partir dos ciclos.
 * -------------------------------------------------------------------------- */
void initMoves(void)
{
    int m, c, k;

    for (m = 0; m < 6; m++) {
        int cw    = CW_INDEX[m];
        int prime = PRIME_INDEX[m];

        /* 1) Comeca com a permutacao identidade (cada posicao vem dela mesma) */
        for (k = 0; k < NUM_FACELETS; k++)
            gMovePerm[cw][k] = k;

        /* 2) Aplica cada ciclo. Para o ciclo (a b c d) temos
         *       novo[b]=antigo[a], novo[c]=antigo[b], ...
         *    ou seja perm[ proximo ] = atual.                                 */
        for (c = 0; c < NUM_CYCLES; c++) {
            for (k = 0; k < 4; k++) {
                int atual    = BASE_CYCLES[m][c][k];
                int proximo  = BASE_CYCLES[m][c][(k + 1) % 4];
                gMovePerm[cw][proximo] = atual;
            }
        }

        /* 3) O movimento "linha" e a permutacao inversa do horario:
         *       se novo[proximo]=antigo[atual] no horario,
         *       entao no inverso novo[atual]=antigo[proximo].                 */
        for (k = 0; k < NUM_FACELETS; k++)
            gMovePerm[prime][ gMovePerm[cw][k] ] = k;
    }
}

/* --------------------------------------------------------------------------
 *  applyMove: out[i] = in[ perm[i] ]
 *  Usa um buffer temporario para permitir in == out.
 * -------------------------------------------------------------------------- */
void applyMove(Cube *out, const Cube *in, int moveIndex)
{
    int i;
    char tmp[NUM_FACELETS];
    const int *perm = gMovePerm[moveIndex];

    for (i = 0; i < NUM_FACELETS; i++)
        tmp[i] = in->f[ perm[i] ];

    for (i = 0; i < NUM_FACELETS; i++)
        out->f[i] = tmp[i];

    out->f[NUM_FACELETS] = '\0';
}

/* --------------------------------------------------------------------------
 *  copyCube
 * -------------------------------------------------------------------------- */
void copyCube(Cube *dst, const Cube *src)
{
    memcpy(dst->f, src->f, NUM_FACELETS + 1);
}

/* --------------------------------------------------------------------------
 *  isValidColors: 54 caracteres, 9 de cada cor e 6 centros diferentes.
 * -------------------------------------------------------------------------- */
int isValidColors(const char *str)
{
    int counts[256] = {0};
    int i, j, len;
    const char *cores = "YWGBRO";

    len = (int) strlen(str);
    if (len != NUM_FACELETS)
        return 0;

    for (i = 0; i < len; i++) {
        unsigned char ch = (unsigned char) str[i];
        if (!strchr(cores, ch))   /* caractere fora do conjunto de cores      */
            return 0;
        counts[ch]++;
    }

    /* Cada uma das 6 cores deve aparecer exatamente 9 vezes.                  */
    for (i = 0; i < 6; i++)
        if (counts[(unsigned char) cores[i]] != 9)
            return 0;

    /* Os 6 centros (indice 4 de cada face) devem ser todos diferentes: cada
     * face tem uma cor propria. Isso tambem rejeita cubos absurdos.           */
    for (i = 0; i < 6; i++)
        for (j = i + 1; j < 6; j++)
            if (str[i * 9 + 4] == str[j * 9 + 4])
                return 0;

    return 1;
}

/* --------------------------------------------------------------------------
 *  initCube: valida a string e copia para a estrutura.
 * -------------------------------------------------------------------------- */
int initCube(Cube *cube, const char *str)
{
    if (!isValidColors(str))
        return 0;
    memcpy(cube->f, str, NUM_FACELETS);
    cube->f[NUM_FACELETS] = '\0';
    return 1;
}

/* --------------------------------------------------------------------------
 *  isSolved: todas as 6 faces uniformes (iguais ao seu centro).
 * -------------------------------------------------------------------------- */
int isSolved(const Cube *cube)
{
    int face, i;
    for (face = 0; face < 6; face++) {
        int base   = face * 9;
        char centro = cube->f[base + 4];
        for (i = 0; i < 9; i++)
            if (cube->f[base + i] != centro)
                return 0;
    }
    return 1;
}

/* --------------------------------------------------------------------------
 *  printCube: planificacao do cubo no terminal.
 * -------------------------------------------------------------------------- */
static void printFaceRow(const Cube *c, int off, int row)
{
    printf("%c %c %c ", c->f[off+row*3], c->f[off+row*3+1], c->f[off+row*3+2]);
}

void printCube(const Cube *cube)
{
    int r;
    /* Cima */
    for (r = 0; r < 3; r++) {
        printf("        ");
        printFaceRow(cube, U_OFF, r);
        printf("\n");
    }
    /* Esquerda Frente Direita Tras (lado a lado) */
    for (r = 0; r < 3; r++) {
        printFaceRow(cube, L_OFF, r);
        printFaceRow(cube, F_OFF, r);
        printFaceRow(cube, R_OFF, r);
        printFaceRow(cube, B_OFF, r);
        printf("\n");
    }
    /* Baixo */
    for (r = 0; r < 3; r++) {
        printf("        ");
        printFaceRow(cube, D_OFF, r);
        printf("\n");
    }
}
