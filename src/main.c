/**
 * @file main.c
 * @brief Arquivo principal para execução da regressão logistica.
 * 
 * Esse arquivo contém os principais métodos para execução do algoritmo
 * de regressão logistica. A execução é iniciada pela função main(), que
 * realiza a chamada a todas as funções auxiliares (internas ou externas
 * ao arquivo) necessárias.
 * 
 * @author Nadine Cerqueira Marques (nadymarkes@gmail.com)
 * @author Valmir Vinicius de Almeida Santos (vvalmeida96@gmail.com)
 * @version 0.1
 * @date 12/10/2018
 * 
 * @copyright Copyright (c) 2018
 * 
 */

/* -- Includes -- */

/** Inclusão da biblioteca stdio **/
#include <stdio.h>

/** Inclusão da biblioteca stdlib **/
#include <stdlib.h>

/** Inclusão da biblioteca string **/
#include <string.h>

/** Inclusão da biblioteca errno **/
#include <errno.h>

/** Inclusão da biblioteca math **/
#include <math.h>

/** Inclusão do arquivo de cabeçalho responsável pela leitura do arquivo de entrada **/
#include "csv.h"


/**
 * 
 * @brief Constante definindo o número de imagens para treinamento.
 * 
 * Constante definindo o número total de imagens para treinamento e mais
 * uma entrada para o bias.
 * 
 */
static const int NUM_IMAGES_TRAINING = 10049 + 1;

/**
 * @brief Constante definindo o número de imagens para teste.
 * 
 */
static const int NUM_IMAGES_TESTING = 1149;

/**
 * @brief Constante definindo o número de pixels.
 * 
 */
static const int NUM_PIXELS = 2304;


/**
 * @brief Realiza alocação dinâmica para uma string.
 * 
 * Realiza a alocação dinâmica para um espaço de memória de acordo com
 * a string informada.
 * 
 * @param s string para a qual se deseja alocar um espaço de memória
 * @return void* ponteiro para o endereço de memória alocado
 */
char *estrdup (char *s) {
	char *t;
	t = (char *) malloc(strlen(s)+1);
	strcpy(t, s);
	return t;
}

/**
 * @brief Gera números reais aleatórios.
 * 
 * Gera números reais aleatórios de acordo com o intervalo entre
 * mínimo e máximo informado.
 * 
 * @param min valor mínimo
 * @param max valor máximo
 * @return double valor aleatório gerado 
 */
double rand_range(double min, double max) {
    return min + ((double)(max - min) * (rand() / (RAND_MAX + 1.0)));
}

/**
 * @brief Inicializa o vetor de pesos.
 * 
 * Inicializa o vetor de pesos com valores aleatórios gerados.
 * 
 * @param row referência para o vetor de pesos
 */
void initialize_weights(double *row) {
    for(int i=0; i < NUM_PIXELS; i++) {
        *(row+i) = rand_range(-1, 1) / (NUM_PIXELS + NUM_IMAGES_TRAINING);
    }
}

/**
 * @brief Realiza a leitura completa do arquivo .csv de entrada.
 * 
 * Realiza a leitura completa do arquivo .csv de entrada e armazena
 * os pixels lidos nas matrizes para dados de teste e treinamento. Os
 * labels lidos são armazenados nos vetores de treinamento e de teste.
 * 
 * @param file_input ponteiro para leitura de arquivo
 * @param data_testing referência para matriz com dados para teste
 * @param data_training referência para matriz com dados para treinamento
 * @param labels_testing referência para vetor com labels para teste
 * @param labels_training referência para vetor com labels para treinamento
 */
void read_data_and_labels(FILE *file_input, double **data_testing, double **data_training, int *labels_testing, int *labels_training) {
    char *line; //linha lida do arquivo
    int row_testing = 0; //contador para linhas da matriz com dados para teste
    int row_training = 1; //contador para linhas da matriz com dados para treinamento
    char *label, *pixels, *usage, *pch; 
    double temp; 

    /** aloca espaço para o bias e adiciona o bias ao dataset de treinamento **/
    data_training[0] = (double *) malloc(NUM_PIXELS * sizeof(double));
    for(int i = 0; i < NUM_PIXELS; i++) {
        data_training[0][i] = 0;
    } 
    labels_training[0] = 1;

    /* itera o arquivo até o final, ou seja, até a linha obtida ser NULL */
    while(((line = csvgetline(file_input, ',', 0)) != NULL)) {
        label = estrdup(csvfield(0)); //obtém o primeiro campo de uma linha (label)

        if((strcmp(label, "2") == 0 || strcmp(label, "4") == 0)) { //verifica se é algum dos labels de interesse
            pixels = estrdup(csvfield(1)); //obtém o segundo campo de uma linha (pixels)
            usage = estrdup(csvfield(2)); //obtém o terceiro campo de uma linha (usage)
            pch = strtok(pixels, " "); //separa os pixels por espaços

            if(strcmp(usage, "PublicTest") == 0) { //verifica se a linha possui dados para teste
                data_testing[row_testing] = (double *) malloc(NUM_PIXELS * sizeof(double));
                labels_testing[row_testing] = atoi(label)/4; //converte a label para inteiro e divide por 4, tornando-a 1 ou 0
            } else {
                data_training[row_training] = (double *) malloc(NUM_PIXELS * sizeof(double));
                labels_training[row_training] = atoi(label)/4;
            }

            /** realiza iteração para cada pixel que será lido **/
            for(int c = 0; c < NUM_PIXELS; c++) {
                temp = strtof(pch, NULL); //obtém 1 pixel
                temp = temp/255; //realiza normalização no pixel

                if(strcmp(usage, "PublicTest") == 0) { 
                    data_testing[row_testing][c] = temp;
                } else {
                    data_training[row_training][c] = temp;
                }

                pch = strtok(NULL, " ");
            }

            if(strcmp(usage, "PublicTest") == 0) {
                row_testing++; //atualiza a contagem das linhas de teste
            } else {
                row_training++; //atualiza a contagem das linhas de treinamento
            }
        }
    }
}

/**
 * @brief Salva os resultados em arquivo
 * 
 * @param epoch_num número da epoca
 * @param results vetor contendo resultados da regressão logistica
 * @param labels labels lidas do arquivo (valores corretos)
 * @param num_images número de imagens que foram processadas
 */
void save_results(int epoch_num, int *results, int *labels, int num_images) {
    int true_positive = 0, true_negative = 0, false_positive = 0, false_negative = 0;
    double accuracy = 0, precision = 0, recall = 0, f1 = 0;
    int **confusion_matrix = (int **) malloc(2 * sizeof(int *));
    confusion_matrix[0] = (int *) malloc(2 * sizeof(int));
    confusion_matrix[1] = (int *) malloc(2 * sizeof(int));

    /** computa os verdadeiros positivos e negativos e os falsos positivos e negativos **/
    for(int i=0; i < num_images; i++) {
        if(results[i] == 1 && labels[i] == 1) {
            true_positive++;
        } else if(results[i] == 1 && labels[i] == 0) {
            false_positive++;
        } else if(results[i] == 0 && labels[i] == 1) {
            false_negative++;
        } else {
            true_negative++;
        }
    }

    /** atualiza a matriz de confusão **/
    confusion_matrix[0][0] = true_negative;
    confusion_matrix[0][1] = false_positive;
    confusion_matrix[1][0] = false_negative;
    confusion_matrix[1][1] =  true_positive;

    accuracy = (double) (true_positive + true_negative) / (true_positive + true_negative + false_positive + false_negative);
    precision = (double) (true_positive) / (true_positive + false_positive); 
    recall = (double) true_positive/(true_positive + false_negative);
    f1 = 2 * ((precision * recall) / (precision + recall));

    printf("\nAcurácia: %lf     Precisão: %lf      Recall: %lf      F1: %lf\n", accuracy, precision, recall, f1);
}

/**
 * @brief Realiza o cálculo da função hipótese.
 * 
 * Realiza o cálculo da função hipótese de acordo com uma
 * linha da matriz e com o vetor de pesos informado.
 * 
 * @param row linha da matriz
 * @param weights vetor de pesos
 * @return double resultado da função hipotese
 */
double hypothesis_function(double *row, double *weights) {
    double result = 0;

    for(int c=0; c < NUM_PIXELS; c++) {
        result += weights[c] * row[c];
    }

    return 1/(1 + exp(-result)); //aplica a função sigmoid e retorna o resultado
}

/**
 * @brief Realiza o cálculo do gradiente descendente.
 * 
 * Realiza o cálculo do gradiente descendente de acordo com o dataset de treinamento
 * informado, labels de treinamento e valores de hipótese.
 * 
 * @param data_training dataset de treinamento
 * @param labels labels de treinamento
 * @param all_hypothesis valores calculados para hipótese
 * @param c valor atual da coluna
 * @param learning_rate taxa de aprendizado
 * @return double valor de gradiente resultante
 */
double gradient(double **data_training, int *labels, double *all_hypothesis, int c, double learning_rate) {
    double gradient_sum = 0;

    for(int r = 0; r < NUM_IMAGES_TRAINING; r++) {
        gradient_sum += (all_hypothesis[r] - labels[r]) * data_training[r][c];
    }

    return gradient_sum * (learning_rate);
}

/**
 * @brief Realiza a atualização dos pesos.
 * 
 * Realiza a atualização do vetor de pesos após a
 * execução de uma época de treinamento.
 * 
 * @param data_training dataset de treinamento
 * @param weights vetor de pesos
 * @param all_hypothesis valores calculados para hipótese
 * @param labels labels de treinamento
 * @param learning_rate taxa de aprendizado
 */
void update_weights(double **data_training, double *weights, double *all_hypothesis, int *labels, double learning_rate) {

    for(int c=0; c < NUM_PIXELS; c++) {
        weights[c] = weights[c] - (gradient(data_training, labels, all_hypothesis, c, learning_rate)/NUM_IMAGES_TRAINING);
    }

}

/**
 * @brief Realiza o cálculo da função de custo.
 * 
 * Realiza o cálculo da função de custo.
 * 
 * @param all_hypothesis vetor com valores calculados para hipótese
 * @param weights vetor de pesos
 * @param labels vetor de labels
 * @param num_images número de imagens
 * @return double custo resultante
 */
double cost_function(double *all_hypothesis, double *weights, int *labels, int num_images) {
    double cost = 0;

    for(int r=0; r < num_images; r++) {
        cost += -(labels[r] - log(all_hypothesis[r])) - (1 - labels[r]) * log(1 - all_hypothesis[r]);
    }

    return cost / NUM_IMAGES_TRAINING;
}

/**
 * @brief Função principal, na qual é iniciada a execução do algoritmo.
 * 
 * Função principal, na qual é iniciada a execução do algoritmo de acordo
 * com os argumntos informados no terminal para número de épocas e taxa
 * de aprendizagem.
 * 
 * @param argc quantidade de argumentos
 * @param argv vetor contendo os argumentos número de épocas e taxa de aprendizado
 * @return int 0, se a execução foi finalizada sem erros; -1, caso contrário
 */
int main(int argc, char *argv[]) {

    /** obtém os argumentos, converte para int ou float e inicializa o número de 
     * épocas e a taxa de aprendizado **/
    int num_max_epochs = atoi(argv[1]);
    double learning_rate = atof(argv[2]);

    /* vetor de pesos */
    double *weights = (double *) malloc(NUM_PIXELS * sizeof(double));
    /* número de épocas */
    int num_epochs = 0;

    int corretos = 0;
    
    /* vetor contendo todos os valores de hipóteses calculados na épca */
    double *all_hypothesis = (double *) malloc(NUM_IMAGES_TRAINING * sizeof(double));

    /* vetor contendo os resultados, ou seja, os valores de hipótese binarizados */
    int *results = (int *) malloc(NUM_IMAGES_TRAINING * sizeof(int));

    /* ponteiro para o arquivo de entrada */
    FILE *file_input;

    /* ponteiro para o arquivo de log de saída */
    FILE *file_log_output;

    /* ponteiro para o arquivo de dados de saída */
    FILE *file_data_output;

    /* linha do arquivo */
    char *line; 

    /* matrizes com dados para teste e treinamento */
    double **data_testing, **data_training; 

    /* vetores com labels para teste e treinamento */
    int *labels_testing, *labels_training; 

    /* cria o arquivo log de saída */
    file_log_output = fopen("log_output", "w");

    /* cria o arquivo de saída de dados */
    file_data_output = fopen("data_output", "w");

    /* tenta abrir o arquivo de entrada */
    if ((file_input = fopen("../data/fer2013.csv", "r")) == NULL) {
        fprintf(file_log_output, "Não foi possível abrir o arquivo!");
        return -1;
    }

    /* lê a primeira linha do arquivo */
    if (csvgetline(file_input, ',', 0) == NULL) {
        fprintf(file_log_output, "Arquivo está vazio!");
        return -1;
    }

    /* realiza alocação de espaços de memórias para matrizes e vetores usados */
    data_testing = (double **) malloc(NUM_IMAGES_TESTING * sizeof(double *));
    data_training = (double **) malloc(NUM_IMAGES_TRAINING * sizeof(double *));
    labels_testing = (int *) malloc(NUM_IMAGES_TESTING * sizeof(int));
    labels_training = (int *) malloc(NUM_IMAGES_TRAINING * sizeof(int));

    read_data_and_labels(file_input, data_testing, data_training, labels_testing, labels_training);

    initialize_weights(weights);

    /* realiza iterações até o número máximo de épocas */
    while (num_epochs < num_max_epochs) {

        /* realiza iterações de acordo com o número de imagens para treinamento */
        for(int r=0; r < NUM_IMAGES_TRAINING; r++) {

            all_hypothesis[r] = hypothesis_function((double *) data_training[r], weights);

            //realiza binarização dos valores de hipótese
            if(all_hypothesis[r] >= 0.5) {
                results[r] = 1;
            } else {
                results[r] = 0;
            }
        }

        update_weights(data_training, weights, all_hypothesis, labels_training, learning_rate);

        //for(int r=0; r < NUM_PIXELS; r++) {
          //  printf("%lf \n", weights[r]);
        //}


        save_results(num_epochs, results, labels_training, NUM_IMAGES_TRAINING);
        printf("Custo: %lf\n", cost_function(all_hypothesis, weights, labels_training, NUM_IMAGES_TRAINING));
        num_epochs++;
    }
    
    printf("\n\n\n\n\n\n\n\n");

    double hypothesis = 0;

    for(int r=0; r < NUM_IMAGES_TESTING; r++) {
         hypothesis = hypothesis_function((double *) data_testing[r], weights);
                 
        //binarize hypothesis
        if(hypothesis >= 0.5) {
            results[r] = 1;
        } else {
            results[r] = 0;
        }

         if(hypothesis >= 0.5 && labels_testing[r] == 1) {
             corretos++;
         } else if(hypothesis < 0.5 && labels_testing[r] == 0) {
            corretos++;
         }
    }
    
    printf("CORRETOS: %d\n", corretos);

    save_results(num_epochs, results, labels_testing, NUM_IMAGES_TESTING);
}
