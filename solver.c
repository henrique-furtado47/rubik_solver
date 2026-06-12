/* ==========================================================================
 *  solver.c  -  Construcao da arvore e algoritmos de busca
 * ==========================================================================
 *
 *  CONTEUDO
 *    1. Tabela hash de estados visitados (evita repetir estados).
 *    2. Alocacao de nos e construcao da arvore.
 *    3. generateChildren  -> expansao de um no (gera os filhos).
 *    4. bfsSolve          -> Busca em Largura (BFS).
 *    5. astarSolve        -> Busca A* (best-first com heuristica).
 *    6. reconstructPath   -> caminho da raiz ate a solucao.
 *    7. printSolution     -> saida formatada + arvore em ASCII.
 * ========================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "solver.h"

/* Limites de seguranca para a busca (trabalho academico: scrambles rasos).   */
#define MAX_DEPTH      9          /* profundidade maxima explorada             */
#define MAX_NODES      4000000    /* limite de nos alocados                    */
#define HASH_SIZE      (1 << 21)  /* 2.097.152 baldes na tabela hash           */

/* ==========================================================================
 *  1. TABELA HASH DE ESTADOS VISITADOS
 *  --------------------------------------------------------------------------
 *  Por que precisamos disso? O mesmo estado do cubo pode ser alcancado por
 *  caminhos diferentes. Sem controle, a "arvore" exploraria o mesmo estado
 *  varias vezes (explosao combinatoria). A tabela hash registra cada estado
 *  ja gerado; um novo filho so e criado se seu estado for inedito.
 *
 *  Implementacao: hash com encadeamento separado (listas ligadas por balde).
 *  A chave e a propria string de 54 cores; usamos a funcao djb2.
 * ========================================================================== */

typedef struct HashEntry {
    char state[NUM_FACELETS + 1];
    struct HashEntry *next;
} HashEntry;

typedef struct {
    HashEntry **buckets;
} VisitedSet;

/* Funcao de hash djb2 sobre a string do estado. */
static unsigned long hashState(const char *s)
{
    unsigned long h = 5381;
    int c;
    while ((c = (unsigned char) *s++))
        h = ((h << 5) + h) + c;   /* h * 33 + c */
    return h;
}

static VisitedSet *visitedCreate(void)
{
    VisitedSet *v = (VisitedSet *) malloc(sizeof(VisitedSet));
    v->buckets = (HashEntry **) calloc(HASH_SIZE, sizeof(HashEntry *));
    return v;
}

/* Retorna 1 se 's' ja estava no conjunto. Se nao estava, insere e retorna 0. */
static int visitedCheckAndAdd(VisitedSet *v, const char *s)
{
    unsigned long idx = hashState(s) & (HASH_SIZE - 1);
    HashEntry *e = v->buckets[idx];
    while (e) {
        if (strcmp(e->state, s) == 0)
            return 1;             /* ja visitado                              */
        e = e->next;
    }
    /* inedito: insere no inicio do balde                                      */
    e = (HashEntry *) malloc(sizeof(HashEntry));
    memcpy(e->state, s, NUM_FACELETS + 1);
    e->next = v->buckets[idx];
    v->buckets[idx] = e;
    return 0;
}

static void visitedFree(VisitedSet *v)
{
    int i;
    for (i = 0; i < HASH_SIZE; i++) {
        HashEntry *e = v->buckets[i];
        while (e) {
            HashEntry *n = e->next;
            free(e);
            e = n;
        }
    }
    free(v->buckets);
    free(v);
}

/* ==========================================================================
 *  2. ALOCACAO DE NOS E ARVORE
 * ========================================================================== */

static Node *newNode(Tree *tree, const Cube *state, int move,
                     int depth, Node *parent)
{
    Node *n = (Node *) malloc(sizeof(Node));
    copyCube(&n->state, state);
    n->move        = move;
    n->depth       = depth;
    n->parent      = parent;
    n->children    = NULL;
    n->numChildren = 0;
    n->g           = depth;
    n->h           = countMismatches(state);  /* nro de adesivos fora do lugar */
    n->f           = n->g + n->h;

    /* registra o no no repositorio da arvore (para liberar depois)            */
    if (tree->count == tree->capacity) {
        tree->capacity = tree->capacity ? tree->capacity * 2 : 1024;
        tree->allNodes = (Node **) realloc(tree->allNodes,
                                           tree->capacity * sizeof(Node *));
    }
    tree->allNodes[tree->count++] = n;
    return n;
}

Tree *createTree(const Cube *start)
{
    Tree *t = (Tree *) malloc(sizeof(Tree));
    t->allNodes = NULL;
    t->count    = 0;
    t->capacity = 0;
    t->root     = newNode(t, start, -1, 0, NULL);
    return t;
}

void freeTree(Tree *tree)
{
    int i;
    for (i = 0; i < tree->count; i++) {
        free(tree->allNodes[i]->children);
        free(tree->allNodes[i]);
    }
    free(tree->allNodes);
    free(tree);
}

/* Anexa 'child' ao vetor de filhos de 'parent'. */
static void addChild(Node *parent, Node *child)
{
    parent->children = (Node **) realloc(parent->children,
                                 (parent->numChildren + 1) * sizeof(Node *));
    parent->children[parent->numChildren++] = child;
}

/* Inverso de um movimento (U<->U', etc.) para evitar desfazer o ultimo giro. */
static int inverseMove(int m)
{
    return (m % 2 == 0) ? m + 1 : m - 1;
}

/* ==========================================================================
 *  3. generateChildren
 *  --------------------------------------------------------------------------
 *  Expande um no: para cada um dos 12 movimentos, calcula o estado resultante.
 *  Um filho so e criado se:
 *     - o movimento NAO for o inverso do que gerou o no (poda: evita desfazer
 *       imediatamente o ultimo giro, o que nunca encurta a solucao);
 *     - o estado resultante ainda nao tiver sido visitado.
 *  O VisitedSet e passado por parametro (ver bfsSolve/astarSolve).
 * ========================================================================== */
static int generateChildrenInternal(Tree *tree, Node *node, VisitedSet *visited)
{
    int m, created = 0;
    for (m = 0; m < NUM_MOVES; m++) {
        Cube child;

        /* Poda 1: nao desfazer o movimento anterior.                          */
        if (node->move != -1 && m == inverseMove(node->move))
            continue;

        applyMove(&child, &node->state, m);

        /* Poda 2: estado repetido (controle de visitados via hash).           */
        if (visitedCheckAndAdd(visited, child.f))
            continue;

        /* Estado inedito -> vira um novo no filho na arvore.                  */
        {
            Node *c = newNode(tree, &child, m, node->depth + 1, node);
            addChild(node, c);
            created++;
        }
    }
    return created;
}

/* Versao publica de generateChildren (cria seu proprio conjunto de visitados
 * apenas com a raiz; util para demonstracao isolada da expansao de um no).    */
int generateChildren(Tree *tree, Node *node)
{
    int n;
    VisitedSet *v = visitedCreate();
    visitedCheckAndAdd(v, node->state.f);
    n = generateChildrenInternal(tree, node, v);
    visitedFree(v);
    return n;
}

/* ==========================================================================
 *  4. bfsSolve  -  BUSCA EM LARGURA
 *  --------------------------------------------------------------------------
 *  Explora a arvore nivel a nivel: primeiro todos os estados a 1 movimento,
 *  depois a 2 movimentos, etc. Como avanca por profundidade crescente, o
 *  primeiro estado resolvido encontrado esta no MENOR numero de movimentos
 *  possivel (solucao otima). Usa uma FILA (FIFO) de nos.
 * ========================================================================== */
Node *bfsSolve(Tree *tree, const Cube *start)
{
    VisitedSet *visited;
    Node      **queue;
    long        head = 0, tail = 0;
    long        capacity = 1 << 16;
    Node       *result = NULL;

    (void) start;  /* a raiz da arvore ja contem o estado inicial             */

    /* Caso trivial: ja resolvido.                                            */
    if (isSolved(&tree->root->state))
        return tree->root;

    visited = visitedCreate();
    visitedCheckAndAdd(visited, tree->root->state.f);

    queue = (Node **) malloc(capacity * sizeof(Node *));
    queue[tail++] = tree->root;

    while (head < tail) {
        Node *cur = queue[head++];

        if (isSolved(&cur->state)) { result = cur; break; }
        if (cur->depth >= MAX_DEPTH) continue;
        if (tree->count >= MAX_NODES) break;

        /* expande o no atual (gera filhos ineditos)                          */
        generateChildrenInternal(tree, cur, visited);

        /* enfileira os filhos recem-criados                                  */
        {
            int i;
            for (i = 0; i < cur->numChildren; i++) {
                if (tail == capacity) {
                    capacity *= 2;
                    queue = (Node **) realloc(queue, capacity * sizeof(Node *));
                }
                queue[tail++] = cur->children[i];
            }
        }
    }

    free(queue);
    visitedFree(visited);
    return result;
}

/* ==========================================================================
 *  5. astarSolve  -  BUSCA A*
 *  --------------------------------------------------------------------------
 *  A* escolhe sempre expandir o no com menor f = g + h, onde:
 *     g = profundidade (numero de movimentos ja feitos);
 *     h = heuristica: estimativa (admissivel) de movimentos que faltam.
 *
 *  Heuristica usada: cada giro de 90 graus reposiciona no maximo 20 adesivos,
 *  entao h = teto( adesivos_fora_do_lugar / 20 ). Como h nunca superestima o
 *  custo real, A* continua encontrando a solucao OTIMA, geralmente visitando
 *  menos nos que o BFS.
 *
 *  A fila de prioridade e um HEAP BINARIO de minimo, indexado por f.
 * ========================================================================== */

/* Heuristica admissivel (ver explicacao acima). */
static int heuristic(const Cube *c)
{
    int mism = countMismatches(c);
    return (mism + 19) / 20;   /* teto da divisao por 20                       */
}

/* ---- Heap binario de minimo de ponteiros Node*, ordenado por node->f ---- */
typedef struct {
    Node **data;
    long   size, capacity;
} MinHeap;

static MinHeap *heapCreate(void)
{
    MinHeap *h = (MinHeap *) malloc(sizeof(MinHeap));
    h->capacity = 1 << 16;
    h->size     = 0;
    h->data     = (Node **) malloc(h->capacity * sizeof(Node *));
    return h;
}
static void heapPush(MinHeap *h, Node *n)
{
    long i;
    if (h->size == h->capacity) {
        h->capacity *= 2;
        h->data = (Node **) realloc(h->data, h->capacity * sizeof(Node *));
    }
    i = h->size++;
    h->data[i] = n;
    while (i > 0) {                       /* sobe enquanto menor que o pai     */
        long p = (i - 1) / 2;
        if (h->data[p]->f <= h->data[i]->f) break;
        { Node *t = h->data[p]; h->data[p] = h->data[i]; h->data[i] = t; }
        i = p;
    }
}
static Node *heapPop(MinHeap *h)
{
    Node *top = h->data[0];
    long i = 0;
    h->data[0] = h->data[--h->size];
    for (;;) {                            /* desce restaurando a propriedade   */
        long l = 2*i + 1, r = 2*i + 2, s = i;
        if (l < h->size && h->data[l]->f < h->data[s]->f) s = l;
        if (r < h->size && h->data[r]->f < h->data[s]->f) s = r;
        if (s == i) break;
        { Node *t = h->data[s]; h->data[s] = h->data[i]; h->data[i] = t; }
        i = s;
    }
    return top;
}

Node *astarSolve(Tree *tree, const Cube *start)
{
    VisitedSet *visited;
    MinHeap    *open;
    Node       *result = NULL;

    (void) start;

    visited = visitedCreate();
    visitedCheckAndAdd(visited, tree->root->state.f);

    /* recalcula h da raiz com a heuristica de A* (a raiz foi criada com o
     * numero bruto de mismatches; aqui usamos a versao admissivel)            */
    tree->root->h = heuristic(&tree->root->state);
    tree->root->f = tree->root->g + tree->root->h;

    open = heapCreate();
    heapPush(open, tree->root);

    while (open->size > 0) {
        Node *cur = heapPop(open);
        int   m;

        if (isSolved(&cur->state)) { result = cur; break; }
        if (cur->depth >= MAX_DEPTH) continue;
        if (tree->count >= MAX_NODES) break;

        for (m = 0; m < NUM_MOVES; m++) {
            Cube child;
            if (cur->move != -1 && m == inverseMove(cur->move)) continue;

            applyMove(&child, &cur->state, m);
            if (visitedCheckAndAdd(visited, child.f)) continue;

            {
                Node *c = newNode(tree, &child, m, cur->depth + 1, cur);
                c->h = heuristic(&child);     /* heuristica admissivel         */
                c->f = c->g + c->h;
                addChild(cur, c);
                heapPush(open, c);
            }
        }
    }

    free(open->data);
    free(open);
    visitedFree(visited);
    return result;
}

/* ==========================================================================
 *  6. reconstructPath
 *  --------------------------------------------------------------------------
 *  A solucao e um no folha. Subimos pelos ponteiros 'parent' ate a raiz,
 *  guardando os nos, e depois invertemos a ordem (raiz -> solucao).
 * ========================================================================== */
int reconstructPath(Node *solution, Node **path, int maxLen)
{
    int len = 0, i, j;
    Node *n = solution;

    while (n != NULL && len < maxLen) {
        path[len++] = n;
        n = n->parent;
    }
    /* inverte para ficar da raiz para a solucao                               */
    for (i = 0, j = len - 1; i < j; i++, j--) {
        Node *t = path[i]; path[i] = path[j]; path[j] = t;
    }
    return len;
}

/* ==========================================================================
 *  7. printSolution
 * ========================================================================== */
void printSolution(Node *solution)
{
    Node *path[256];
    int   len, i;

    if (solution == NULL) {
        printf("Nenhuma solucao encontrada dentro dos limites de busca.\n");
        return;
    }

    len = reconstructPath(solution, path, 256);

    /* --- quantidade de movimentos e sequencia ---------------------------- */
    printf("Quantidade de movimentos: %d\n\n", len - 1);   /* raiz nao conta   */
    printf("Movimentos:\n");
    for (i = 1; i < len; i++)                               /* pula a raiz      */
        printf("%s%s", MOVE_NAMES[path[i]->move], (i < len - 1) ? " " : "\n");
    if (len == 1) printf("(cubo ja estava resolvido)\n");

    /* --- representacao da arvore (ramo) da solucao em ASCII -------------- */
    printf("\nArvore da solucao:\n");
    printf("Raiz\n");
    for (i = 1; i < len; i++) {
        int k;
        for (k = 0; k < i; k++) printf("    ");   /* indentacao por nivel      */
        printf("└── %s\n", MOVE_NAMES[path[i]->move]);
    }
    /* marca o estado final como SOLUCAO */
    {
        int k;
        for (k = 0; k < len; k++) printf("    ");
        printf("└── SOLUCAO\n");
    }
}
