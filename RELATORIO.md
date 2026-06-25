# Relatório Técnico — Solucionador de Cubo Mágico 3×3 com Árvores

**Disciplina:** Estrutura de Dados — Árvores e Grafos
**Linguagem:** C puro (padrão C90/C99, compilável com `gcc`)

---

## 1. Objetivo

Demonstrar, na prática, o uso de **árvores** como estrutura de dados para
resolver um problema de busca em espaço de estados: encontrar a sequência de
movimentos que leva um Cubo Mágico 3×3 embaralhado de volta ao estado
resolvido.

Cada **estado** do cubo é um nó; cada **movimento** (giro de uma face) cria um
nó filho. O conjunto de todos esses nós e ligações forma a **árvore de busca**.

---

## 2. Estruturas de Dados Utilizadas

### 2.1. `Cube` — o estado do cubo (`cube.h`)

```c
typedef struct {
    char f[NUM_FACELETS + 1];   /* 54 cores + '\0' */
} Cube;
```

O cubo é representado pelo **modelo de adesivos (facelets)**: um vetor de 54
caracteres, um por adesivo. Guardar como string traz três vantagens diretas:

- **comparar** estados é `strcmp`;
- **copiar** estados é `memcpy`/`strcpy`;
- **calcular o hash** (controle de visitados) é trivial sobre a string.

A ordem das faces no vetor é `U L F R B D` (Cima, Esquerda, Frente, Direita,
Trás, Baixo), conforme o enunciado, com 9 adesivos por face numerados:

```
0 1 2
3 4 5
6 7 8
```

O adesivo central (índice 4) de cada face nunca se move e define a cor
"verdadeira" daquela face.

### 2.2. `Node` — nó da árvore (`solver.h`)

```c
typedef struct Node {
    Cube          state;        /* estado completo do cubo            */
    int           move;         /* movimento que gerou o nó (-1=raiz) */
    int           depth;        /* profundidade = nº de movimentos    */
    struct Node  *parent;       /* ponteiro para o pai                */
    struct Node **children;     /* vetor dinâmico de filhos           */
    int           numChildren;  /* nº de filhos criados               */
    int           g, h, f;      /* custos da busca A* (f = g + h)     */
} Node;
```

É o tipo central do trabalho. Cada nó:

- **armazena o estado completo** do cubo (`state`);
- **sabe qual movimento** o originou (`move`) e em **qual profundidade** está
  (`depth`);
- aponta para o **pai** (`parent`) — isso permite **reconstruir o caminho** da
  solução subindo até a raiz;
- mantém uma **lista de filhos** (`children`), um vetor que cresce com
  `realloc` à medida que o nó é expandido;
- guarda `g`, `h` e `f` para a busca **A\***.

### 2.3. `Tree` — a árvore (`solver.h`)

```c
typedef struct {
    Node  *root;        /* raiz = estado inicial            */
    Node **allNodes;    /* repositório de todos os nós       */
    int    count;       /* nº de nós alocados                */
    int    capacity;    /* capacidade do vetor allNodes      */
} Tree;
```

Além da `root`, a árvore mantém um **vetor com todos os nós alocados**. Isso
não é obrigatório para a busca, mas torna a **liberação de memória** (`free`)
simples e segura: ao final, percorremos `allNodes` uma única vez liberando cada
nó e seu vetor de filhos.

### 2.4. Tabela hash de estados visitados (`solver.c`)

```c
typedef struct HashEntry {
    char state[NUM_FACELETS + 1];
    struct HashEntry *next;     /* encadeamento separado */
} HashEntry;
```

Hash com **encadeamento separado** (cada balde é uma lista ligada). A chave é a
string de 54 cores e a função de espalhamento é a **djb2**. Serve para
**evitar estados repetidos** (ver seção 5).

### 2.5. Estruturas auxiliares de busca

- **Fila (FIFO)** para o BFS: vetor dinâmico com índices `head`/`tail`.
- **Heap binário de mínimo** para o A*: vetor onde o pai do índice `i` é
  `(i-1)/2`, ordenado por `f`. `heapPush`/`heapPop` em O(log n).

---

## 3. Como a Árvore é Construída

1. A árvore nasce com **um único nó raiz**, contendo o estado inicial informado
   pelo usuário (`createTree`).
2. A busca retira um nó e o **expande** (`generateChildren`): para cada um dos
   12 movimentos possíveis (`U U' D D' L L' R R' F F' B B'`), calcula o estado
   resultante com `applyMove`.
3. Cada estado novo (não visitado) vira um **nó filho**, ligado ao pai por
   ponteiro, e é registrado na árvore.
4. O processo se repete sobre os filhos, **expandindo a árvore em níveis**, até
   encontrar um nó cujo estado esteja resolvido (`isSolved`).

### Como os nós são expandidos

A expansão de um nó está em `generateChildrenInternal`. Para cada movimento,
aplicamos duas **podas** (otimizações que mantêm a corretude):

- **Poda 1 — não desfazer o último movimento:** se o nó foi gerado por `R`, não
  geramos `R'` a partir dele (desfazer nunca encurta a solução).
- **Poda 2 — estados repetidos:** se o estado resultante já está na tabela
  hash, ele é descartado.

Os movimentos são implementados como **permutações de adesivos**. Cada giro de
90° é descrito por ciclos `(a b c d)` ("o conteúdo de `a` vai para `b`…"),
derivados de um modelo geométrico 3D do cubo e **validados por teste
automático** (aplicar qualquer movimento 4 vezes retorna ao estado original;
um movimento seguido de seu inverso também).

---

## 4. Algoritmos de Busca

### 4.1. BFS — Busca em Largura (padrão)

Explora a árvore **nível a nível**: todos os estados a 1 movimento, depois a 2,
etc. Como avança por profundidade crescente, **o primeiro estado resolvido
encontrado está no menor número de movimentos possível** (solução ótima). Usa
uma **fila FIFO**.

### 4.2. A* — Best-First com heurística

A* expande sempre o nó de menor `f = g + h`:

- `g` = profundidade (movimentos já feitos);
- `h` = **heurística admissível**: como um giro reposiciona no máximo 20
  adesivos, usamos `h = teto(adesivos_fora_do_lugar / 20)`. Por **nunca
  superestimar** o custo restante, A* continua encontrando a solução **ótima**,
  porém visitando **muito menos nós** que o BFS.

> **Resultado medido** no exemplo (`R U F' U'`): BFS gerou **17.065 nós**; A*
> gerou apenas **800 nós** — ambos encontraram a mesma solução de 4 movimentos
> `U F U' R'`. Isso ilustra concretamente o ganho da heurística.

### 4.3. BFS Bidirecional — alcançando cubos mais profundos

O custo do BFS comum é `O(b^d)`: cada movimento a mais multiplica o número de
nós por ~`b`. Para `d` grande isso estoura a memória. A **busca bidirecional**
ataca exatamente esse ponto: em vez de uma árvore crescendo do estado inicial
até a solução, crescemos **duas árvores ao mesmo tempo** —

- uma a partir do **estado embaralhado**;
- outra a partir do **estado resolvido**.

Quando as duas fronteiras se **encontram** num estado comum, a solução é a
junção dos dois meio-caminhos. Como cada árvore só precisa atingir
profundidade `d/2`, o custo cai de `O(b^d)` para `O(b^{d/2})` — uma redução
enorme (a raiz quadrada do número de nós).

**Detecção do encontro:** cada lado guarda seus estados num **mapa
estado → nó** (`NodeMap`). Ao gerar um filho, consultamos o mapa do lado
oposto; se o estado já existe lá, achamos a ponte.

**Reconstrução:** o caminho final é
`(início → encontro)` da árvore-da-frente, concatenado com
`(encontro → resolvido)`, que é o ramo da árvore-de-trás percorrido ao
contrário, **trocando cada movimento pelo seu inverso** (pois aquele ramo foi
construído partindo do resolvido).

> **Resultado medido:** um cubo embaralhado que o BFS comum **não resolvia**
> (parava ao gerar 4.000.000 de nós) foi resolvido pelo bidirecional em
> **10 movimentos**, gerando apenas **~180.000 nós** e usando **~64 MB**.
> Alcance prático: ~11–12 movimentos, contra ~7 do BFS/A* comuns.

---

## 4.4. Verificação de Validade (cubo possível × impossível)

Ter 9 adesivos de cada cor **não garante** que o cubo possa existir: muitas
montagens são fisicamente impossíveis (não se chega a elas por nenhuma
sequência de giros). A função `isSolvable` (em `cube.c`) testa as **três
invariantes do grupo do cubo**:

1. **Paridade:** a permutação dos 8 cantos e a dos 12 meios têm a mesma
   paridade.
2. **Orientação dos cantos:** a soma das torções é múltiplo de 3.
3. **Orientação das arestas:** a soma dos *flips* é par.

Se qualquer uma falhar, o programa imprime `Estado invalido do cubo.`
(atendendo ao requisito de detectar estados impossíveis). As tabelas de
cantos/arestas foram **validadas empiricamente**: 20.000 embaralhamentos
aleatórios foram todos reconhecidos como solúveis, e estados corrompidos
(canto torcido, aresta invertida, troca de peças) como impossíveis.

## 5. Controle de Estados Visitados

O mesmo estado pode ser alcançado por caminhos diferentes (ex.: `R U` e outra
sequência podem coincidir). Sem controle, a árvore exploraria estados repetidos
indefinidamente. A **tabela hash** registra cada estado já gerado: um filho só é
criado se seu estado for inédito (`visitedCheckAndAdd` retorna 0). É isso que
mantém a busca finita e eficiente.

---

## 6. Reconstrução do Caminho

Quando a solução é encontrada, temos apenas o **nó folha**. Para obter a
sequência de movimentos, `reconstructPath` sobe pelos ponteiros `parent` até a
raiz, empilhando os nós, e depois **inverte a ordem** (raiz → solução). Os
campos `move` de cada nó do caminho formam a sequência de movimentos.

---

## 7. Análise de Complexidade

Seja `b` o fator de ramificação (≈ 12 movimentos, ~11 após a poda de não
desfazer) e `d` a profundidade da solução (número de movimentos).

### Complexidade Temporal

- **BFS:** `O(b^d)` no pior caso — explora todos os nós até o nível `d`.
  Com a tabela hash, o número real de nós é limitado pela quantidade de
  **estados distintos** alcançáveis em até `d` movimentos.
- **A\*:** também `O(b^d)` no pior caso, mas a heurística admissível **poda
  fortemente** a busca na prática (no exemplo, ~21× menos nós).

### Complexidade Espacial

- **BFS:** `O(b^d)` — precisa guardar todos os nós da fronteira e os visitados.
  Este é o gargalo do BFS (consome muita memória em profundidades grandes).
- **A\*:** `O(b^d)` no pior caso (mantém abertos + visitados), mas tipicamente
  bem menor pela poda heurística.

Por isso o programa define limites de segurança (`MAX_DEPTH`, `MAX_NODES`):
o espaço de estados do cubo é gigantesco (≈ 4,3×10¹⁹ estados) e o número de
Deus é 20, de modo que embaralhamentos profundos são inviáveis por busca cega.
Para um **trabalho de Estrutura de Dados**, o foco é demonstrar a árvore e a
busca em embaralhamentos rasos (até ~7 movimentos), o que os algoritmos
resolvem rapidamente.

---

## 8. Por que uma Árvore? Diferença entre Árvore e Grafo neste Problema

### Por que árvore foi utilizada

O problema é naturalmente **hierárquico**: partimos de um estado raiz e, a cada
movimento, geramos novos estados "filhos". Essa relação **pai → filhos** é
exatamente uma **árvore de busca**:

- a **raiz** é o estado inicial;
- cada **aresta** é um movimento;
- cada **caminho** da raiz a um nó é uma sequência de movimentos;
- o **ponteiro para o pai** permite reconstruir a solução trivialmente.

A árvore expressa de forma direta "qual movimento levou a qual estado",
que é o que precisamos relatar ao usuário.

### Árvore × Grafo

O **espaço de estados do cubo é, na verdade, um GRAFO**: um mesmo estado pode
ser alcançado por várias sequências de movimentos diferentes (há ciclos — por
exemplo, aplicar um movimento 4 vezes volta ao mesmo estado). Num grafo, um
"nó" (estado) pode ter **vários pais**.

Neste trabalho impomos uma **estrutura de árvore sobre esse grafo**: ao usar a
**tabela hash de visitados**, aceitamos cada estado **uma única vez** e
ignoramos as arestas que reencontrariam um estado já visto. O resultado é a
**árvore de busca** (uma *spanning tree* do grafo de estados a partir da raiz):
cada estado tem exatamente **um** pai — o caminho pelo qual foi descoberto.

Resumindo:

| Aspecto | Grafo de estados (real) | Árvore de busca (usada) |
|---|---|---|
| Pais por nó | vários | exatamente um |
| Ciclos | sim | não (eliminados pela hash) |
| Caminho raiz→nó | muitos | único |
| Estrutura em memória | lista de adjacências | nós com `parent` + `children` |

Ou seja: a árvore é a **visão de busca** que extraímos do grafo de estados, e é
o que torna a reconstrução do caminho e a visualização (ASCII e Graphviz)
simples e inequívocas.

---

## 9. Organização dos Arquivos

| Arquivo | Responsabilidade |
|---|---|
| `cube.h` / `cube.c` | estrutura `Cube`, movimentos, `initCube`, `applyMove`, `copyCube`, `isSolved`, validação |
| `solver.h` / `solver.c` | `Node`, `Tree`, hash de visitados, `generateChildren`, `bfsSolve`, `astarSolve`, reconstrução do caminho, `printSolution` |
| `graphviz.h` / `graphviz.c` | `exportGraphviz` (arquivo `.dot`) |
| `main.c` | leitura/validação da entrada, orquestração e saída |

Funções exigidas pelo enunciado, todas presentes: `initCube`, `applyMove`,
`copyCube`, `isSolved`, `generateChildren`, `bfsSolve`, `printSolution`,
`exportGraphviz` (+ `astarSolve` e `reconstructPath`).
