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
        pthread_mutex_lock(&gate_queue_mutex);
        if (is_queue_empty(gate_queue)) {
            pthread_mutex_unlock(&gate_queue_mutex);
            break;
        }

        int id = dequeue(gate_queue);
        pthread_mutex_unlock(&gate_queue_mutex);
        sem_post(&clients_semaphores[id - 1]);
    }
    pthread_exit(NULL);
}

// Essa função recebe como argumento informações sobre a bilheteria e deve iniciar os atendentes.
void open_tickets(tickets_args* args) {
    // Sua lógica aqui
    pthread_mutex_init(&gate_queue_mutex, NULL);

    for (int i = 0; i < args->n; i++) {
        pthread_create(&args->tickets[i]->thread, NULL, sell, NULL);
    }
}

// Essa função deve finalizar a bilheteria
void close_tickets(tickets_args* args) {
    //Sua lógica aqui

    for (int i = 0; i < args->n; i++) {
        pthread_join(args->tickets[i]->thread, NULL);
    }
    pthread_mutex_destroy(&gate_queue_mutex);
}