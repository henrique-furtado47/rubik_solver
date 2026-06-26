/* ==========================================================================
 *  solver_bid.h  -  Busca BIDIRECIONAL (encontro no meio) com BST
 * ==========================================================================
 *
 *  --- A ideia ("Codigo 2") -------------------------------------------------
 *  Uma arvore unica que desce ate, digamos, 12 movimentos teria ~9,5^12 ~ 500
 *  bilhoes de nos: inviavel. A busca BIDIRECIONAL quebra esse problema em duas
 *  metades muito menores:
 *
 *     - Arvore de TRAS: a partir do cubo RESOLVIDO, geramos todos os estados a
 *       ate 'profTras' movimentos e os guardamos numa BST (chave = as 54 cores).
 *     - Arvore da FRENTE: a partir do cubo EMBARALHADO, exploramos ate
 *       'profFrente' movimentos. Para cada estado alcancado, perguntamos a BST:
 *       "voce ja chegou aqui pelo outro lado?".
 *
 *  Quando um estado aparece nas duas arvores, "costuramos": o caminho da frente
 *  (embaralhado -> encontro) + o inverso do caminho de tras (encontro ->
 *  resolvido) formam a solucao. Ela tem no maximo profFrente+profTras
 *  movimentos e e MINIMA (achamos o menor total possivel).
 *
 *  Custo: ~9,5^6 + ~9,5^6 ~ 1,5 milhao de nos para 12 movimentos -- milhares de
 *  vezes menos que a arvore unica.
 *
 *  Estruturas de arvore usadas: a arvore de estados (n-aria, percorrida em
 *  profundidade, dos dois lados) E a Arvore Binaria de Busca (bst.h) que casa
 *  os estados. Tudo arvore -- nenhum grafo, nenhuma tabela hash.
 * ========================================================================== */

#ifndef SOLVER_BID_H
#define SOLVER_BID_H

#include "cube.h"

#define BID_MAX_SOL 32   /* teto de movimentos na solucao remontada            */

typedef struct {
    int  caminho[BID_MAX_SOL]; /* sequencia de movimentos da solucao           */
    int  qtd;                  /* tamanho da solucao                           */
    int  encontrou;            /* 1 se achou solucao dentro do limite          */
    int  limite;               /* profFrente + profTras (limite de busca)      */
    long nosTras;              /* nos gerados construindo a arvore de tras     */
    long nosFrente;            /* nos gerados na busca da frente               */
    long estadosBST;           /* estados distintos guardados na BST           */
} SolucaoBid;

/* resolverBidirecional: monta a BST com os estados ate 'profTras' movimentos do
 * cubo resolvido e busca, a partir de 'inicial', um estado em comum em ate
 * 'profFrente' movimentos. Preenche 'out' e retorna 1 se resolveu.            */
int resolverBidirecional(const Cube *inicial, int profFrente, int profTras,
                         SolucaoBid *out);

/* imprimirSolucaoBid: mostra contagens, a sequencia e confere aplicando-a.    */
void imprimirSolucaoBid(const Cube *inicial, const SolucaoBid *s);

#endif /* SOLVER_BID_H */
