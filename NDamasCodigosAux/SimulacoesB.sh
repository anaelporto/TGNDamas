#!/bin/bash

# Define o número de repetições para cada simulação
REPETICOES=100
NUM_THREADS = 4
# Loop externo: Varia o parâmetro 'x' de 8 a 15
for x in {8..15}; do
    
    # Define o nome do arquivo de saída (ex: n8)
    # ARQUIVO_SAIDA="sn${x}t${NUM_THREADS}.txt" # sequencial
    ARQUIVO_SAIDA="pn${x}t${NUM_THREADS}.txt" # paralelo
    
    # Limpa o arquivo de saída antes de iniciar as 100 execuções
    # Isso garante que a saída anterior não seja mantida.
    > "$ARQUIVO_SAIDA"
    
    echo "Iniciando ${REPETICOES} execuções para ${x} (Saída em ${ARQUIVO_SAIDA})..."
    
    # Loop interno: Executa o comando várias vezes
    for i in $(seq 1 $REPETICOES); do
        # O comando é executado e a saída (stdout) é adicionada ao arquivo.
        # ./ndbs "${x}" >> "$ARQUIVO_SAIDA" # sequencial
        ./ndbp "${x}" >> "$ARQUIVO_SAIDA" # paralelo
    done
    
    echo "Concluído para x=${x}. Total de ${REPETICOES} execuções registradas."

done

echo "Todas as simulações foram concluídas."