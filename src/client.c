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


// Função onde o cliente compra as moedas para usar os brinquedos
void buy_coins(client_t* self) {
    self->coins = rand() % (MAX_COINS) + 1;
}

// Função onde o cliente espera a liberação da bilheteria para adentrar no parque.
void wait_ticket(client_t* self) {
    // Aguarda o atendimento de um funcionário da bilheteria
    sem_wait(&clients_ticket_booth_access[self->id - 1]);
}

void enjoy_toys(client_t* self) {
    while (self->coins > 0) {
        toy_t* toy = self->toys[rand() % self->number_toys];  // O cliente escolhe um brinquedo aleatório
        debug("[TOY] - Turista [%d] escolheu brincar no brinquedo [%d].\n", self->id, toy->id);

        self->coins--;  // O cliente gasta uma moeda para brincar
        debug("[COINS] - Turista [%d] gastou uma moeda. Restam [%d] moedas.\n", self->id, self->coins);
    }
}

// Função onde o cliente entra na fila da bilheteria
void queue_enter(client_t* self) {
    pthread_mutex_lock(&gate_queue_mutex);
    enqueue(gate_queue, self->id);  // O cliente entra na fila do parque
    pthread_mutex_unlock(&gate_queue_mutex);

    sem_post(&clients_in_queue_sem);  // Sinalizamos para a bilheteria que um cliente entrou na fila

    debug("[WAITING] - Turista [%d] entrou na fila do portao principal\n", self->id);
    wait_ticket(self);  // Aguarda que alguém na bilheteria o atenda

    buy_coins(self);  // Um funcionário finalmente atende o cliente e ele compra suas moedas
    debug("[CASH] - Turista [%d] comprou [%d] moedas.\n", self->id, self->coins);
}

// Thread que implementa o fluxo do cliente no parque.
void* enjoy(void* arg) {
    client_t* self = (client_t*) arg;

    queue_enter(self);  // Para entrar no parque, primeiro passamos pela fila e compramos moedas

    enjoy_toys(self);  // Depois de comprar moedas, o cliente aproveita os brinquedos

    debug("Turista [%d] está aproveitando o parque...\n", self->id);  // O cliente saiu da fila e agora está dentro do parque.

    debug("[EXIT] - O turista [%d] saiu do parque.\n", self->id);
    pthread_exit(NULL);
}

// Essa função recebe como argumento informações sobre o cliente e deve iniciar os clientes.
void open_gate(client_args* args) {
    /*
    Função de abertura dos portões, permitindo a entrada de clientes. Inicializamos
    gate_queue_mutex, clients_in_queue_sem, remaining_clients, remaining_clients_mutex e
    clients_ticket_booth_access, estruturas que nos ajudarão a sincronizar a entrada dos
    clientes no parque (ver shared.c).
    */

    pthread_mutex_init(&gate_queue_mutex, NULL);
    sem_init(&clients_in_queue_sem, 0, 0);

    remaining_clients = args->n;
    pthread_mutex_init(&remaining_clients_mutex, NULL);

    clients_ticket_booth_access = malloc(args->n * sizeof(sem_t));

    for (int i = 0; i < args->n; i++) {
        sem_init(&clients_ticket_booth_access[i], 0, 0);
        pthread_create(&args->clients[i]->thread, NULL, enjoy, args->clients[i]);
    }
}

// Essa função deve finalizar os clientes
void close_gate(client_args* args) {
   /*
    Depois que um cliente termina de aproveitar o parque, podemos destruir o semáforo
    associado a ele.
   */

    for (int i = 0; i < args->n; i++) {
        pthread_join(args->clients[i]->thread, NULL);
        sem_destroy(&clients_ticket_booth_access[args->clients[i]->id - 1]);
    }

    free(clients_ticket_booth_access);
}
