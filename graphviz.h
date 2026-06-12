/* ==========================================================================
 *  graphviz.h  -  Exportacao da arvore-solucao para o formato DOT (Graphviz)
 * ==========================================================================
 *  Gera um arquivo .dot que pode ser visualizado com a ferramenta Graphviz:
 *
 *      dot -Tpng solution.dot -o solution.png
 *
 *  O grafo gerado mostra o caminho da raiz ate a solucao.
 * ========================================================================== */

#ifndef GRAPHVIZ_H
#define GRAPHVIZ_H

#include "solver.h"

/* exportGraphviz: escreve em 'filename' a representacao DOT do caminho que
 * leva da raiz ate o no 'solution'.                                          */
void exportGraphviz(Node *solution, const char *filename);

#endif /* GRAPHVIZ_H */
