/* ==========================================================================
 *  solver.h  -  Arvore de estados e algoritmos de busca (BFS e A*)
 * ==========================================================================
 *
 *  Aqui esta o coracao do trabalho: a ARVORE de estados do cubo.
 *
 *  --- Estrutura Node (no da arvore) ---------------------------------------
 *  Cada no representa UM estado do cubo alcancado por uma sequencia de
 *  movimentos a partir da raiz. Ele guarda:
 *      - state    : o estado completo do cubo;
 *      - move     : o movimento que gerou este no (vindo do pai);
 *      - depth    : profundidade na arvore (numero de movimentos desde a raiz);
 *      - parent   : ponteiro para o no pai (permite reconstruir o caminho);
 *      - children : lista (vetor dinamico) de ponteiros para os filhos;
 *      - g, h, f  : custos usados pela busca A* (g=profundidade, h=heuristica).
 *
 *  --- Estrutura Tree -------------------------------------------------------
 *  A arvore guarda a raiz e uma lista de TODOS os nos alocados, para que o
 *  programa possa liberar a memoria (free) ao final de uma so vez.
 * ========================================================================== */

#ifndef SOLVER_H
#define SOLVER_H

#include "cube.h"

/* No da arvore de busca. */
typedef struct Node {
    Cube          state;        /* estado completo do cubo neste no            */
    int           move;         /* movimento (0..11) que gerou o no; -1 = raiz */
    int           depth;        /* profundidade (g) = movimentos desde a raiz  */
    struct Node  *parent;       /* ponteiro para o pai                         */
    struct Node **children;     /* vetor dinamico de filhos                    */
    int           numChildren;  /* quantidade de filhos ja criados             */
    int           g, h, f;      /* custos A*: f = g + h                        */
} Node;

/* A arvore: raiz + repositorio de todos os nos (para liberar no final). */
typedef struct {
    Node  *root;
    Node **allNodes;    /* vetor com todos os nos alocados                     */
    int    count;       /* quantos nos foram alocados                          */
    int    capacity;    /* capacidade do vetor allNodes                        */
} Tree;

/* Cria uma arvore com no raiz contendo o estado inicial 'start'. */
Tree *createTree(const Cube *start);

/* Libera toda a memoria da arvore (todos os nos + estruturas internas). */
void freeTree(Tree *tree);

/* generateChildren: expande 'node', criando um filho para cada movimento
 * valido (que nao gere um estado ja visitado nem desfaca o ultimo movimento).
 * Os filhos sao registrados na arvore. Retorna o numero de filhos criados.    */
int generateChildren(Tree *tree, Node *node);

/* bfsSolve: busca em LARGURA. Garante o caminho de MENOR numero de movimentos.
 * Retorna o no-solucao, ou NULL se nao encontrar dentro dos limites.          */
Node *bfsSolve(Tree *tree, const Cube *start);

/* astarSolve: busca A* (best-first) usando heuristica admissivel.
 * Tambem retorna o caminho otimo; costuma visitar menos nos que o BFS.        */
Node *astarSolve(Tree *tree, const Cube *start);

/* reconstructPath: a partir do no-solucao, percorre os ponteiros 'parent'
 * ate a raiz e preenche 'path' (do raiz ao solucao). Retorna o tamanho.       */
int reconstructPath(Node *solution, Node **path, int maxLen);

/* printSolution: imprime quantidade de movimentos, a sequencia e a
 * representacao da arvore (ramo) da solucao em ASCII.                         */
void printSolution(Node *solution);

#endif /* SOLVER_H */
