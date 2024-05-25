/**
 * Esse arquivo tem como objetivo a implementação da bilheteria em um parque.
 * Você pode criar novas funções dentro desse arquivo caso julgue necessário.
 * ATENÇÃO: NÃO APAGUE OU EDITE O NOME OU O(S) ARGUMENTO(S) DAS FUNÇÕES FORNECIDAS.
*/
#include <stdio.h>
#include <stdlib.h>
#include <tickets.h>
#include <unistd.h>

#include <queue.h>
#include "shared.h"


// Thread que implementa uma bilheteria
void* sell(void* args) {
    debug("[INFO] - Bilheteria Abriu!\n");

    while (TRUE) {
        pthread_mutex_lock(&remaining_clients_mutex);
        if (remaining_clients == 0) {  // Os últimos clientes foram/vão ser atendidos. O funcionário encerra seu turno
            pthread_mutex_unlock(&remaining_clients_mutex);
            break;
        }
        remaining_clients--;  //
        pthread_mutex_unlock(&remaining_clients_mutex);

        sem_wait(&clients_in_queue_sem);  // Aguarda a chegada de um cliente na fila

        pthread_mutex_lock(&gate_queue_mutex);
        int id = dequeue(gate_queue);  // Atende o cliente cujo ID está na frente da fila
        pthread_mutex_unlock(&gate_queue_mutex);

        sem_post(&clients_access_controls[id - 1]);  // Libera o cliente para entrar no parque
    }

    pthread_exit(NULL);
}

// Essa função recebe como argumento informações sobre a bilheteria e deve iniciar os atendentes.
void open_tickets(tickets_args* args) {
    // Cria as threads dos atendentes
    for (int i = 0; i < args->n; i++) {
        pthread_create(&args->tickets[i]->thread, NULL, sell, NULL);
    }
}

// Essa função deve finalizar a bilheteria
void close_tickets(tickets_args* args) {
    // Espera todas as threads dos atendentes finalizarem e destrói mutexes/semáforos
    for (int i = 0; i < args->n; i++) {
        pthread_join(args->tickets[i]->thread, NULL);
    }

    pthread_mutex_destroy(&gate_queue_mutex);
    sem_destroy(&clients_in_queue_sem);
    pthread_mutex_destroy(&remaining_clients_mutex);
}