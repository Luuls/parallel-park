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
    ticket_t* ticket = (ticket_t*) args;
    debug("[INFO] - Bilheteria [%d] abriu!\n", ticket->id);

    while (TRUE) {
        pthread_mutex_lock(&clients_to_be_served_mutex);
        if (clients_to_be_served == 0) {  // Os últimos clientes foram/vão ser atendidos. O funcionário encerra seu turno
            pthread_mutex_unlock(&clients_to_be_served_mutex);
            break;
        }
        clients_to_be_served--;
        pthread_mutex_unlock(&clients_to_be_served_mutex);

        sem_wait(&clients_in_queue_sem);  // Aguarda a chegada de um cliente na fila

        pthread_mutex_lock(&gate_queue_mutex);
        int id = dequeue(gate_queue);  // Chama o cliente que está frente da fila para ser atendido
        pthread_mutex_unlock(&gate_queue_mutex);

        sem_post(&clients_ticket_booth_access[id - 1]);  // Atende o cliente
    }

    debug("[INFO] - Bilheteria [%d] fechou!\n", ticket->id);
    pthread_exit(NULL);
}

// Essa função recebe como argumento informações sobre a bilheteria e deve iniciar os atendentes.
void open_tickets(tickets_args* args) {
    for (int i = 0; i < args->n; i++) {
        pthread_create(&args->tickets[i]->thread, NULL, sell, (void*) args->tickets[i]);
    }
}

// Essa função deve finalizar a bilheteria
void close_tickets(tickets_args* args) {
    /*
    Assim que as threads dos atendentes terminam, podemos destruir os mutexes e
    semáforos restantes, pois não precisaremos mais deles.
    */

    for (int i = 0; i < args->n; i++) {
        pthread_join(args->tickets[i]->thread, NULL);
    }

    pthread_mutex_destroy(&gate_queue_mutex);
    sem_destroy(&clients_in_queue_sem);
    pthread_mutex_destroy(&clients_to_be_served_mutex);
}