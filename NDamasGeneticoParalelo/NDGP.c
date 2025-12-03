// Algoritmo Genético
// Tenta resolver o Problema das N-Damas
// Abordagem otimizada com paralelismo que busca uma solução válida (decisão)
// Trechos revisados para melhorar funcionamento e consistência
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h> // Biblioteca do openmp

// Parâmetros de execução dos experimentos
#define N_QUEENS 30 // Tamanho do tabuleiro
#define POP_SIZE 2000 // Tamanho da população
#define MAX_GENERATIONS 10000 // Número máximo de gerações
#define MUTATION_RATE 0.10 // Taxa de mutação
#define TOURNAMENT_SIZE 10 // Tamanho do torneio de aptidão
#define STAGNATION_LIMIT 1000 // Limite de parada após gerações sem evolução
#define N_THREADS 4 // Número de threads operando durante a execução

// Estrutura do indivíduo
typedef struct {
    int position[N_QUEENS]; // Posição final do indivíduo em uma coluna
    int fitness; // Aptidão associada ao número de conflitos
} Individual;

// Função para gerar um valor inteiro aleatório seguro para threads
int get_random_int_r(int max, unsigned int *seed) {
    return (int)(rand_r(seed) % max);
}

// Função para gerar um valor de ponto flutuante aleatório seguro para threads
double get_random_double_r(unsigned int *seed) {
    return (double)rand_r(seed) / (double)RAND_MAX;
}

// Função que troca a posição de dois indivíduos
void swap(int *a, int *b) {
    int temp = *a; 
    *a = *b; 
    *b = temp;
}

// Função que calcula a aptidão de cada indivíduo
// Valores locais seguros para threads
int calculate_fitness(int *positions){
    int d1_counts[2 * N_QUEENS - 1] = {0};
    int d2_counts[2 * N_QUEENS - 1] = {0};

    for(int i = 0; i < N_QUEENS; i++){
        d1_counts[i - positions[i] + (N_QUEENS - 1)]++;
        d2_counts[i + positions[i]]++;                  
    }

    int conflicts = 0;
    for(int i = 0; i < 2 * N_QUEENS - 1; i++){
        if(d1_counts[i] > 1){
            conflicts += d1_counts[i] - 1;
        }
        if(d2_counts[i] > 1){
            conflicts += d2_counts[i] - 1;
        }
    }
    return conflicts;
}

// Função que define a configuração inicial do tabuleiro
// Distribui as atribuições e as trocas entre as threads
void initialize_population_parallel(Individual population[], unsigned int base_seed){
    #pragma omp parallel num_threads(N_THREADS)
    {
        // Define uma semente para cada thread
        int thread_id = omp_get_thread_num();
        unsigned int seed = base_seed + thread_id;

        #pragma omp for schedule(static)
        for(int i = 0; i < POP_SIZE; i++){
            // Atribuição inicial na diagonal principal
            for(int j = 0; j < N_QUEENS; j++){
                population[i].position[j] = j;
            }

            // Embaralha as posições com o algoritmo de Fisher-Yates
            for(int j = N_QUEENS - 1; j > 0; j--){
                // Posição de troca aleatória segura para threads
                int k = get_random_int_r(j + 1, &seed);
                swap(&population[i].position[j], &population[i].position[k]);
            }
            // Avalia a aptidão da primeira geração
            population[i].fitness = calculate_fitness(population[i].position);
        }
    }
}

// Função que realiza o torneio de aptidão
// Chamada dentro de trecho paralelo seguro
Individual tournament_selection_parallel(const Individual population[], unsigned int *seed){
    Individual best = population[get_random_int_r(POP_SIZE, seed)];

    // Escolhe o melhor indivíduo
    for(int i = 1; i < TOURNAMENT_SIZE; i++){
        Individual current = population[get_random_int_r(POP_SIZE, seed)];
        if(current.fitness < best.fitness){
            best = current;
        }
    }
    return best;
}

// Função para cruzar indivíduos, gerando dois novos indivíduos
// Chamada dentro de trecho paralelo seguro
void crossover_parallel(const Individual *parent1, const Individual *parent2, 
                        Individual *child1, Individual *child2, unsigned int *seed){
    int cut = get_random_int_r(N_QUEENS, seed);
    int k1 = cut, k2 = cut;

    for(int i = 0; i < cut; i++){
        child1->position[i] = parent1->position[i];
        child2->position[i] = parent2->position[i];
    }

    for(int i = 0; i < N_QUEENS; i++) {
        int val = parent2->position[i], present = 0;
        for(int j = 0; j < cut; j++){
            if(child1->position[j] == val){
                 present = 1; break;
            }
        } 

        if(!present){
            child1->position[k1++] = val;
        }
    }
    for(int i = 0; i < N_QUEENS; i++){
        int val = parent1->position[i], present = 0;
        
        for(int j = 0; j < cut; j++){
            if(child2->position[j] == val){
                 present = 1; break; 
            }
        }

        if(!present){
            child2->position[k2++] = val;
        } 
    }
}

// Função que aplica mutação em um indivíduo
// Chamada dentro de trecho paralelo seguro
void mutate_parallel(Individual *individual, unsigned int *seed){
    if(get_random_double_r(seed) < MUTATION_RATE){
        int index1 = get_random_int_r(N_QUEENS, seed);
        int index2 = get_random_int_r(N_QUEENS, seed);
        
        // Faz uma troca aleatória das posições
        if(index1 != index2){
            swap(&individual->position[index1], &individual->position[index2]);
        }
        
        // Avalia a nova aptidão do indivíduo
        individual->fitness = calculate_fitness(individual->position);
    }
}

// Função que imprime o tabuleiro para fins de validação
// Fora do loop paralelo de interesse
void print_solution(Individual solution){
    printf("\nSolucao encontrada para N=%d)\n", N_QUEENS);
    printf("Aptidão: %d\n", solution.fitness);

    if(N_QUEENS <= 50){    
        // Impressão em formato de matriz (0 = vazio e 1 = dama)
        for(int i = 0; i < N_QUEENS; i++){
            for(int j = 0; j < N_QUEENS; j++){
                printf("%d ", solution.position[i] == j ? 1 : 0);
            }
            printf("\n");
        }
    }
    else{    
        // Impressão em formato de lista (posições)
        for(int i = 0; i < N_QUEENS; i++){
            printf("(%d, %d) ", i, solution.position[i]);
        }
        printf("\n");
    }
}

// Função que gerencia o processamento principal
// Trecho paralelo de interesse
int main(){
    int generation = 0;
    int stagnation_counter = 0;
    struct timeval tv, start, stop;

    gettimeofday(&tv, NULL);

    // Semente aleatória com definição aprimorada
    unsigned long base_seed = (unsigned long)tv.tv_sec * 1000000 + tv.tv_usec;

    Individual population[POP_SIZE];
    Individual new_population[POP_SIZE];
    Individual best_solution;
    best_solution.fitness = N_QUEENS * N_QUEENS;

    // Inicializa valores das primeiras populações
    initialize_population_parallel(population, base_seed);

    // Obtém o tempo inicial
    gettimeofday(&start, NULL);

    // Loop principal de simulação
    for(generation = 0; generation < MAX_GENERATIONS; generation++){
        // Aplicando elitismo
        Individual current_best;
        current_best.fitness = N_QUEENS * N_QUEENS;

        // Primeiro trecho paralelo seguro
        #pragma omp parallel num_threads(N_THREADS)
        {
            Individual local_best;
            local_best.fitness = N_QUEENS * N_QUEENS;

            // Distribui as comparações entre as threads
            #pragma omp for nowait schedule(static)
            for(int i = 0; i < POP_SIZE; i++){
                if(population[i].fitness < local_best.fitness){
                    local_best = population[i];
                }
            }

            // Ponto crítico de sincronização
            #pragma omp critical
            {
                if(local_best.fitness < current_best.fitness){
                    current_best = local_best;
                }
            }
        }

        // Verificação de estagnação
        if(current_best.fitness < best_solution.fitness){ 
            best_solution = current_best; 
            stagnation_counter = 0;
        }
        else{
            stagnation_counter++;
        }

        // Encerra se encontrar solução ou atingir estagnação
        if(best_solution.fitness == 0 || stagnation_counter >= STAGNATION_LIMIT){
            gettimeofday(&stop, NULL);
            break;
        }

        // Segue a busca por solução
        new_population[0] = best_solution;
        
        // Segundo trecho paralelo seguro
        #pragma omp parallel num_threads(N_THREADS)
        {
            int tid = omp_get_thread_num();
            // Define uma semente para cada thread
            unsigned int seed = base_seed + generation * POP_SIZE + tid;

            // Processo de variabilidade genética
            #pragma omp for schedule(static)
            for(int i = 1; i < POP_SIZE; i += 2){
                // Escolhe dois "bons" indivíduos
                Individual parent1 = tournament_selection_parallel(population, &seed);
                Individual parent2 = tournament_selection_parallel(population, &seed);

                Individual child1, child2;

                // Cruzamento entre os dois indivíduos escolhidos
                crossover_parallel(&parent1, &parent2, &child1, &child2, &seed);

                // Aplica mutação no primeiro filho gerado pelo cruzamento
                mutate_parallel(&child1, &seed);

                new_population[i] = child1;

                // Aplica mutação no segundo filho gerado pelo cruzamento
                // Preenche a nova população e reinicia o processo
                if(i + 1 < POP_SIZE){
                    mutate_parallel(&child2, &seed);
                    new_population[i + 1] = child2;
                }
            }
        }

        // Substitui a população antiga pela nova, reavaliando-a
        // Distribui as atribuições entre as threads
        #pragma omp parallel for schedule(static) num_threads(N_THREADS)
        for(int i = 0; i < POP_SIZE; i++){
            population[i] = new_population[i];
        }
    }
    // Cálculo do tempo gasto pelo processo
    double t = (double)(stop.tv_sec - start.tv_sec) * 1000.0 + (double)(stop.tv_usec - start.tv_usec) / 1000.0;

    // Confirma se houve solução encontrada ou não
    if(best_solution.fitness == 0){
        printf("Solucao na Geracao %d!\n", generation);
        fprintf(stdout, "Tempo decorrido = %g ms\n", t);
    }else{
        printf("Não foi possível encontrar solução otima. Melhor fitness: %d\n", best_solution.fitness);
    }

    // Imprime o tabuleiro para confirmação visual
    //print_solution(best_solution);

    return 0;
}
