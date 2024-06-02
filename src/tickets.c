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

#define NO_MORE_CLIENTS -1

// Variável local ao arquivo que armazena os dados dos tickets
static tickets_args *tickets_data = NULL;

int choose_client_to_serve(void) {
    /*
    Escolhe um cliente para ser atendido. Se todos os clientes já foram ou serão
    atendidos (clients_to_be_served == 0), a função retorna NO_MORE_CLIENTS e o
    funcionário encerra seu turno.
    */

    pthread_mutex_lock(&clients_to_be_served_mutex);
    if (clients_to_be_served == 0) {
        pthread_mutex_unlock(&clients_to_be_served_mutex);
        return NO_MORE_CLIENTS;
    }
    clients_to_be_served--;
    pthread_mutex_unlock(&clients_to_be_served_mutex);

    sem_wait(&clients_in_queue_sem);  // Aguarda a chegada de um cliente para atendê-lo

    pthread_mutex_lock(&gate_queue_mutex);
    int client_id = dequeue(gate_queue);
#ifndef NDEBUG
    debug("[INFO] - Turista [%d] está aproveitando o parque...\n", client_id);
#endif
    pthread_mutex_unlock(&gate_queue_mutex);

#ifdef NDEBUG
    debug("[INFO] - Turista [%d] está aproveitando o parque...\n", client_id);
#endif

    return client_id;
}

void* sell(void* args) {
    // Rotina que descreve o comportamento de um atendente de bilheteria

    ticket_t* ticket = (ticket_t*) args;
    debug("[INFO] - Bilheteria [%d] abriu!\n", ticket->id);

    while (TRUE) {
        int client_id = choose_client_to_serve();
        if (client_id == NO_MORE_CLIENTS) {
            break;
        }

        sem_post(&clients_ticket_booth_access[client_id - 1]);  // Atende o cliente
    }

    debug("[INFO] - Bilheteria [%d] fechou!\n", ticket->id);
    pthread_exit(NULL);
}

void open_tickets(tickets_args* args) {
    // Essa função recebe como argumento informações sobre a bilheteria e inicia os atendentes.
    tickets_data = args;
    for (int i = 0; i < args->n; i++)
    {
        pthread_create(&args->tickets[i]->thread, NULL, sell, (void*) args->tickets[i]);
    }
}

// Essa função deve finalizar a bilheteria
void close_tickets() {
    // Espera as threads acabarem e destrói mutexes e semáforos.

    for (int i = 0; i < tickets_data->n; i++) {
        pthread_join(tickets_data->tickets[i]->thread, NULL);
    }

    pthread_mutex_destroy(&gate_queue_mutex);
    sem_destroy(&clients_in_queue_sem);
    pthread_mutex_destroy(&clients_to_be_served_mutex);
}
