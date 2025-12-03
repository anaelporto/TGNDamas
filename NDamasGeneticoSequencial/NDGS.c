// Algoritmo Genético
// Tenta resolver o Problema das N-Damas
// Abordagem sequencial que busca uma solução válida (decisão)
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

// Parâmetros de execução dos experimentos
#define N_QUEENS 100 // Tamanho do tabuleiro
#define POP_SIZE 2000 // Tamanho da população
#define MAX_GENERATIONS 10000 // Número máximo de gerações
#define MUTATION_RATE 0.10 // Taxa de mutação
#define TOURNAMENT_SIZE 10 // Tamanho do torneio de aptidão
#define STAGNATION_LIMIT 1000 // Limite de parada após gerações sem evolução

// Estrutura do indivíduo
typedef struct{
    int position[N_QUEENS]; // Posição da dama em uma coluna
    int fitness; // Aptidão associada ao número de conflitos
} Individual;

// Função para gerar valores inteiros aleatórios
int get_random_int(int max){
    return rand() % max; 
}

// Função que troca a posição de dois indivíduos
void swap(int *a, int *b){
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Função que calcula a aptidão de cada indivíduo
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

// Função que avalia a aptidão de uma população
void evaluate_population(Individual population[]){
    for(int i = 0; i < POP_SIZE; i++){
        population[i].fitness = calculate_fitness(population[i].position);
    }
}

// Função que define a configuração inicial do tabuleiro
void initialize_population(Individual population[]){
    for(int i = 0; i < POP_SIZE; i++){
        // Atribuição inicial na diagonal principal
        for(int j = 0; j < N_QUEENS; j++){
            population[i].position[j] = j;
        }

        // Embaralha as posições com o algoritmo de Fisher-Yates
        for(int j = N_QUEENS - 1; j > 0; j--){
            // Posição de troca aleatória
            int k = get_random_int(j + 1);
            swap(&population[i].position[j], &population[i].position[k]);
        }
    }
}

// Função que realiza o torneio de aptidão
Individual tournament_selection(Individual population[]){
    Individual best = population[get_random_int(POP_SIZE)];

    // Escolhe o melhor indivíduo
    for(int i = 1; i < TOURNAMENT_SIZE; i++){
        Individual current = population[get_random_int(POP_SIZE)];
        if(current.fitness < best.fitness){
            best = current;
        }
    }
    return best;
}

// Função para cruzar indivíduos, gerando dois novos indivíduos
void crossover(Individual parent1, Individual parent2, Individual *child1, Individual *child2){
    int i, j, k1, k2;
    int cut_point = get_random_int(N_QUEENS);
    
    for(i = 0; i < cut_point; i++){
        child1->position[i] = parent1.position[i];
        child2->position[i] = parent2.position[i];
    }
    
    k1 = cut_point;
    for(i = 0; i < N_QUEENS; i++){
        int val = parent2.position[i];
        int present = 0;
        
        for(j = 0; j < cut_point; j++){
            if (child1->position[j] == val){
                present = 1;
                break;
            }
        }
        
        if(!present){
            child1->position[k1++] = val;
        }
    }
    
    k2 = cut_point;
    for(i = 0; i < N_QUEENS; i++){
        int val = parent1.position[i];
        int present = 0;
        
        for(j = 0; j < cut_point; j++){
            if(child2->position[j] == val){
                present = 1;
                break;
            }
        }
        
        if(!present){
            child2->position[k2++] = val;
        }
    }
}

// Função que aplica mutação em um indivíduo
void mutate(Individual *individual){
    if((double)rand() / RAND_MAX < MUTATION_RATE){
        int index1 = get_random_int(N_QUEENS);
        int index2 = get_random_int(N_QUEENS);
        
        // Faz uma troca aleatória das posições
        if(index1 != index2){ 
            swap(&individual->position[index1], &individual->position[index2]);
        }

        // Avalia a nova aptidão do indivíduo
        individual->fitness = calculate_fitness(individual->position);
    }
}

// Função que imprime o tabuleiro para fins de validação
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
int main(){
    int generation = 0;
    int stagnation_counter = 0;
    struct timeval tv, start, stop;

    gettimeofday(&tv, NULL);
    
    // Semente aleatória com definição aprimorada
    unsigned long seed = (unsigned long)tv.tv_sec * 1000000 + tv.tv_usec;
    srand((unsigned int)seed); 

    Individual population[POP_SIZE];
    Individual new_population[POP_SIZE];
    Individual best_solution;
    best_solution.fitness = N_QUEENS * N_QUEENS;

    // Inicializa valores e avalia as primeiras populações
    initialize_population(population);
    evaluate_population(population);

    // Obtém o tempo inicial
    gettimeofday(&start, NULL);

    // Loop principal de simulação
    for(generation = 0; generation < MAX_GENERATIONS; generation++){
        // Aplicando elitismo
        Individual current_best = population[0];
        for(int i = 1; i < POP_SIZE; i++){
            if(population[i].fitness < current_best.fitness){
                current_best = population[i];
            }
        }
        
        if(current_best.fitness < best_solution.fitness){
            best_solution = current_best;
            stagnation_counter = 0; // Reseta o contador
        }else{
            stagnation_counter++;
        }

        // Condição de parada: Solução ótima encontrada
        if(best_solution.fitness == 0){
            gettimeofday(&stop, NULL); 
            //printf("\n Solucao na Geracao %d!\n", generation);
            goto end_simulation;
        }
        
        // Condição de parada: Estagnação
        if(stagnation_counter >= STAGNATION_LIMIT){
            gettimeofday(&stop, NULL); 
            //printf("\nPARADA POR ESTAGNACAO na Geracao %d!\n", generation);
            goto end_simulation;
        }

        new_population[0] = best_solution;
        
        // Variabilidade genética
        for(int i = 1; i < POP_SIZE; i += 2){
            Individual parent1 = tournament_selection(population);
            Individual parent2 = tournament_selection(population);

            Individual child1, child2;

            crossover(parent1, parent2, &child1, &child2);

            mutate(&child1);
            if(i + 1 < POP_SIZE){
                mutate(&child2);
            }

            new_population[i] = child1;
            if(i + 1 < POP_SIZE){
                new_population[i+1] = child2;  
            } 
        }
        
        // Substitui a população antiga pela nova, reavaliando-a
        for(int i = 0; i < POP_SIZE; i++){
            population[i] = new_population[i];
            population[i].fitness = calculate_fitness(population[i].position); 
        }
    }
    // goto para unificar critérios de parada
    end_simulation:;

    // Obtém o tempo final
    gettimeofday(&stop, NULL); 

    // Cálculo do tempo gasto pelo processo
    double t = (double)(stop.tv_sec - start.tv_sec) * 1000.0 + (double)(stop.tv_usec - start.tv_usec) / 1000.0;
    
    // Confirma se houve solução encontrada ou não e imprime junto do tempo decorrido
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