# Solucionador de Cubo Mágico 3×3 com Árvore de Estados

Trabalho de Estrutura de Dados (Árvores) em **C puro**. O programa monta uma
**árvore de estados** do cubo (a raiz é o cubo embaralhado; cada nó tem até 12
filhos, um por movimento) e a percorre por **busca em profundidade recursiva
com aprofundamento iterativo**, encontrando a **menor** sequência de movimentos
que resolve o cubo.

## Duas versões

São dois programas que compartilham o mesmo modelo do cubo:

| | Executável | Estrutura | Alcance |
|---|---|---|---|
| **Código 1** | `cubo` | uma árvore de estados (DFS + poda) e, como reserva, uma busca gulosa por fases | resolve **mínimo** até ~7-8 movimentos |
| **Código 2** | `cubo_bid` | **duas** árvores que se encontram no meio + uma **Árvore Binária de Busca (BST)** para casar os estados | resolve **mínimo** até ~12 movimentos |

O Código 1 mostra a árvore de busca clássica e seu limite prático; o Código 2
mostra como **duas árvores curtas + uma BST** vencem a explosão exponencial.
Detalhes do Código 2 em [RELATORIO_BIDIRECIONAL.md](RELATORIO_BIDIRECIONAL.md).

## Arquivos

```
cube.h / cube.c           -> representação do cubo (54 adesivos) e os 12 movimentos
solver.h / solver.c       -> Código 1: a árvore (struct No), DFS e a busca gulosa
main.c                    -> Código 1: programa principal
bst.h / bst.c             -> Código 2: a Árvore Binária de Busca de estados
solver_bid.h / solver_bid.c -> Código 2: a busca bidirecional (encontro no meio)
main_bid.c                -> Código 2: programa principal
entrada.txt               -> exemplo de entrada
Makefile                  -> compilação dos dois executáveis
RELATORIO.md              -> relatório do Código 1
RELATORIO_BIDIRECIONAL.md -> relatório do Código 2 (bidirecional + BST)
```

## Compilação

```bash
make                # compila os dois: 'cubo' e 'cubo_bid'
# ou, diretamente:
gcc main.c cube.c solver.c -o cubo
gcc main_bid.c cube.c solver_bid.c bst.c -o cubo_bid
```

## Execução

```bash
# Código 1 (uma árvore; até ~7-8 movimentos)
./cubo entrada.txt        # resolve o cubo do arquivo
./cubo entrada.txt 8      # permite até 8 movimentos
./cubo scramble 5         # gera um cubo embaralhado com 5 giros (para testar)

# Código 2 (bidirecional + BST; até ~12 movimentos)
./cubo_bid entrada.txt    # frente=6, tras=6
./cubo_bid entrada.txt 7 5  # frente=7, tras=5 (usa menos memória)
```

Também aceita entrada redirecionada:

```bash
./cubo /dev/stdin < entrada.txt
```

## Formato da entrada

São **54 cores** na ordem de faces **Cima, Esquerda, Frente, Direita, Trás,
Baixo** (e, dentro de cada face, da esquerda para a direita e de cima para
baixo). Cores: `Y` amarelo, `W` branco, `G` verde, `B` azul, `R` vermelho,
`O` laranja.

Pode ser uma **string única**:

```
WGGWWYWWYROOWOOWBBGGBGGBOOBRRORRGRRGYYGRBBRBBYYWYYWOOY
```

O leitor ignora espaços, vírgulas e quebras de linha, então também aceita uma
face por linha. Linhas iniciadas por `#` são comentários.

A entrada é válida se tiver 54 cores, **9 de cada** e os **6 centros
diferentes** (uma cor por face). Caso contrário:

```
Estado invalido do cubo.
```

## Como funciona a árvore (resumo)

- **Raiz**: o cubo embaralhado que foi lido.
- **Filhos**: a partir de um estado há 12 movimentos possíveis
  (`U U' D D' L L' R R' F F' B B'`); cada um gera um estado filho.
- **Folha de sucesso**: um nó cujo estado está resolvido (cada face de uma só
  cor). O caminho da raiz até ele é a solução.
- **Percurso**: busca em profundidade (DFS) recursiva. A recursão desce por um
  galho até um limite de profundidade; se não achou a solução, volta
  (*backtracking*) e tenta o próximo movimento.
- **Aprofundamento iterativo**: repetimos a DFS com limite 0, 1, 2, … A
  primeira solução encontrada é a **mais curta** (menor número de movimentos).
- **Poda de galhos**: antes de gerar um filho, descartamos movimentos que só
  levariam a sequências redundantes — (1) não desfazer o último movimento,
  (2) fixar uma ordem para faces opostas (que comutam) e (3) não repetir a mesma
  face além de dois giros. Todas são seguras (não perdem a solução mínima) e
  baixam o fator de ramificação de ~12 para ~9,5, gerando cerca de **3× menos
  nós** em 7 movimentos.
- **Memória**: ficam vivos apenas os nós do galho atual — proporcional à
  profundidade, não ao tamanho total da árvore.

## Limite (importante para a apresentação)

A árvore cresce muito por nível (sem poda, 12×; com as podas, ~9,5×). Em 7
movimentos ainda são milhões de estados; em 20 movimentos seria ~10¹⁹, inviável.
Por isso a busca em árvore resolve apenas cubos **pouco embaralhados** (até ~7
giros em segundos). Um cubo
totalmente embaralhado (que pode exigir até 20 movimentos) **não** é resolvido
por essa abordagem — o programa avisa isso de forma clara em vez de travar.

Para demonstrar, gere um embaralhamento curto e resolva:

```bash
./cubo scramble 5   # mostra os giros e a string gerada
# copie a string para um arquivo e rode ./cubo nesse arquivo
```

### Plano B: busca gulosa por fases

Se o cubo **não** couber no limite exato, o programa não desiste: entra numa
**busca gulosa por fases**. Cada fase monta uma árvore de profundidade 7 e dá os
movimentos que deixam o cubo **mais perto de resolvido** (medido por quantos
adesivos já estão na cor do próprio centro), tratando o resultado como uma nova
raiz e repetindo. Continua sendo **só árvore** + uma função de avaliação.

Em troca de alcançar cubos mais embaralhados, ela **abre mão da solução mínima**
e **pode empacar num ótimo local** (a heurística é enganosa — às vezes é preciso
piorar para depois resolver). Quando empaca, o programa informa o melhor estado
alcançado, em %. Detalhes na seção 9.1 do [RELATORIO.md](RELATORIO.md).

## Detalhes técnicos

Veja [RELATORIO.md](RELATORIO.md): a estrutura de dados do cubo, a struct `No`
da árvore, a busca em profundidade com aprofundamento iterativo, a reconstrução
do caminho pelos ponteiros `pai`, a análise de complexidade e por que a busca
em árvore tem um limite prático de profundidade.
