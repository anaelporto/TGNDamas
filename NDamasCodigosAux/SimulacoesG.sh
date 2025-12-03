#!/bin/bash

# Define o número de repetições para cada simulação
REPETICOES=30
n=100
p=2000
t=4

# ARQUIVO_SAIDA="sn${n}p${p}t${t}.txt" # sequencial
ARQUIVO_SAIDA="pn${n}p${p}t${t}.txt" # paralelo

# Limpa o arquivo de saída antes de iniciar as 100 execuções
# Isso garante que a saída anterior não seja mantida.
> "$ARQUIVO_SAIDA"

echo "${REPETICOES} execuções para = ${n} POP = ${p} THREADS = ${t}"

# Loop interno: Executa o comando várias vezes
for i in $(seq 1 $REPETICOES); do
    # O comando é executado e a saída (stdout) é adicionada ao arquivo.
    # ./ndgs >> "$ARQUIVO_SAIDA" # sequencial
    ./ndgp >> "$ARQUIVO_SAIDA" # paralelo
done

echo "Concluído para x=${n}. Total de ${REPETICOES} execuções registradas."
