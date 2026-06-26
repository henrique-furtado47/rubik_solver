/* ==========================================================================
 *  solver_bid.c  -  Busca bidirecional (encontro no meio) com BST
 * ==========================================================================
 *  Veja a explicacao geral em solver_bid.h.
 * ========================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "solver_bid.h"
#include "bst.h"

/* --------------------------------------------------------------------------
 *  Poda dos galhos (identica a do Codigo 1): corta sequencias redundantes sem
 *  perder estados alcancaveis. E segura nos dois sentidos da busca.
 * -------------------------------------------------------------------------- */
static int movimentoInverso(int m) { return (m % 2 == 0) ? m + 1 : m - 1; }
static int faceDe(int m)           { return m / 2; }
static int eixoDe(int m)           { return m / 4; }

static int podarMov(int ultimo, int penultimo, int m)
{
    if (ultimo == -1) return 0;
    if (m == movimentoInverso(ultimo)) return 1;                 /* nao cancela  */
    if (eixoDe(m) == eixoDe(ultimo) && faceDe(m) != faceDe(ultimo)
        && faceDe(m) < faceDe(ultimo)) return 1;                 /* faces opostas */
    if (faceDe(m) == faceDe(ultimo)) {
        if (ultimo % 2 == 1) return 1;                           /* R' R' = R R   */
        if (penultimo != -1 && faceDe(penultimo) == faceDe(ultimo))
            return 1;                                            /* 3x mesma face */
    }
    return 0;
}

/* montarAlvo: o cubo RESOLVIDO correspondente ao 'inicial'. Como os centros
 * nunca se movem, a cor verdadeira de cada face e o seu centro; preenchemos
 * cada face inteira com a cor do proprio centro.                              */
static void montarAlvo(Cube *alvo, const Cube *inicial)
{
    static const int offs[6] = { U_OFF, L_OFF, F_OFF, R_OFF, B_OFF, D_OFF };
    int f, i;

    copyCube(alvo, inicial);            /* garante tamanho e terminador '\0'    */
    for (f = 0; f < 6; f++) {
        char centro = inicial->f[offs[f] + 4];
        for (i = 0; i < 9; i++)
            alvo->f[offs[f] + i] = centro;
    }
}

/* ==========================================================================
 *  ARVORE DE TRAS: gera os estados a ate 'profTras' do alvo e insere na BST.
 * ========================================================================== */
typedef struct {
    NoBST        *raiz;
    long          nos;                    /* nos gerados                        */
    long          distintos;              /* estados distintos inseridos        */
    unsigned char atual[BST_MAX_CAMINHO]; /* caminho do galho atual             */
} ConstrTras;

/* Gera, por DFS, exatamente os estados a profundidade 'alvoProf' e os insere.
 * Chamando para alvoProf = 0,1,...,profTras, cada estado entra na MENOR
 * profundidade em que aparece (insercoes repetidas sao ignoradas pela BST).   */
static void construirTras(const Cube *estado, int prof, int alvoProf,
                          int ultimo, int penultimo, ConstrTras *c)
{
    int m;

    if (prof == alvoProf) {
        if (bstInserir(&c->raiz, estado, c->atual, prof))
            c->distintos++;
        return;
    }
    for (m = 0; m < NUM_MOVES; m++) {
        Cube filho;
        if (podarMov(ultimo, penultimo, m)) continue;
        applyMove(&filho, estado, m);
        c->atual[prof] = (unsigned char) m;
        c->nos++;
        construirTras(&filho, prof + 1, alvoProf, m, ultimo, c);
    }
}

/* ==========================================================================
 *  ARVORE DA FRENTE: a partir do embaralhado, procura um estado na BST.
 * ========================================================================== */
typedef struct {
    const NoBST *bstTras;
    SolucaoBid  *out;
    int          melhorTotal;     /* menor (frente+tras) ja achado             */
    int          fwd[BID_MAX_SOL]; /* movimentos do galho atual da frente       */
    long         nos;
} BuscaFrente;

/* costura: combina o caminho da frente (fwd[0..a-1]) com o de tras invertido. */
static void costura(BuscaFrente *bf, int a, const NoBST *no)
{
    int total = a + no->len, k, q = 0;

    if (total >= bf->melhorTotal)
        return;                            /* nao melhora a melhor solucao      */
    bf->melhorTotal = total;

    for (k = 0; k < a; k++)                /* embaralhado -> encontro           */
        bf->out->caminho[q++] = bf->fwd[k];
    for (k = no->len - 1; k >= 0; k--)     /* encontro -> resolvido (inverso)   */
        bf->out->caminho[q++] = movimentoInverso(no->caminho[k]);

    bf->out->qtd       = q;
    bf->out->encontrou = 1;
}

/* DFS da frente a exatamente 'alvoProf' movimentos; nos desse nivel consultam
 * a BST. Chamado para alvoProf = 0,1,... (aprofundamento iterativo).          */
static void buscarFrente(const Cube *estado, int prof, int alvoProf,
                         int ultimo, int penultimo, BuscaFrente *bf)
{
    int m;

    if (prof == alvoProf) {
        const NoBST *no = bstBuscar(bf->bstTras, estado);
        if (no != NULL)
            costura(bf, prof, no);
        return;
    }
    for (m = 0; m < NUM_MOVES; m++) {
        Cube filho;
        if (podarMov(ultimo, penultimo, m)) continue;
        applyMove(&filho, estado, m);
        bf->fwd[prof] = m;
        bf->nos++;
        buscarFrente(&filho, prof + 1, alvoProf, m, ultimo, bf);
    }
}

/* ==========================================================================
 *  Orquestracao
 * ========================================================================== */
int resolverBidirecional(const Cube *inicial, int profFrente, int profTras,
                         SolucaoBid *out)
{
    Cube        alvo;
    ConstrTras  ct;
    BuscaFrente bf;
    int         a, d;

    if (profTras > BST_MAX_CAMINHO) profTras = BST_MAX_CAMINHO;
    if (profFrente > BID_MAX_SOL - profTras) profFrente = BID_MAX_SOL - profTras;

    out->qtd = 0; out->encontrou = 0;
    out->limite = profFrente + profTras;
    out->nosTras = out->nosFrente = out->estadosBST = 0;

    montarAlvo(&alvo, inicial);

    /* 1) Constroi a BST com os estados a ate profTras movimentos do resolvido. */
    ct.raiz = NULL; ct.nos = 0; ct.distintos = 0;
    for (d = 0; d <= profTras; d++)
        construirTras(&alvo, 0, d, -1, -1, &ct);
    out->nosTras    = ct.nos;
    out->estadosBST = ct.distintos;

    /* 2) Busca da frente, aprofundamento iterativo. Paramos quando a propria
     *    profundidade da frente ja nao puder melhorar a melhor solucao.        */
    bf.bstTras     = ct.raiz;
    bf.out         = out;
    bf.melhorTotal = profFrente + profTras + 1;   /* "infinito"                 */
    bf.nos         = 0;
    for (a = 0; a <= profFrente && a < bf.melhorTotal; a++)
        buscarFrente(inicial, 0, a, -1, -1, &bf);
    out->nosFrente = bf.nos;

    bstLiberar(ct.raiz);
    return out->encontrou;
}

/* ==========================================================================
 *  Saida
 * ========================================================================== */
void imprimirSolucaoBid(const Cube *inicial, const SolucaoBid *s)
{
    Cube c;
    int  i;

    printf("\n%sArvore de tras:%s %s%ld%s estados distintos na BST "
           "(%s%ld%s nos gerados)\n",
           ansi(A_BOLD), ansi(A_RESET), ansi(A_BOLD), s->estadosBST, ansi(A_RESET),
           ansi(A_DIM), s->nosTras, ansi(A_RESET));
    printf("%sArvore da frente:%s %s%ld%s nos gerados\n",
           ansi(A_BOLD), ansi(A_RESET), ansi(A_BOLD), s->nosFrente, ansi(A_RESET));

    if (!s->encontrou) {
        printf("\n%s%s== SOLUCAO NAO ENCONTRADA ==%s\n",
               ansi(A_BOLD), ansi(A_YEL), ansi(A_RESET));
        printf("Nenhuma solucao de ate %d movimentos. Aumente profFrente/profTras.\n",
               s->limite);
        return;
    }

    printf("\n%s%s== SOLUCAO ENCONTRADA (bidirecional + BST) ==%s\n",
           ansi(A_BOLD), ansi(A_GREEN), ansi(A_RESET));
    printf("Quantidade de movimentos: %s%d%s %s(minima)%s\n\n",
           ansi(A_BOLD), s->qtd, ansi(A_RESET), ansi(A_DIM), ansi(A_RESET));

    printf("Movimentos: %s%s", ansi(A_BOLD), ansi(A_CYAN));
    for (i = 0; i < s->qtd; i++)
        printf("%s%s", MOVE_NAMES[s->caminho[i]], (i < s->qtd - 1) ? " " : "");
    printf("%s\n", ansi(A_RESET));

    /* Conferencia: aplica a solucao no cubo inicial e mostra o resultado. */
    copyCube(&c, inicial);
    for (i = 0; i < s->qtd; i++)
        applyMove(&c, &c, s->caminho[i]);

    printf("\n%sCubo apos aplicar a solucao:%s\n\n", ansi(A_BOLD), ansi(A_RESET));
    printCube(&c);
    if (isSolved(&c))
        printf("\n%s>> conferido: cubo resolvido!%s\n", ansi(A_GREEN), ansi(A_RESET));
    else
        printf("\n%s>> ATENCAO: a sequencia nao resolveu (revisar).%s\n",
               ansi(A_YEL), ansi(A_RESET));
}
