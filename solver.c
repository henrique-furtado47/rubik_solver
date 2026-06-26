/* ==========================================================================
 *  solver.c  -  Busca em profundidade (DFS) na arvore de estados do cubo
 * ==========================================================================
 *
 *  Estrategia: APROFUNDAMENTO ITERATIVO.
 *
 *  Fazemos varias buscas em profundidade, cada uma com um limite maior:
 *      limite = 0, depois 1, depois 2, ... ate PROF_MAXIMA.
 *  A busca de limite L so olha solucoes de ate L movimentos. Como tentamos os
 *  limites em ordem crescente, a primeira solucao encontrada e a MAIS CURTA
 *  (o menor numero de movimentos), que e exatamente o que o enunciado pede.
 *
 *  A busca em profundidade em si e uma simples recursao: estando em um no,
 *  geramos um filho para cada movimento e chamamos a recursao nesse filho.
 *  Se o galho nao leva a solucao, liberamos seus nos e tentamos o proximo
 *  movimento (isto e o "backtracking").
 * ========================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "solver.h"

/* movimentoInverso: indice do movimento que desfaz 'm'.
 * Os movimentos vem em pares (horario, anti-horario): 0/1, 2/3, ...
 * Logo o inverso de um indice par e o seguinte, e o de um impar e o anterior. */
static int movimentoInverso(int m) { return (m % 2 == 0) ? m + 1 : m - 1; }

/* faceDe / eixoDe: a partir do indice do movimento (0..11) descobrimos a face
 * e o eixo. A ordem dos movimentos e U U' D D' L L' R R' F F' B B', logo:
 *   - face = m/2  ->  0=U 1=D 2=L 3=R 4=F 5=B
 *   - eixo = m/4  ->  0=U/D 1=L/R 2=F/B   (faces opostas tem o mesmo eixo)      */
static int faceDe(int m) { return m / 2; }
static int eixoDe(int m) { return m / 4; }

/* --------------------------------------------------------------------------
 *  podar: decide se NAO vale a pena gerar o filho do movimento 'm' a partir
 *  do no 'no'. Todas as regras abaixo sao SEGURAS: elas so cortam galhos que
 *  levam a sequencias redundantes (existe sempre outra sequencia igual ou mais
 *  curta que sobrevive). Logo a menor solucao nunca e perdida.
 *
 *  Retorna 1 para podar (pular o movimento) ou 0 para gera-lo.
 * -------------------------------------------------------------------------- */
static int podarMov(int ultimo, int penultimo, int m)
{
    if (ultimo == -1)
        return 0;                       /* nada antes: nada a podar             */

    /* (1) Nao desfazer o ultimo movimento: R seguido de R' so volta ao pai.    */
    if (m == movimentoInverso(ultimo))
        return 1;

    /* (2) Faces opostas COMUTAM (ex.: R e L nao se atrapalham), entao "R L" e
     *     "L R" levam ao mesmo estado. Para nao explorar as duas ordens,
     *     fixamos uma: depois de uma face, so permitimos a face oposta se ela
     *     vier "em ordem" (numero maior). Assim cada par comutativo aparece
     *     uma unica vez.                                                        */
    if (eixoDe(m) == eixoDe(ultimo) && faceDe(m) != faceDe(ultimo)
        && faceDe(m) < faceDe(ultimo))
        return 1;

    /* (3) Mesma face em sequencia: dois giros bastam para qualquer efeito.
     *     - 3a) Das duas formas de dar meia-volta (R R e R' R'), guardamos so
     *           uma: se o ultimo foi anti-horario, nao repetimos a face (o caso
     *           horario-horario ja cobre a meia-volta).
     *     - 3b) Nunca a mesma face 3x seguidas: R R R = R' (mais curto).        */
    if (faceDe(m) == faceDe(ultimo)) {
        if (ultimo % 2 == 1)            /* 3a: evita R' R' (duplicata de R R)    */
            return 1;
        if (penultimo != -1 && faceDe(penultimo) == faceDe(ultimo))
            return 1;                   /* 3b: terceira repeticao da mesma face  */
    }

    return 0;
}

/* podar: versao para a arvore exata (extrai ultimo/penultimo dos ponteiros). */
static int podar(const No *no, int m)
{
    int ultimo    = no->movimento;
    int penultimo = (no->pai != NULL) ? no->pai->movimento : -1;
    return podarMov(ultimo, penultimo, m);
}

/* criarNo: aloca um no da arvore com o estado e os dados informados. */
static No *criarNo(const Cube *estado, int movimento, int profundidade, No *pai)
{
    No *no = (No *) malloc(sizeof(No));
    if (!no) { fprintf(stderr, "Erro: memoria insuficiente.\n"); exit(1); }
    copyCube(&no->estado, estado);
    no->movimento    = movimento;
    no->profundidade = profundidade;
    no->pai          = pai;
    return no;
}

/* --------------------------------------------------------------------------
 *  buscaProfundidade: explora a arvore a partir de 'no', sem ultrapassar
 *  'limite' movimentos. Retorna o no-solucao se encontrar, ou NULL.
 *
 *  - Se o estado deste no ja esta resolvido, achamos a solucao.
 *  - Se atingimos o limite de profundidade, este galho acabou (volta NULL).
 *  - Caso contrario, geramos um filho para cada movimento e descemos a
 *    recursao. Galhos que falham tem seus nos liberados (backtracking).
 * -------------------------------------------------------------------------- */
static No *buscaProfundidade(No *no, int limite, long *nosVisitados)
{
    int m;

    if (isSolved(&no->estado))
        return no;                       /* chegamos a um cubo resolvido       */

    if (no->profundidade >= limite)
        return NULL;                     /* nao podemos descer mais            */

    for (m = 0; m < NUM_MOVES; m++) {
        Cube  filhoEstado;
        No   *filho, *resultado;

        /* Poda dos galhos redundantes (ver podar()): nao desfazer o ultimo
         * movimento, fixar a ordem de faces opostas e nao repetir a mesma
         * face alem do necessario. Nenhuma delas perde a solucao mais curta.   */
        if (podar(no, m))
            continue;

        applyMove(&filhoEstado, &no->estado, m);
        filho = criarNo(&filhoEstado, m, no->profundidade + 1, no);
        (*nosVisitados)++;

        resultado = buscaProfundidade(filho, limite, nosVisitados);
        if (resultado != NULL)
            return resultado;            /* solucao achada neste galho         */

        free(filho);                     /* galho sem solucao: libera o filho  */
    }
    return NULL;
}

/* --------------------------------------------------------------------------
 *  resolver: aprofundamento iterativo (limites crescentes).
 * -------------------------------------------------------------------------- */
No *resolver(const Cube *inicial, int profMax, long *nosVisitados)
{
    int limite;

    *nosVisitados = 0;

    for (limite = 0; limite <= profMax; limite++) {
        No *raiz, *solucao;

        raiz = criarNo(inicial, -1, 0, NULL);
        (*nosVisitados)++;

        printf("  procurando solucao de ate %d movimento(s)...\n", limite);
        solucao = buscaProfundidade(raiz, limite, nosVisitados);

        if (solucao != NULL)
            return solucao;              /* primeira (e mais curta) solucao    */

        free(raiz);                      /* nada neste limite: tenta o proximo */
    }
    return NULL;                         /* nao resolveu dentro de profMax      */
}

/* --------------------------------------------------------------------------
 *  remontarCaminho: sobe pelos 'pai' acumulando os movimentos e depois
 *  inverte, para devolver na ordem da raiz ate a solucao.
 * -------------------------------------------------------------------------- */
int remontarCaminho(No *solucao, int *caminho, int maxLen)
{
    int  n = 0, i;
    No  *atual = solucao;

    /* Sobe da solucao ate a raiz (a raiz tem movimento -1, que ignoramos).    */
    while (atual != NULL && atual->movimento != -1 && n < maxLen) {
        caminho[n++] = atual->movimento;
        atual = atual->pai;
    }
    /* Inverte para a ordem correta (raiz -> solucao).                         */
    for (i = 0; i < n / 2; i++) {
        int t = caminho[i];
        caminho[i] = caminho[n - 1 - i];
        caminho[n - 1 - i] = t;
    }
    return n;
}

/* liberarCaminho: libera os nos do galho da solucao (da solucao ate a raiz). */
void liberarCaminho(No *solucao)
{
    No *atual = solucao;
    while (atual != NULL) {
        No *pai = atual->pai;
        free(atual);
        atual = pai;
    }
}

/* --------------------------------------------------------------------------
 *  imprimirSolucao: mostra a quantidade de movimentos, a sequencia e um
 *  desenho do galho da arvore (um nivel por movimento).
 * -------------------------------------------------------------------------- */
void imprimirSolucao(No *solucao)
{
    int caminho[128];
    int n, i;

    n = remontarCaminho(solucao, caminho, 128);

    printf("\n%s%s== SOLUCAO ENCONTRADA ==%s\n", ansi(A_BOLD), ansi(A_GREEN), ansi(A_RESET));
    printf("Quantidade de movimentos: %s%d%s\n\n", ansi(A_BOLD), n, ansi(A_RESET));

    if (n == 0) {
        printf("O cubo ja estava resolvido!\n");
        return;
    }

    printf("Movimentos: %s%s", ansi(A_BOLD), ansi(A_CYAN));
    for (i = 0; i < n; i++)
        printf("%s%s", MOVE_NAMES[caminho[i]], (i < n - 1) ? " " : "");
    printf("%s\n", ansi(A_RESET));

    /* Desenho do galho da arvore (raiz -> ... -> solucao). */
    printf("\n%sGalho da arvore percorrido ate a solucao:%s\n", ansi(A_DIM), ansi(A_RESET));
    printf("(raiz: cubo embaralhado)\n");
    for (i = 0; i < n; i++) {
        int k;
        for (k = 0; k <= i; k++) printf("  ");
        printf("|_ %s%s%s   %s(nivel %d)%s\n",
               ansi(A_CYAN), MOVE_NAMES[caminho[i]], ansi(A_RESET),
               ansi(A_DIM), i + 1, ansi(A_RESET));
    }
    printf("  ");
    for (i = 0; i < n; i++) printf("  ");
    printf("%s>> cubo resolvido%s\n", ansi(A_GREEN), ansi(A_RESET));

    /* Confirmacao visual: o cubo apos aplicar a solucao (deve estar resolvido). */
    printf("\n%sCubo resolvido:%s\n\n", ansi(A_BOLD), ansi(A_RESET));
    printCube(&solucao->estado);
}
 * ==========================================================================
 *
 *  Ideia: se a busca exata nao acha solucao no limite, paramos de exigir a
 *  solucao MINIMA e passamos a fazer PROGRESSO por fases:
 *
 *    1. A partir do estado atual, montamos uma arvore de profundidade 'profFase'
 *       (ex.: 7) e procuramos a FOLHA mais proxima de resolvida -- isto e, a que
 *       tem o maior numero de adesivos na cor do proprio centro (heuristica).
 *    2. Aplicamos os movimentos ate essa folha. Agora o cubo esta "mais perto".
 *    3. Tratamos esse novo estado como uma raiz nova e repetimos.
 *
 *  E uma arvore comum por fase + uma funcao de avaliacao + um laco. NAO garante
 *  a solucao minima e PODE empacar num "otimo local" (um ponto em que nenhuma
 *  sequencia de 'profFase' movimentos melhora a contagem). Nesses casos o
 *  programa reporta honestamente ate onde chegou.
 * ========================================================================== */

/* adesivosCorretos: heuristica "quao perto de resolvido". Conta quantos dos 54
 * adesivos ja estao na cor do centro da propria face (54 = cubo resolvido).    */
static int adesivosCorretos(const Cube *c)
{
    static const int offs[6] = { U_OFF, L_OFF, F_OFF, R_OFF, B_OFF, D_OFF };
    int face, i, n = 0;

    for (face = 0; face < 6; face++) {
        char centro = c->f[offs[face] + 4];
        for (i = 0; i < 9; i++)
            if (c->f[offs[face] + i] == centro)
                n++;
    }
    return n;
}

/* Estado de trabalho de UMA fase da busca gulosa. */
typedef struct {
    int melhorScore;      /* maior numero de adesivos certos visto na fase     */
    int len;              /* comprimento do caminho ate esse melhor estado     */
    int caminho[16];      /* movimentos ate o melhor estado (<= profFase)      */
    int atual[16];        /* movimentos do galho que esta sendo explorado      */
} Fase;

/* buscaFase: DFS ate 'limite' movimentos guardando a folha de maior pontuacao.
 * E a mesma arvore de antes, mas em vez de parar no cubo resolvido, ela mede
 * cada no pela heuristica e memoriza o melhor. Memoria O(limite) (recursao).
 *
 * Empate: se a pontuacao apenas igual a melhor ja vista e a melhor era a raiz
 * (len 0), aceitamos um caminho de mesmo valor para tentar atravessar "platos".*/
static void buscaFase(const Cube *estado, int prof, int limite,
                      int ultimo, int penultimo, Fase *f, long *nos)
{
    int s = adesivosCorretos(estado);
    int m;

    if (s > f->melhorScore || (s == f->melhorScore && f->len == 0 && prof > 0)) {
        f->melhorScore = s;
        f->len = prof;
        if (prof > 0)
            memcpy(f->caminho, f->atual, (size_t) prof * sizeof(int));
    }

    if (s == NUM_FACELETS) return;       /* resolvido: nao precisa descer mais  */
    if (prof >= limite) return;          /* atingiu a profundidade da fase      */

    for (m = 0; m < NUM_MOVES; m++) {
        Cube filho;
        if (podarMov(ultimo, penultimo, m))
            continue;
        applyMove(&filho, estado, m);
        f->atual[prof] = m;
        (*nos)++;
        buscaFase(&filho, prof + 1, limite, m, ultimo, f, nos);
        if (f->melhorScore == NUM_FACELETS)
            return;                      /* ja achou o cubo resolvido           */
    }
}

/* resolverGuloso: roda fases ate resolver, empacar ou estourar 'maxFases'.
 * Retorna 1 se resolveu, 0 caso contrario, preenchendo 'out'.                  */
int resolverGuloso(const Cube *inicial, int profFase, int maxFases,
                   SolucaoGulosa *out, long *nosVisitados)
{
    Cube cur;
    int  total = 0, semProgresso = 0, fase;
    const int PLATO_MAX = 5;             /* fases seguidas sem ganho antes de desistir */

    copyCube(&cur, inicial);
    *nosVisitados = 0;

    for (fase = 0; fase < maxFases; fase++) {
        Fase f;
        int  scoreAntes = adesivosCorretos(&cur), i;

        if (scoreAntes == NUM_FACELETS)
            break;                       /* ja esta resolvido                   */

        f.melhorScore = -1;
        f.len = 0;
        buscaFase(&cur, 0, profFase, -1, -1, &f, nosVisitados);

        printf("  fase %s%d%s: melhor estado = %s%d/%d%s adesivos certos (%d%%)\n",
               ansi(A_BOLD), fase + 1, ansi(A_RESET),
               ansi(A_BOLD), f.melhorScore, NUM_FACELETS, ansi(A_RESET),
               (f.melhorScore * 100) / NUM_FACELETS);

        if (f.len == 0)
            break;                       /* nenhum movimento melhora: pico/otimo local */

        if (f.melhorScore <= scoreAntes) {
            if (++semProgresso > PLATO_MAX)
                break;                   /* preso num plato: desiste            */
        } else {
            semProgresso = 0;
        }

        for (i = 0; i < f.len && total < MAX_MOV_GULOSO; i++) {
            applyMove(&cur, &cur, f.caminho[i]);
            out->caminho[total++] = f.caminho[i];
        }

        if (isSolved(&cur))
            break;
    }

    out->qtd       = total;
    out->resolvido = isSolved(&cur);
    out->pct       = (adesivosCorretos(&cur) * 100) / NUM_FACELETS;
    copyCube(&out->estadoFinal, &cur);
    return out->resolvido;
}

/* imprimirSolucaoGulosa: mostra o resultado da busca por fases. */
void imprimirSolucaoGulosa(const SolucaoGulosa *s)
{
    int i;

    if (s->resolvido) {
        printf("\n%s%s== SOLUCAO ENCONTRADA (busca gulosa por fases) ==%s\n",
               ansi(A_BOLD), ansi(A_GREEN), ansi(A_RESET));
        printf("Quantidade de movimentos: %s%d%s %s(nao necessariamente minima)%s\n\n",
               ansi(A_BOLD), s->qtd, ansi(A_RESET), ansi(A_DIM), ansi(A_RESET));
    } else {
        printf("\n%s%s== NAO RESOLVIDO (a busca gulosa empacou) ==%s\n",
               ansi(A_BOLD), ansi(A_YEL), ansi(A_RESET));
        printf("Cheguei a %s%d%%%s de adesivos certos em %s%d%s movimento(s); a partir\n",
               ansi(A_BOLD), s->pct, ansi(A_RESET), ansi(A_BOLD), s->qtd, ansi(A_RESET));
        printf("daqui nenhuma sequencia curta melhora (otimo local da heuristica).\n\n");
    }

    if (s->qtd > 0) {
        printf("Movimentos: %s%s", ansi(A_BOLD), ansi(A_CYAN));
        for (i = 0; i < s->qtd; i++)
            printf("%s%s", MOVE_NAMES[s->caminho[i]], (i < s->qtd - 1) ? " " : "");
        printf("%s\n", ansi(A_RESET));
    }

    printf("\n%s%s:%s\n\n", ansi(A_BOLD),
           s->resolvido ? "Cubo resolvido" : "Melhor estado alcancado", ansi(A_RESET));
    printCube(&s->estadoFinal);
}
