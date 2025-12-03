#include <stdio.h>
#include <stdlib.h> 

#define TAMANHO_LINHA 35
#define TAMANHO_NOME_ARQUIVO 11
#define POSICAO_DADO 18
#define NUM_THREADS 4

int main() {
    int i;
    
    // Variáveis locais para cada arquivo
    double local_sum;
    int count; 
    double average;
    
    char filename[TAMANHO_NOME_ARQUIVO];
    char line[TAMANHO_LINHA];
    FILE *fp_input;
    FILE *fp_output;

    // Abre o arquivo de saída: "medias.txt" (modo "w" para criar/sobrescrever)
    fp_output = fopen("medias.txt", "w");
    
    if (fp_output == NULL) {
        perror("Erro ao abrir o arquivo de saída 'medias.txt'");
        return 1;
    }

    // Loop principal para abrir os arquivos
    for (i = 8; i < 16; i++) {
        // Zera as variáveis para o NOVO arquivo
        local_sum = 0.0;
        count = 0;
        
        // Gera o nome do arquivo
        //snprintf(filename, sizeof(filename), "sn%dt%d.txt", i, NUM_THREADS); // sequencial
        snprintf(filename, sizeof(filename), "pn%dt%d.txt", i, NUM_THREADS); // paralelo

        if ((fp_input = fopen(filename, "r")) != NULL) {
            
            // Loop para ler linha por linha do arquivo de ENTRADA
            while (fgets(line, sizeof(line), fp_input) != NULL) {
                
                // Verifica se a linha começa com 'T'
                if (line[0] == 'T') {
                    double valor_lido;
                    
                    // Extração segura usando sscanf a partir da posição definida
                    if (sscanf(line + POSICAO_DADO, "%lf", &valor_lido) == 1) { 
                        local_sum += valor_lido;
                        count++;
                    }
                }
            }
            
            fclose(fp_input);

            // 2. Calcula e armazena a média do arquivo
            if (count > 0) {
                average = local_sum / count;
                
                // 3. Escreve a MÉDIA no arquivo de SAÍDA (medias.txt)
                fprintf(fp_output, "Média do arquivo %d: %.4lf\n", i, average);
                
            } else {
                // Se nenhum valor foi encontrado
                fprintf(fp_output, "Média do arquivo %d: Nenhum valor 'T' encontrado.\n", i);
            }
            
        } else {
            // Se o arquivo de ENTRADA não puder ser aberto
            fprintf(fp_output, "Média do arquivo %d: Erro ao abrir.\n", i);
        }
    }
    
    // Fecha o arquivo de saída após o loop
    fclose(fp_output);

    printf("Processamento concluído. O resultado das médias está no arquivo 'medias.txt'.\n");
    
    return 0;
}