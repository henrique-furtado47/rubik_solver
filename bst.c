/* ==========================================================================
 *  bst.c  -  Implementacao da Arvore Binaria de Busca de estados
 * ==========================================================================
 *
 *  Insercao e busca classicas de BST, comparando os estados do cubo pela
 *  string de 54 cores (strcmp). Sem balanceamento: como os estados entram numa
 *  ordem "embaralhada" (na ordem em que a busca os gera, nao alfabetica), a
 *  arvore fica razoavelmente equilibrada na pratica, e cada operacao custa, em
 *  media, O(log n).
 * ========================================================================== */

#include <stdlib.h>
#include <string.h>
#include "bst.h"

int bstInserir(NoBST **raiz, const Cube *estado,
               const unsigned char *caminho, int len)
{
    NoBST *novo;

    /* Desce ate achar o lugar (ou um estado igual). '*raiz' aponta para o
     * ponteiro que devera receber o no novo (ponteiro-para-ponteiro).         */
    while (*raiz != NULL) {
        int cmp = strcmp(estado->f, (*raiz)->estado.f);
        if (cmp == 0)
            return 0;                       /* ja existe (igual ou mais curto)  */
        raiz = (cmp < 0) ? &(*raiz)->esq : &(*raiz)->dir;
    }

    novo = (NoBST *) malloc(sizeof(NoBST));
    if (!novo)
        return 0;                           /* sem memoria: ignora o estado     */

    copyCube(&novo->estado, estado);
    novo->len = len;
    if (len > 0)
        memcpy(novo->caminho, caminho, (size_t) len);
    novo->esq = novo->dir = NULL;

    *raiz = novo;
    return 1;
}

const NoBST *bstBuscar(const NoBST *raiz, const Cube *estado)
{
    while (raiz != NULL) {
        int cmp = strcmp(estado->f, raiz->estado.f);
        if (cmp == 0)
            return raiz;                    /* achou                            */
        raiz = (cmp < 0) ? raiz->esq : raiz->dir;
    }
    return NULL;                            /* nao esta na arvore               */
}

void bstLiberar(NoBST *raiz)
{
    if (raiz == NULL)
        return;
    bstLiberar(raiz->esq);
    bstLiberar(raiz->dir);
    free(raiz);
}
