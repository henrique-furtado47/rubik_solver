/* ==========================================================================
 *  solver.h  -  Arvore de estados e busca em profundidade (DFS)
 * ==========================================================================
 *
 *  O coracao do trabalho e a ARVORE de estados do cubo.
 *
 *  --- A ideia da arvore ----------------------------------------------------
 *  A RAIZ da arvore e o estado embaralhado que recebemos. A partir de um
 *  estado podemos aplicar 12 movimentos (U U' D D' L L' R R' F F' B B'), e
 *  cada um leva a um novo estado: sao os 12 FILHOS daquele no. Repetindo isso
 *  formamos uma arvore que cresce a cada nivel. Resolver o cubo e encontrar,
 *  nessa arvore, um no cujo estado esteja resolvido (todas as faces de uma
 *  unica cor). A sequencia de movimentos da raiz ate esse no e a solucao.
 *
 *  --- Como percorremos a arvore --------------------------------------------
 *  Usamos BUSCA EM PROFUNDIDADE (DFS) recursiva: a propria recursao desce por
 *  um galho da arvore ate um limite de profundidade; se nao achou a solucao,
 *  "volta" (backtracking) e tenta o proximo galho.
 *
 *  Para garantir a MENOR solucao possivel usamos APROFUNDAMENTO ITERATIVO:
 *  procuramos primeiro solucoes de 0 movimentos, depois 1, depois 2, ... A
 *  primeira que aparecer e necessariamente a mais curta.
 *
 *  --- Estrutura No (no da arvore) ------------------------------------------
 *  Cada no guarda:
 *      - estado       : o estado completo do cubo neste no;
 *      - movimento    : o movimento (0..11) que gerou este no (-1 = raiz);
 *      - profundidade : numero de movimentos desde a raiz;
 *      - pai          : ponteiro para o no pai (permite remontar o caminho).
 *
 *  Mantemos vivos na memoria apenas os nos do galho que estamos explorando
 *  no momento (a memoria usada e proporcional a profundidade, nao ao tamanho
 *  total da arvore): ao desistir de um galho, liberamos seus nos.
 * ========================================================================== */

#ifndef SOLVER_H
#define SOLVER_H

#include "cube.h"

/* Profundidade maxima padrao da busca (em numero de movimentos).
 * A arvore cresce ~12x a cada nivel, entao limites altos ficam lentos.
 * Cubos embaralhados com ate ~7 giros sao resolvidos em segundos.            */
#define PROF_MAXIMA 7

/* No da arvore de busca. */
typedef struct No {
    Cube        estado;        /* estado completo do cubo neste no            */
    int         movimento;     /* movimento (0..11) que gerou o no; -1 = raiz */
    int         profundidade;  /* movimentos desde a raiz                     */
    struct No  *pai;           /* ponteiro para o no pai                      */
} No;

/* resolver: monta a arvore (raiz = 'inicial') e faz a busca em profundidade
 * com aprofundamento iterativo, ate 'profMax' movimentos.
 *
 * Retorna o no-solucao (de onde se remonta o caminho pelos ponteiros 'pai'),
 * ou NULL se nao houver solucao dentro de 'profMax' movimentos.
 *
 * 'nosVisitados' recebe quantos nos da arvore foram criados durante a busca
 * (da uma nocao concreta de como a arvore cresce).                           */
No *resolver(const Cube *inicial, int profMax, long *nosVisitados);

/* remontarCaminho: a partir do no-solucao, sobe pelos ponteiros 'pai' ate a
 * raiz e preenche 'caminho' com os movimentos, na ordem da raiz ate a
 * solucao. Retorna a quantidade de movimentos.                               */
int remontarCaminho(No *solucao, int *caminho, int maxLen);

/* liberarCaminho: libera os nos do galho-solucao (da solucao ate a raiz).    */
void liberarCaminho(No *solucao);

/* imprimirSolucao: imprime a quantidade de movimentos, a sequencia e um
 * desenho em ASCII do galho da arvore que leva a solucao.                    */
void imprimirSolucao(No *solucao);

/* --------------------------------------------------------------------------
 *  Busca gulosa por fases (fallback quando o exato nao resolve em profMax)
 *  --------------------------------------------------------------------------
 *  Quando o cubo nao cabe no limite da busca exata, repetimos fases: em cada
 *  fase montamos uma arvore de profundidade 'profFase' e damos os movimentos
 *  que deixam o cubo MAIS PERTO de resolvido (maior numero de adesivos certos),
 *  tratando o estado resultante como uma nova raiz. NAO garante solucao minima
 *  e pode parar num "otimo local" sem resolver -- nesse caso reporta o melhor
 *  estado alcancado.
 * -------------------------------------------------------------------------- */
#define MAX_MOV_GULOSO 256

typedef struct {
    int  caminho[MAX_MOV_GULOSO];  /* sequencia total de movimentos            */
    int  qtd;                      /* quantos movimentos foram dados           */
    int  resolvido;                /* 1 se chegou a 100%, 0 se empacou         */
    int  pct;                      /* % de adesivos certos no estado final     */
    Cube estadoFinal;              /* cubo apos aplicar a sequencia            */
} SolucaoGulosa;

/* resolverGuloso: tenta resolver por fases (cada fase = uma arvore de
 * profundidade 'profFase'), ate 'maxFases' fases. Retorna 1 se resolveu e
 * preenche 'out'. 'nosVisitados' recebe o total de nos gerados.               */
int resolverGuloso(const Cube *inicial, int profFase, int maxFases,
                   SolucaoGulosa *out, long *nosVisitados);

/* imprimirSolucaoGulosa: imprime o resultado da busca por fases.             */
void imprimirSolucaoGulosa(const SolucaoGulosa *s);

#endif /* SOLVER_H */
