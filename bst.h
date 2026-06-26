/* ==========================================================================
 *  bst.h  -  Arvore Binaria de Busca (BST) de estados do cubo
 * ==========================================================================
 *
 *  Esta e a estrutura-chave do "Codigo 2" (busca bidirecional).
 *
 *  Na busca de encontro-no-meio precisamos, MUITAS vezes, responder rapido a
 *  pergunta: "este estado do cubo ja foi alcancado a partir do cubo resolvido?".
 *  Guardar os estados numa lista e procurar um por um seria O(n) por consulta
 *  (lento demais). Uma BUSCA BINARIA em arvore deixa cada consulta em O(log n).
 *
 *  A CHAVE de comparacao e a propria string de 54 cores do cubo (estado.f).
 *  Comparamos com strcmp: menor vai para a esquerda, maior para a direita.
 *  Para cada estado guardamos tambem o CAMINHO (sequencia de movimentos) que o
 *  gera a partir do cubo resolvido, para depois remontar a solucao.
 * ========================================================================== */

#ifndef BST_H
#define BST_H

#include "cube.h"

/* Comprimento maximo de caminho guardado num no (= profundidade da arvore de
 * tras). Movimentos cabem em 1 byte cada (0..11).                             */
#define BST_MAX_CAMINHO 12

/* No da BST: chave = estado.f; carga = caminho do alvo ate o estado. */
typedef struct NoBST {
    Cube           estado;                    /* chave (54 cores + '\0')        */
    unsigned char  caminho[BST_MAX_CAMINHO];  /* movimentos: alvo -> estado     */
    int            len;                       /* tamanho do caminho (profund.)  */
    struct NoBST  *esq, *dir;                 /* filhos da BST                  */
} NoBST;

/* bstInserir: insere 'estado' (com seu 'caminho' de tamanho 'len') na BST.
 * Como inserimos em profundidade crescente, se o estado ja existe o que esta
 * la e igual ou mais curto; nesse caso nada muda. Retorna 1 se inseriu um no
 * NOVO, 0 se o estado ja existia.                                             */
int bstInserir(NoBST **raiz, const Cube *estado,
               const unsigned char *caminho, int len);

/* bstBuscar: procura 'estado' pela chave (strcmp). Devolve o no (de onde se
 * leem caminho/len) ou NULL se nao estiver na arvore.                         */
const NoBST *bstBuscar(const NoBST *raiz, const Cube *estado);

/* bstLiberar: libera recursivamente toda a arvore.                           */
void bstLiberar(NoBST *raiz);

#endif /* BST_H */
