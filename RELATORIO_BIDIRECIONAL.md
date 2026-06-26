# Relatório — Código 2: Busca Bidirecional com Árvore Binária de Busca (BST)

Complemento do trabalho de Árvores. Este documento explica o **Código 2**
(`cubo_bid`), que resolve cubos bem mais embaralhados que o Código 1.

> **Resumo de uma frase:** em vez de uma árvore descendo 12 níveis (inviável),
> usamos **duas árvores de 6 níveis** que se encontram no meio, e uma **Árvore
> Binária de Busca (BST)** para descobrir, rápido, quando elas se encontram.

---

## 1. Por que o Código 1 não chega a 12 movimentos

A árvore de estados cresce ~9,5× por nível (após a poda). Logo:

| Profundidade | Nós a explorar | Tempo |
|---|---|---|
| 8  | ~10⁸ | dezenas de segundos |
| 10 | ~10⁹ | horas |
| 12 | ~5×10¹¹ | dias |

Uma árvore **única** simplesmente não alcança 12 movimentos. Não é questão de
otimizar o código — é a explosão exponencial.

---

## 2. A ideia: encontrar-se no meio (busca bidirecional)

Um caminho de 12 movimentos do **embaralhado** até o **resolvido** tem um
**ponto médio**. Em vez de procurar o caminho inteiro de um lado só, crescemos
**duas árvores curtas** até esse ponto médio:

```
 EMBARALHADO ──(árvore da frente, 6 níveis)──►  M  ◄──(árvore de trás, 6 níveis)── RESOLVIDO
```

- **Árvore de trás:** a partir do cubo **resolvido**, geramos todos os estados
  alcançáveis em até 6 movimentos.
- **Árvore da frente:** a partir do cubo **embaralhado**, exploramos até 6
  movimentos.
- Se um estado `M` aparece **nas duas**, juntamos: caminho da frente
  (embaralhado→M) + inverso do caminho de trás (M→resolvido) = **solução**.

### O ganho (o argumento central)

| | Uma árvore (12) | Duas árvores (6 + 6) |
|---|---|---|
| Nós | ~500.000.000.000 | ~735.000 + ~735.000 ≈ **1,5 milhão** |
| Tempo | dias | **segundos** |

Como `12 = 6 + 6` e o custo é exponencial, **somar duas árvores de 6 é
incomparavelmente mais barato que uma árvore de 12** (`a^6 + a^6 ≪ a^12`).

A solução encontrada é **mínima**: procuramos o menor total
`(profundidade da frente) + (profundidade de trás)`.

---

## 3. Onde entra a BST (e por que ela é necessária)

O passo crítico é, para cada estado da árvore da frente, perguntar:
**"este estado já foi alcançado pela árvore de trás?"**

Se guardássemos os ~735 mil estados de trás numa **lista** e procurássemos um a
um, cada consulta seria **O(n)** — e fazemos centenas de milhares de consultas.
Inviável.

A solução é guardar os estados de trás numa **Árvore Binária de Busca (BST)**
([bst.h](bst.h) / [bst.c](bst.c)):

- A **chave** de cada nó é a própria string de 54 cores do cubo (`estado.f`).
- Comparamos com `strcmp`: estado "menor" vai para a subárvore **esquerda**,
  "maior" para a **direita**.
- Assim cada **inserção** e cada **busca** custam **O(log n)** em média.

Cada nó da BST guarda também o **caminho** (sequência de movimentos do resolvido
até aquele estado), para depois remontar a solução.

```c
typedef struct NoBST {
    Cube           estado;      /* chave: as 54 cores                    */
    unsigned char  caminho[12]; /* movimentos: resolvido -> estado       */
    int            len;         /* tamanho do caminho                    */
    struct NoBST  *esq, *dir;   /* filhos (BST)                          */
} NoBST;
```

> **Duas árvores clássicas no mesmo trabalho:** a *árvore de estados* (n-ária,
> percorrida em profundidade) e a *árvore binária de busca* (que casa os
> estados). Nenhuma tabela hash, nenhum grafo — é tudo árvore.

---

## 4. Como a solução é remontada

Achado o encontro `M`:

- **Frente:** `fwd[0..a-1]` leva embaralhado → `M` (já na ordem certa).
- **Trás:** `caminho[0..b-1]` leva resolvido → `M`. Para ir de `M` → resolvido,
  **invertemos cada movimento e percorremos de trás para frente**
  (`R` vira `R'`, e a ordem se inverte).

A solução final é a concatenação das duas metades. O programa ainda **aplica a
sequência ao cubo inicial e confere** que ele fica resolvido (`isSolved`),
garantindo que a remontagem está correta.

---

## 5. Poda

A mesma poda do Código 1 vale aqui, nos **dois** sentidos (ver função `podarMov`
em [solver_bid.c](solver_bid.c)): não cancelar o último movimento, fixar a ordem
de faces opostas e não repetir a mesma face além do necessário. Ela só remove
sequências **redundantes**, então não perde nenhum estado alcançável — o
encontro no meio continua garantido.

---

## 6. Custo

Sejam `f` e `t` as profundidades da frente e de trás (`f + t` = limite).

- **Tempo:** O(b^f) para a frente + O(b^t) para construir a BST, mais
  O(log n) por consulta. Para 6 + 6: ~1,5 milhão de nós, alguns segundos.
- **Memória:** a BST guarda os ~10⁶ estados distintos de até `t` movimentos
  (~100 MB para `t = 6`). Esse é o **preço** da técnica: trocamos tempo por
  memória. Para usar menos memória, rode com `t` menor, por exemplo
  `./cubo_bid entrada.txt 7 5` (frente 7, trás 5) — a BST cai para ~10⁵ estados.

Esse contraste é honesto e didático: o Código 1 usa **O(profundidade)** de
memória (só o galho atual) mas não escala em profundidade; o Código 2 escala até
~12 movimentos **gastando memória** para guardar metade do caminho na BST.

---

## 7. Limite

Com `f + t` movimentos no total, resolve qualquer cubo cuja solução mínima tenha
até `f + t` movimentos (padrão 12). Acima disso, aumente `f`/`t` — mas a memória
da BST cresce exponencialmente com `t`, então há um teto prático (~7 de cada
lado). O número de Deus é 20; nem a busca bidirecional simples alcança 20 sem
técnicas adicionais (tabelas de padrões etc.), que fogem do escopo de árvores.

---

## 8. Como compilar e executar

```bash
make                       # compila 'cubo' (Codigo 1) e 'cubo_bid' (Codigo 2)

./cubo_bid entrada.txt     # padrao: resolve ate 12 movimentos
./cubo_bid entrada.txt 15  # informa o TOTAL; o programa divide em 8 + 7 sozinho
./cubo_bid entrada.txt 8 7 # split manual (frente, tras)
```

Passando só o **total**, o programa escolhe o split: dá metade para o lado de
trás (limitada a 7, para a memória da BST não estourar) e o resto para a frente.
Assim `12` vira 6+6, `13` vira 7+6, `14` vira 7+7 e `15` vira 8+7.
