# Relatório Técnico — Solucionador de Cubo Mágico 3×3 com Árvore de Estados

Trabalho de Estrutura de Dados — tema **Árvores**. Linguagem **C puro**.

---

## 1. Objetivo

Ler as cores de um cubo mágico 3×3, e usando uma **estrutura de árvore**,
calcular e exibir:

- a **quantidade** de movimentos necessários para resolver o cubo;
- a **sequência** de movimentos da solução.

A entrada é digitada ou redirecionada de um arquivo texto, na ordem de faces:
**Cima, Esquerda, Frente, Direita, Trás, Baixo**.

---

## 2. Representação do cubo (modelo *facelet*)

O cubo tem 6 faces × 9 adesivos = **54 adesivos**. Guardamos os 54 numa string
de caracteres (`Cube.f` em [cube.h](cube.h)), onde cada caractere é a cor de um
adesivo. As cores são `Y W G B R O`.

A ordem das faces no vetor segue o enunciado:

| Índices | Face |
|---|---|
| 0–8   | Cima (U) |
| 9–17  | Esquerda (L) |
| 18–26 | Frente (F) |
| 27–35 | Direita (R) |
| 36–44 | Trás (B) |
| 45–53 | Baixo (D) |

Dentro de cada face os 9 adesivos vão de 0 a 8 em linhas (esquerda→direita,
cima→baixo). O adesivo central (índice 4) **nunca se move**: ele define a cor
da face. O cubo está **resolvido** quando, em cada face, os 9 adesivos são
iguais ao seu centro — teste feito por `isSolved()`. Como comparamos cada
adesivo com o **próprio** centro, o teste funciona em qualquer orientação do
cubo.

Usar uma string facilita **copiar** estados (`copyCube`) e **comparar** se um
estado está resolvido — operações que a busca faz o tempo todo.

---

## 3. Os 12 movimentos

Os movimentos básicos são giros de 90° das faces: `U U' D D' L L' R R' F F' B B'`
(horário e anti-horário de cada face). Cada giro é uma **permutação** dos 54
adesivos, descrita por **ciclos** em [cube.c](cube.c).

Um ciclo `(a b c d)` significa: a cor de `a` vai para `b`, a de `b` para `c`, a
de `c` para `d` e a de `d` volta para `a`. Cada movimento tem 5 ciclos (2 giram
os adesivos da própria face, 3 movem os das faces vizinhas). Em `initMoves()`
montamos, a partir dos ciclos, uma **tabela de permutação** `perm[54]` por
movimento, e `applyMove()` aplica em O(54): `novo[i] = antigo[perm[i]]`.

Validação informal: aplicar o mesmo movimento 4 vezes volta ao estado original.

---

## 4. A árvore de estados (o coração do trabalho)

A ideia central:

- A **raiz** da árvore é o estado embaralhado lido da entrada.
- A partir de um estado podemos aplicar 12 movimentos; cada um leva a um novo
  estado. Esses são os (até) **12 filhos** daquele nó.
- Repetindo, formamos uma árvore que cresce a cada nível.
- Uma **folha de sucesso** é um nó cujo estado está resolvido. A sequência de
  movimentos da raiz até esse nó é a **solução**.

A struct do nó ([solver.h](solver.h)) é simples e direta:

```c
typedef struct No {
    Cube        estado;        /* estado completo do cubo neste nó            */
    int         movimento;     /* movimento (0..11) que gerou o nó; -1 = raiz */
    int         profundidade;  /* movimentos desde a raiz                     */
    struct No  *pai;           /* ponteiro para o nó pai                      */
} No;
```

O ponteiro `pai` é o que torna isto uma **árvore** explícita: a partir de
qualquer nó conseguimos subir até a raiz, e é assim que remontamos a solução.

---

## 5. Percurso: busca em profundidade + aprofundamento iterativo

Percorremos a árvore por **busca em profundidade (DFS) recursiva**. A própria
recursão desce por um galho; quando o galho não leva à solução, ela "volta"
(*backtracking*) e tenta o próximo movimento. Em pseudocódigo
(`buscaProfundidade` em [solver.c](solver.c)):

```
buscaProfundidade(no, limite):
    se no.estado está resolvido:  retorne no          # achou!
    se no.profundidade == limite: retorne NULL        # não pode descer mais
    para cada movimento m (0..11):
        se podar(no, m): pule                          # poda (ver abaixo)
        filho = aplica m em no
        r = buscaProfundidade(filho, limite)
        se r != NULL: retorne r                        # solução neste galho
        libere filho                                   # backtracking
    retorne NULL
```

DFS pura tem um problema: ela pode achar uma solução longa antes de uma curta.
Como o enunciado pede a **quantidade** (mínima) de movimentos, usamos
**aprofundamento iterativo** (`resolver` em [solver.c](solver.c)): rodamos a
DFS com limite 0, depois 1, depois 2, … até `PROF_MAXIMA`. Como os limites
crescem, **a primeira solução encontrada é a mais curta**.

```
resolver(inicial, profMax):
    para limite = 0 até profMax:
        raiz = nó(inicial)
        sol = buscaProfundidade(raiz, limite)
        se sol != NULL: retorne sol
    retorne NULL                 # não resolvível dentro de profMax
```

### Poda dos galhos (`podar` em [solver.c](solver.c))

**Podar um galho** é decidir, *antes* de gerar um filho, que aquele movimento
só levaria a sequências redundantes — e então nem criar esse ramo da árvore.
Cortar perto da raiz é o que mais economiza, porque elimina **toda** a subárvore
que cresceria embaixo (e ela cresceria ~12× por nível).

Usamos três podas, todas **seguras**: para cada galho cortado existe sempre uma
sequência igual ou mais curta que sobrevive, então **a menor solução nunca é
perdida**.

1. **Não desfazer o último movimento.** `R` seguido de `R'` só volta ao nó pai.
   Corta o inverso do último movimento.
2. **Faces opostas em ordem fixa.** Faces opostas **comutam** (girar `R` e `L`
   não interfere uma na outra), então `R L` e `L R` chegam ao mesmo estado. Para
   não explorar as duas ordens, fixamos uma: depois de uma face só permitimos a
   face oposta se ela vier "em ordem" (número de face maior). Cada par
   comutativo passa a aparecer **uma só vez**.
3. **Mesma face sem repetição inútil.** Dois giros já bastam para qualquer
   efeito numa face: (a) das duas formas de dar a meia-volta (`R R` e `R' R'`,
   ambas iguais a girar 180°) guardamos só uma; (b) nunca giramos a mesma face
   **3 vezes** seguidas, pois `R R R = R'` (mais curto).

Por que são seguras? São todas baseadas em **equivalências de movimentos**
(comutar faces opostas, `R R R = R'`, `R R = R' R'`). Sempre que um galho é
cortado, a sequência equivalente — de mesmo comprimento ou menor — continua
sendo explorada por outro galho. Logo o aprofundamento iterativo ainda encontra
uma solução de comprimento mínimo.

**Efeito.** Sozinha, a poda (1) deixava o fator de ramificação em ~11. Com (2) e
(3) o fator efetivo cai para cerca de **9,5** movimentos por nó. Como o custo é
exponencial na profundidade, o ganho se acumula: em 7 níveis, `(9,5/11)^7 ≈
0,36`, ou seja, **cerca de 1/3 dos nós** — perto de **3× menos trabalho** que
antes, sem mudar a estrutura (continua sendo uma árvore comum, só com galhos
podados). Os números exatos aparecem na execução, em "Nos gerados na arvore".

---

## 6. Reconstrução do caminho

Achado o nó-solução, `remontarCaminho()` sobe pelos ponteiros `pai` coletando
os movimentos e inverte a lista (para dar a ordem raiz→solução).
`imprimirSolucao()` então mostra a quantidade de movimentos, a sequência e um
desenho em ASCII do galho percorrido.

---

## 7. Validação da entrada

`isValidColors()` aceita a entrada se:

1. tem exatamente 54 caracteres, todos em `Y W G B R O`;
2. há **9 de cada** cor;
3. os **6 centros** (índice 4 de cada face) são todos **diferentes**.

Optamos por uma validação simples e explicável. Não verificamos a
"resolubilidade física" do cubo por teoria de grupos: se o cubo for válido em
cores mas impossível de montar, a busca simplesmente não acha solução dentro do
limite e o programa avisa.

---

## 8. Complexidade

Seja `b` o fator de ramificação (~9,5 após as três podas) e `d` a profundidade
da solução.

- **Tempo:** O(b^d). Sem poda a árvore cresceria 12× por nível; as podas baixam
  esse fator efetivo para ~9,5. O número de nós realmente gerados é impresso em
  "Nos gerados na arvore" ao final da execução — rode `make run` para conferir o
  valor exato na sua máquina.
- **Memória:** O(d). Como liberamos cada galho ao desistir dele, só ficam vivos
  os nós do caminho atual — proporcional à **profundidade**, não ao tamanho da
  árvore. Essa é a grande vantagem da DFS sobre a busca em largura aqui.

O aprofundamento iterativo refaz os níveis rasos a cada iteração, mas como o
último nível domina a contagem (cresce exponencialmente), o custo extra é
pequeno.

---

## 9. Limite prático da busca em árvore

Este é um ponto importante e honesto do trabalho. O número de Deus do cubo 3×3
é **20**: existem embaralhamentos que exigem 20 movimentos. Mas uma busca em
árvore com b≈11 chega a, na prática, ~7 movimentos em tempo razoável (acima
disso o número de estados explode: 11²⁰ ≈ 10²⁰).

Logo, **esta abordagem resolve apenas cubos pouco embaralhados** (até ~7
giros). Um cubo totalmente embaralhado à mão **não** é resolvível por busca em
árvore em tempo viável — resolvê-lo exigiria um algoritmo de outra natureza
(método das camadas, Kociemba etc.), que não usa árvore de busca. Quando a
solução não cabe no limite, o programa informa isso claramente, em vez de
travar.

Para demonstrar o programa funcionando, geramos um embaralhamento curto com
`./cubo scramble N` (N ≤ 7) e resolvemos a string gerada.

---

## 9.1. Plano B: busca gulosa por fases

Quando a busca exata **não** resolve dentro do limite, em vez de só desistir o
programa tenta uma **busca gulosa por fases** (`resolverGuloso` em
[solver.c](solver.c)). A ideia abre mão da solução mínima em troca de conseguir
fazer **progresso** em cubos mais embaralhados:

1. A partir do estado atual, montamos **uma árvore comum** de profundidade
   `PROF_FASE` (= 7) e, em vez de procurar o cubo resolvido, procuramos a
   **folha mais próxima de resolvida**. A proximidade é medida por uma
   **heurística** simples — `adesivosCorretos()`, que conta quantos dos 54
   adesivos já estão na cor do centro da própria face (54 = resolvido).
2. Aplicamos os movimentos até essa folha. O cubo fica "mais perto".
3. Tratamos o novo estado como uma **raiz nova** e repetimos a fase.

É a **mesma estrutura de antes** — uma árvore por fase, percorrida em
profundidade, com a poda da seção 5 — só que guiada por uma função de avaliação.
Não há estrutura nova nem grafo: continua sendo árvore.

**Limitações (assumidas honestamente):**

- A solução **não é mínima** (sai em múltiplos de até `PROF_FASE` movimentos).
- A heurística "adesivos certos" é **enganosa** no cubo: às vezes é preciso
  *piorar* temporariamente para depois resolver. Por isso a busca gulosa pode
  **empacar num ótimo local** — um ponto em que nenhuma sequência de 7
  movimentos melhora a contagem. Quando isso acontece (após algumas fases sem
  ganho), o programa para e **reporta o melhor estado alcançado** (em %), em vez
  de rodar para sempre. Olhar 7 movimentos à frente torna a busca bem mais
  robusta que uma gulosa de 1 movimento, mas não elimina os ótimos locais.

Ou seja: a busca exata continua sendo o coração do trabalho (e dá a solução
mínima quando cabe); a gulosa é um complemento que estende o alcance, deixando
claro o que ganha (profundidade) e o que perde (otimalidade e garantia).

---

## 10. Árvore × Grafo

O espaço de estados do cubo é, a rigor, um **grafo** (estados diferentes podem
levar ao mesmo estado por caminhos diferentes — por exemplo, `R R R R` volta ao
início). Ao explorá-lo como **árvore** a partir da raiz, esses reencontros
viram nós repetidos em galhos distintos. Não mantemos uma tabela global de
estados visitados (que economizaria trabalho, mas custaria muita memória e
fugiria de uma "árvore comum"); em vez disso usamos podas **locais** (seção 5),
que olham só os últimos movimentos do galho atual. Para a faixa de profundidade
que tratamos, isso é suficiente e mantém o código simples e fiel à ideia de
**árvore**.

---

## 11. Compilação e execução

```bash
make                      # compila o executável 'cubo'

./cubo entrada.txt        # resolve o cubo do arquivo (até 7 movimentos)
./cubo entrada.txt 8      # resolve permitindo até 8 movimentos
./cubo scramble 5         # gera um cubo embaralhado com 5 giros (para testar)
```

### Exemplo de saída

> *Ilustrativo.* O número de "Nos gerados" e a sequência exata da solução podem
> variar conforme a poda e a ordem de exploração; o que está garantido é a
> **quantidade mínima** de movimentos.

```
=== Cubo Magico 3x3 - Solucionador por Arvore de Estados ===

Estado inicial informado:
        W G G
        W W Y
        W W Y
...
Busca em profundidade (aprofundamento iterativo), ate 7 movimentos:
  procurando solucao de ate 0 movimento(s)...
  ...
  procurando solucao de ate 4 movimento(s)...

Nos gerados na arvore: 18704

=== SOLUCAO ENCONTRADA ===
Quantidade de movimentos: 4

Movimentos: B' R D' R

Galho da arvore percorrido ate a solucao:
(raiz: cubo embaralhado)
  |_ B'   (nivel 1)
    |_ R   (nivel 2)
      |_ D'   (nivel 3)
        |_ R   (nivel 4)
          >> cubo resolvido
```
