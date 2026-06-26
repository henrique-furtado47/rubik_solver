# Solucionador de Cubo Mágico 3×3 com Árvore de Estados

Trabalho de Estrutura de Dados (Árvores) em **C puro**. O programa monta uma
**árvore de estados** do cubo (a raiz é o cubo embaralhado; cada nó tem até 12
filhos, um por movimento) e a percorre por **busca em profundidade recursiva
com aprofundamento iterativo**, encontrando a **menor** sequência de movimentos
que resolve o cubo.

## Arquivos

```
cube.h / cube.c       -> representação do cubo (54 adesivos) e os 12 movimentos
solver.h / solver.c   -> a árvore (struct No) e a busca em profundidade (DFS)
main.c                -> programa principal (leitura, validação, impressão)
entrada.txt           -> exemplo de entrada (cubo embaralhado em 4 giros)
Makefile              -> compilação
RELATORIO.md          -> relatório técnico completo
```

## Compilação

```bash
make
# ou, diretamente:
gcc main.c cube.c solver.c -o cubo
```

## Execução

```bash
./cubo entrada.txt        # resolve o cubo do arquivo (até 7 movimentos)
./cubo entrada.txt 8      # resolve permitindo até 8 movimentos
./cubo scramble 5         # gera um cubo embaralhado com 5 giros (para testar)
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

## Detalhes técnicos

Veja [RELATORIO.md](RELATORIO.md): a estrutura de dados do cubo, a struct `No`
da árvore, a busca em profundidade com aprofundamento iterativo, a reconstrução
do caminho pelos ponteiros `pai`, a análise de complexidade e por que a busca
em árvore tem um limite prático de profundidade.
