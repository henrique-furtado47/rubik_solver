/* ==========================================================================
 *  graphviz.c  -  Implementacao da exportacao para DOT/Graphviz
 * ==========================================================================
 *
 *  Geramos um no DOT por no do caminho-solucao. Em vez de usar o nome do
 *  movimento como identificador (que poderia se repetir, ex.: dois "U"),
 *  usamos identificadores unicos n0, n1, n2... e colocamos o nome do
 *  movimento no rotulo (label). Isso mantem o arquivo .dot sempre valido.
 *
 *  Exemplo de saida (para a solucao R U F' U'):
 *
 *      digraph Cubo {
 *          rankdir=TB;
 *          n0 [label="Raiz"];
 *          n1 [label="R"];
 *          n0 -> n1;
 *          n2 [label="U"];
 *          n1 -> n2;
 *          ...
 *          nSol [label="SOLUCAO", shape=doublecircle, color=green];
 *          nN -> nSol;
 *      }
 * ========================================================================== */

#include <stdio.h>
#include "graphviz.h"

void exportGraphviz(Node *solution, const char *filename)
{
    Node *path[256];
    int   len, i;
    FILE *fp;

    if (solution == NULL)
        return;

    fp = fopen(filename, "w");
    if (!fp) {
        printf("Aviso: nao foi possivel criar o arquivo %s\n", filename);
        return;
    }

    len = reconstructPath(solution, path, 256);

    fprintf(fp, "digraph Cubo {\n");
    fprintf(fp, "    rankdir=TB;\n");
    fprintf(fp, "    node [shape=circle, style=filled, fillcolor=lightyellow];\n");

    /* declara os nos: n0 = raiz, n1.. = cada movimento                        */
    fprintf(fp, "    n0 [label=\"Raiz\"];\n");
    for (i = 1; i < len; i++)
        fprintf(fp, "    n%d [label=\"%s\"];\n", i, MOVE_NAMES[path[i]->move]);

    /* no final de solucao                                                     */
    fprintf(fp, "    nSol [label=\"SOLUCAO\", shape=doublecircle, "
                "fillcolor=palegreen];\n");

    /* arestas (pai -> filho) ao longo do caminho                              */
    for (i = 1; i < len; i++)
        fprintf(fp, "    n%d -> n%d;\n", i - 1, i);

    /* liga o ultimo no ao no de solucao                                       */
    fprintf(fp, "    n%d -> nSol;\n", len - 1);

    fprintf(fp, "}\n");
    fclose(fp);

    printf("\nArquivo '%s' gerado (visualize com: dot -Tpng %s -o saida.png).\n",
           filename, filename);
}
