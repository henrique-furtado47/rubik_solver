/* ==========================================================================
 *  cube.h  -  Definicao do Cubo Magico 3x3 e das operacoes basicas
 * ==========================================================================
 *
 *  Este cabecalho descreve a ESTRUTURA DE DADOS que representa um estado do
 *  cubo e declara as funcoes que manipulam esse estado.
 *
 *  --- Como o cubo e representado (modelo "facelet") -----------------------
 *
 *  O cubo tem 6 faces, cada uma com 9 adesivos (facelets), totalizando 54.
 *  Guardamos os 54 adesivos em um vetor de caracteres (uma string), onde
 *  cada caractere e a cor do adesivo.
 *
 *  A ordem das faces no vetor (exigida pelo enunciado) e:
 *
 *      Indices  0..8   -> Cima      (U - Up)
 *      Indices  9..17  -> Esquerda  (L - Left)
 *      Indices 18..26  -> Frente    (F - Front)
 *      Indices 27..35  -> Direita   (R - Right)
 *      Indices 36..44  -> Tras      (B - Back)
 *      Indices 45..53  -> Baixo     (D - Down)
 *
 *  Dentro de cada face os 9 adesivos sao numerados de 0 a 8 em linhas
 *  (da esquerda para a direita, de cima para baixo), vistos de fora da face:
 *
 *          0 1 2
 *          3 4 5
 *          6 7 8
 *
 *  O adesivo central (indice 4) de cada face NUNCA se move: ele define a
 *  "cor verdadeira" daquela face. O cubo esta resolvido quando todos os 9
 *  adesivos de cada face sao iguais ao seu centro.
 *
 *  Cores possiveis: Y (amarelo), W (branco), G (verde), B (azul),
 *                   R (vermelho), O (laranja).
 * ========================================================================== */

#ifndef CUBE_H
#define CUBE_H

#define NUM_FACELETS 54   /* total de adesivos                                */
#define NUM_MOVES    12   /* U U' D D' L L' R R' F F' B B'                     */

/* --------------------------------------------------------------------------
 *  Cores ANSI para deixar a saida do terminal bonita.
 *  - cubeInitCores() detecta se a saida e um terminal de verdade; se for um
 *    arquivo/redirecionamento, as cores sao desligadas (para nao "sujar").
 *  - ansi(codigo) devolve o codigo de cor, ou "" se as cores estao desligadas.
 * -------------------------------------------------------------------------- */
#define A_RESET "\033[0m"
#define A_BOLD  "\033[1m"
#define A_DIM   "\033[2m"
#define A_CYAN  "\033[96m"
#define A_GREEN "\033[92m"
#define A_YEL   "\033[93m"

void        cubeInitCores(void);
const char *ansi(const char *codigo);

/* Deslocamentos (offset) de cada face dentro do vetor de 54 posicoes.        */
#define U_OFF 0
#define L_OFF 9
#define F_OFF 18
#define R_OFF 27
#define B_OFF 36
#define D_OFF 45

/* --------------------------------------------------------------------------
 *  Estrutura Cube
 *  --------------------------------------------------------------------------
 *  Guarda o estado completo do cubo como uma string de 54 cores + '\0'.
 *  Usar uma string facilita comparar estados (strcmp), copiar (strcpy) e
 *  calcular o hash (controle de estados visitados).
 * -------------------------------------------------------------------------- */
typedef struct {
    char f[NUM_FACELETS + 1];   /* 54 cores + terminador nulo                 */
} Cube;

/* Nomes textuais dos 12 movimentos, na ordem usada internamente.             */
extern const char *MOVE_NAMES[NUM_MOVES];

/* --------------------------------------------------------------------------
 *  Funcoes basicas do cubo (cada uma exigida no enunciado)
 * -------------------------------------------------------------------------- */

/* Inicializa as tabelas de permutacao dos movimentos. Deve ser chamada uma
 * unica vez no inicio do programa, antes de aplicar qualquer movimento.       */
void initMoves(void);

/* initCube: preenche o cubo a partir de uma string de 54 caracteres.
 * Retorna 1 em caso de sucesso, 0 se a string for invalida (tamanho/cores).   */
int initCube(Cube *cube, const char *str);

/* copyCube: copia o estado de 'src' para 'dst'.                               */
void copyCube(Cube *dst, const Cube *src);

/* applyMove: aplica o movimento 'moveIndex' (0..11) sobre 'in', escrevendo o
 * resultado em 'out'. 'in' e 'out' podem ser ponteiros diferentes.            */
void applyMove(Cube *out, const Cube *in, int moveIndex);

/* isSolved: retorna 1 se o cubo esta resolvido, 0 caso contrario.
 * Resolvido = cada face com uma unica cor (igual ao seu centro). Esse teste
 * funciona para qualquer orientacao do cubo, pois compara cada adesivo com o
 * centro da PROPRIA face (e nao com cores fixas pre-definidas).               */
int isSolved(const Cube *cube);

/* isValidColors: verifica se a string tem 54 caracteres, exatamente 9 de cada
 * uma das 6 cores e os 6 centros todos diferentes (uma cor por face).
 * Retorna 1 se valida, 0 caso contrario.                                      */
int isValidColors(const char *str);

/* printCube: imprime o estado do cubo em formato de "planificacao".           */
void printCube(const Cube *cube);

#endif /* CUBE_H */
