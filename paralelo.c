#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

// #define DEBUG 1            // comentar esta linha quando for medir tempo
#define tam_vetor 10000      // trabalho final com o valores 10000, 100000, 1000000
#define array_cut_percentage 10 // porcetagem de corte dos arrays

void bs(int n, int * vetor, int start)
{
    int c=0, d, troca, trocou =1;

    while (c < (n-1) && trocou )
        {
        trocou = 0;
        for (d = start ; d < n - c - 1; d++)
            if (vetor[d] > vetor[d+1])
                {
                troca      = vetor[d];
                vetor[d]   = vetor[d+1];
                vetor[d+1] = troca;
                trocou = 1;
                }
        c++;
        }
}

int main(int argc, char** argv)
{
    int i;
    int id;            /* Identificador do processo */
    int n;             /* Numero de processos */
    double time1, time2;
    MPI_Status Status; /* Status de retorno */
    int tam = tam_vetor;
    int pronto = 0;
    int maxLeft = 0;
    int leftSorted;
    int x = 0;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    time1 = MPI_Wtime();


    int delta = tam_vetor/n;
    int cutSize = delta*array_cut_percentage/100;
    int vetor[delta+cutSize];
    int sorted[n];
    for (i = 0; i < delta; i++) /* init array with worst case for sorting */
        vetor[i] = (delta*(n-id))-i;
    while(!pronto){
        bs(delta, vetor, 0);

        // verifica parada
        if(id < n-1){ // se não for np-1, mando o meu maior elemento para a direita
            MPI_Send(&vetor[delta-1], 1, MPI_INT, id+1, 0, MPI_COMM_WORLD);
        }

        if(id != 0){ // se não for 0, recebo o maior elemento da esquerda
            MPI_Recv(&maxLeft, 1, MPI_INT, id-1, 0, MPI_COMM_WORLD, &Status);
        }
        if (maxLeft < vetor[0]){ // comparo se o meu menor elemento é maior do que o maior elemento recebido (se sim, estou ordenado em relação ao meu vizinho)
            leftSorted = 1;
        } else {
            leftSorted = 0;
        }
        sorted[id] = leftSorted;
        for(i = 0; i < n; i++){
            // int sorted = leftSorted;
            MPI_Bcast(&sorted[i], 1, MPI_INT, i, MPI_COMM_WORLD);
        }
        pronto = 1;
        for (i=0 ; i<n; i++) { // verifica se esta pronto
            if (!sorted[i]) {
                pronto = 0;
            }
        }
        if(!pronto) {// troco valores para convergir
            if(id != 0){ // se não for o 0, mando os menores valores do meu vetor para a esquerda
                MPI_Send(&vetor, cutSize, MPI_INT, id-1, 1, MPI_COMM_WORLD);
            }
            if(id < n-1) {// se não for np-1, recebo os menores valores da direita
                MPI_Recv(&vetor[delta], cutSize, MPI_INT, id+1, 1, MPI_COMM_WORLD, &Status);

                bs(delta+cutSize, vetor, delta-cutSize); // ordeno estes valores com a parte mais alta do meu vetor local

                MPI_Send(&vetor[delta], cutSize, MPI_INT, id+1, 2, MPI_COMM_WORLD); // devolvo os valores que recebi para a direita
            }

            if(id != 0) { // se não for o 0, recebo de volta os maiores valores da esquerda
                MPI_Recv(&vetor, cutSize, MPI_INT, id-1, 2, MPI_COMM_WORLD, &Status);
            }
        }
    }

    // printf("\nsorted Array, id %d: ", id);
    // for (i=0 ; i<delta; i++)
    //     printf("[%03d] ", vetor[i]);
    // printf("\n");
    if(id == 0) {// sou raiz, mostrar o vetor e o tempo de execução

        #ifdef DEBUG
            printf("\nsorted Array: ");
            for (i=0 ; i<tam; i++)
                printf("[%03d] ", vetor_auxiliar[i]);
        #endif
        time2 = MPI_Wtime();
        printf("\nsize: %d", tam);
        printf("\ntime: %f\n", time2-time1);
    }
    MPI_Finalize();
    return 0;
}