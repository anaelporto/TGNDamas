#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

// Número de Threads (2 ou 4)
#define N_THREADS 4

// Variáveis globais
static int nSolutions = 0;    // Contador para o total de soluções
static int TamTabuleiro;      // Tamanho do tabuleiro

// Função que imprime uma solução encontrada
void printSolution(int *board){
    printf("Solução %d: ", nSolutions);
    for(int col = 0; col < TamTabuleiro; col++){
        printf("(%d, %d) ", board[col], col);
    }
    printf("\n");
}

// Confere se a posição da dama é válida (1) ou inválida (0) antes de colocá-la
int isSafe(int *board, int row, int col){
    int i, j;

    // Confere se existem conflitos na fileira da esquerda
    for(i = 0; i < col; i++){
        if(board[i] == row){
            return 0;
        }
    }
 
    // Confere se existem conflitos na diagonal superior esquerda
    for(i = row-1, j = col-1; i >= 0 && j >= 0; i--, j--){
        if(board[j] == i){
            return 0;
        }
    }
 
    // Confere se existem conflitos na diagonal inferior esquerda
    for(i = row+1, j = col-1; j >= 0 && i < TamTabuleiro; i++, j--){
        if(board[j] == i){
            return 0;
        }
    }
 
    return 1;
}

// Percorre o tabuleiro colocando as damas em posições válidas
void solveNQ(int *board, int col){
    // Lê os valores da coluna atual até o final do tabuleiro
    for(int i = 0; i < TamTabuleiro; i++){
        // Confere se uma posição é válida
        if(isSafe(board, i, col)){
            // Atribui a posição da dama no tabuleiro
            board[col] = i;
      
            // Solução encontrada para a coluna atual
            if(col == TamTabuleiro-1){ 
                #pragma omp critical
                {
                    // Exibe as coordenadas da solução encontrada
                    //printSolution(board);
                }
                
                // Contabiliza a solução
                #pragma omp atomic
                nSolutions++;
            }
            else{ 
                // Copia o tabuleiro atual para um temporário
                int *nb = (int *)malloc(TamTabuleiro*sizeof(int));
                memcpy(nb, board, TamTabuleiro*sizeof(int));
                
                // Segue para a próxima coluna
                #pragma omp task
                {
                    solveNQ(nb, col + 1);

                    // Libera o tabuleiro temporário
                    free(nb);
                }
            }
        }
    }
}
 
// Função principal que recebe N de entrada e resolve o problema medindo o tempo de cada execução
int main(int argc, char *argv[]){
    int i, *board;
    struct timeval start, stop;

    // Verifica se o valor de N foi incluído na linha de comando
    if(argc <=1){
        fprintf(stdout, "É necessário especificar o tamanho do tabuleiro\n");
        exit(-1);
    }

    // Recebe o tamnho do tabuleiro com base na entrada (argv[1])
    TamTabuleiro = strtol(argv[1], NULL, 10);

    // Alocação dinâmica do tabuleiro
    board = (int *)malloc(TamTabuleiro*sizeof(int));

    // Obtém o tempo inicial
    gettimeofday(&start, NULL);
 
    // Resolve o problema das N-Damas coluna por coluna
    #pragma omp parallel num_threads(N_THREADS)
    {
        #pragma omp single
        {
            // Resolve o problema das N-Damas percorrendo todas as colunas
            solveNQ(board,0);
        }
    }
  
    // Obtém o tempo final
    gettimeofday(&stop, NULL); 

    // Cálculo do tempo gasto pelo processo
    double t = (double)(stop.tv_sec - start.tv_sec) * 1000.0 + (double)(stop.tv_usec - start.tv_usec) / 1000.0;

    // Exibe o total de soluções e o tempo decorrido
    fprintf(stdout, "Número total de soluções: %d\n", nSolutions); 
    fprintf(stdout, "Tempo decorrido = %g ms\n", t);

    // Libera o tabuleiro
    free(board);

    return 0;
}
