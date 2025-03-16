#include <stdio.h>
#include <stdlib.h>
#include "rainhas.h"

// Backtracking ----------------------------------------------------------------------------

// Declarações das funções auxiliares ------------------------------------------------------
int permitido(unsigned int lin, unsigned int col, unsigned int k, casa *c, unsigned int *r);
int backtrackingRainhas(unsigned int lin, unsigned int n, unsigned int k, casa *c, unsigned int *r);
// -----------------------------------------------------------------------------------------

int permitido(unsigned int lin, unsigned int col, unsigned int k, casa *c, unsigned int *r)
{
  unsigned int i;
  for (i = 0; i < lin; i++)
  {
    // verifica se a coluna ou a diagonal é a mesma de alguma resposta anterior
    if (r[i] == col || ((col < r[i]) ? (r[i] - col) : (col - r[i])) == (lin - i))
      return 0;
  }
  for (i = 0; i < k; i++) 
  {
    // verifica se é uma casa proibida
    if (c[i].linha == lin+1 && c[i].coluna == col)
      return 0;
  }

  return 1;
}

int backtrackingRainhas(unsigned int lin, unsigned int n, unsigned int k, casa *c, unsigned int *r)
{
  // base da recursividade
  if (lin == n)
    return 1;

  // verifica se há uma coluna onde é permitido posicionar uma rainha
  for (unsigned int col = 1; col <= n; col++)
  {
    if (permitido(lin, col, k, c, r))
    {
      // caso encontre, guarda no vetor resposta e chama recursivamente com lin+1
      r[lin] = col;

      if (backtrackingRainhas(lin+1, n, k, c, r))
        return 1;

      // caso retorne sem sucesso da chamada recursiva, zera o resultado e continua procurando
      r[lin] = 0;
    }
  }

  // retorno que indica fracasso em encontrar posição válida
  return 0;
}

unsigned int *rainhas_bt(unsigned int n, unsigned int k, casa *c, unsigned int *r)
{
  for (unsigned int i = 0; i < n; i++)
    r[i] = 0;
  
  if (backtrackingRainhas(0, n, k, c, r))
    return r;

  return NULL;
}

/******************************************************************************************/

// Conjunto Independente -------------------------------------------------------------------

// Declarações das funções auxiliares ------------------------------------------------------
int **criaGrafo(unsigned int n);
void adicionaAresta(int **grafo, unsigned int vertice1, unsigned int vertice2);
void criaArestasGrafo(unsigned int n, int **grafo);
unsigned int *conjuntoIndependenteRainhas(unsigned int n, int **grafo, unsigned int *I, unsigned int I_tam, unsigned int *C, unsigned int C_tam, unsigned int *r);
// -----------------------------------------------------------------------------------------

int **criaGrafo(unsigned int n)
{
  int **grafo; 
  grafo = (int**)calloc(n*n, sizeof(int*));
  for (unsigned int i = 0; i < n*n; i++)
    grafo[i] = (int*)calloc(n*n, sizeof(int));

  return grafo;
}

void adicionaAresta(int **grafo, unsigned int vertice1, unsigned int vertice2)
{
  grafo[vertice1][vertice2] = grafo[vertice2][vertice1] = 1;
}

void criaArestasGrafo(unsigned int n, int **grafo)
{
  for (unsigned int i = 0; i < n; i++){
    for (unsigned int j = 0; j < n; j++){
      // Para cada posição (i,j), percorre as demais posições da matriz
      for (unsigned int k = 0; k < n; k++){
        for (unsigned int l = 0; l < n; l++){
          // verifica se a posição (i,j) é diferente da posição (k,l)
          if (i != k || j != l){
            // caso as posições (i,j) e (k,l) se ataquem, adiciona uma aresta
            if (i == k || j == l || ((i < k) ? (k - i) : (i - k)) == ((j < l) ? (l - j) : (j - l)))
              adicionaAresta(grafo, i*n + j, k*n + l);
          }
        }
      }
    }
  }
}

unsigned int *conjuntoIndependenteRainhas(unsigned int n, int **grafo, unsigned int *I, unsigned int I_tam, unsigned int *C, unsigned int C_tam, unsigned int *r)
{
  unsigned int i;

  // base da recursividade
  if (I_tam == n)
  {
    div_t vertice;
    // Passa os valores presentes no conjunto independente para o vetor resposta
    // vertice.quot = linha, vertice.rem = col-1
    for (i = 0; i < n; i++){
      vertice = div((int)I[n-i-1], (int)n);
      r[vertice.quot] = (unsigned int)vertice.rem + 1;
    }

    return r;
  }

  // se o tamI + tamC for menor que n, não haverá resposta por esse caminho
  if (I_tam + C_tam < n)
    return NULL;

  // escolhe um vértice do conjunto C
  unsigned int verticeEscolhido = C[C_tam - 1];
  C_tam--;

  // cria um novo conjunto independente contendo o novo vértice
  unsigned int *novoI;
  novoI = (unsigned int*)calloc(n, sizeof(unsigned int));
  for (i = 0; i < I_tam; i++)
    novoI[i] = I[i];
  novoI[I_tam] = verticeEscolhido;
  unsigned int novoI_tam = I_tam + 1;

  // cria um novo conjunto de possíveis vértices desconsiderando os que atacam o vértice escolhido
  unsigned int *novoC;
  novoC = (unsigned int*)calloc(n*n, sizeof(unsigned int));
  unsigned int novoC_tam = 0;
  for (i = 0; i < C_tam; i++){
    if (grafo[verticeEscolhido][C[i]] == 0){
      novoC[novoC_tam] = C[i];
      novoC_tam++;
    }
  }

  // chama recursivamente com os novos conjuntos I e C
  unsigned int *result = conjuntoIndependenteRainhas(n, grafo, novoI, novoI_tam, novoC, novoC_tam, r);

  free(novoI);
  free(novoC);

  if (result != NULL)
    return result;

  // caso tenha retornado sem sucesso, chama recursivamente com os mesmos conjuntos I e C, porém com o tamanho de C decrescido em 1
  return conjuntoIndependenteRainhas(n, grafo, I, I_tam, C, C_tam, r);
}

unsigned int *rainhas_ci(unsigned int n, unsigned int k, casa *c, unsigned int *r)
{
  // cria o grafo como uma matriz de adjacência
  int **grafo;
  grafo = criaGrafo(n);
  criaArestasGrafo(n, grafo);

  // Conjunto Independente
  unsigned int *I;
  I = (unsigned int*)calloc(n, sizeof(unsigned int));
  unsigned int I_tam = 0;

  // Conjunto dos possíveis vértices a serem escolhidos
  unsigned int *C;
  C = (unsigned int*)calloc(n*n, sizeof(unsigned int));
  unsigned int C_tam = 0;

  int proibido;

  for (unsigned int i = 0; i < n; i++){
    for (unsigned int j = 0; j < n; j++){
      proibido = 0;
      // para cada posição (i,j) verifica se é uma casa proibida
      for (unsigned int ind = 0; ind < k; ind++){
        if (i+1 == c[ind].linha && j+1 == c[ind].coluna){
          proibido = 1;
          break;
        }
      }
      // se a posição é válida, coloca no conjunto dos possíveis vértices
      if (!proibido){
        C[C_tam] = (i*n + j);
        C_tam++;
      }
    }
  }

  r = conjuntoIndependenteRainhas(n, grafo, I, I_tam, C, C_tam, r);

  for (unsigned int i = 0; i < n*n; i++)
    free(grafo[i]);

  free(grafo);
  free(I);
  free(C);

  return r;
}
