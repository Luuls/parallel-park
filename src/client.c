/**
 * Esse arquivo tem como objetivo a implementação do cliente em um parque.
 * Você pode criar novas funções dentro desse arquivo caso julgue necessário.
 * ATENÇÃO: NÃO APAGUE OU EDITE O NOME OU O(S) ARGUMENTO(S) DAS FUNÇÕES FORNECIDAS
*/

#include <stdio.h>
#include <stdlib.h>

#include "defs.h"
#include "client.h"
#include "queue.h"
#include "shared.h"

// Funcao onde o cliente compra as moedas para usar os brinquedos
void buy_coins(client_t* self) {
    // Sua lógica aqui
    self->coins = rand() % (MAX_COINS) + 1;
}

// Função onde o cliente espera a liberacao da bilheteria para adentrar ao parque.
void wait_ticket(client_t* self) {
    // Sua lógica aqui
    sem_wait(&clients_semaphores[self->id - 1]);
    debug("[ENTER] - Turista [%d] liberado para o parque.\n", self->id);
}

// Funcao onde o cliente entra na fila da bilheteria
void queue_enter(client_t* self) {
    // Sua lógica aqui.

    enqueue(gate_queue, self->id);

    debug("[WAITING] - Turista [%d] entrou na fila do portao principal\n", self->id);

    // Sua lógica aqui.
    buy_coins(self);

    // Sua lógica aqui.
    debug("[CASH] - Turista [%d] comprou [%d] moedas.\n", self->id, self->coins);
}

// Thread que implementa o fluxo do cliente no parque.
void* enjoy(void* arg) {
    //Sua lógica aqui
    client_t* self = (client_t*) arg;

    wait_ticket(self);

    debug("[EXIT] - O turista saiu do parque.\n");
    pthread_exit(NULL);
}

// Essa função recebe como argumento informações sobre o cliente e deve iniciar os clientes.
void open_gate(client_args* args) {
    // Sua lógica aqui

    for (int i = 0; i < args->n; i++) {
        queue_enter(args->clients[i]);
    }

    clients_semaphores = malloc(args->n * sizeof(sem_t));
    for (int i = 0; i < args->n; i++) {
        sem_init(&clients_semaphores[i], 0, 0);
        pthread_create(&args->clients[i]->thread, NULL, enjoy, args->clients[i]);
    }
}

// Essa função deve finalizar os clientes
void close_gate(client_args* args) {
   //Sua lógica aqui
    for (int i = 0; i < args->n; i++) {
        pthread_join(args->clients[i]->thread, NULL);
    }
    for (int i = 0; i < args->n; i++) {
        sem_destroy(&clients_semaphores[args->clients[i]->id - 1]);
    }
    free(clients_semaphores);
}
