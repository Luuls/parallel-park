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

/*
Variável local ao arquivo para guardar o funcionário do parque. Esse funcionário
é responsável por ficar de olho nos clientes que estão saindo do parque. Quando o
último cliente sai, o funcionário envia um sinal para todos os brinquedos.
*/
static park_employee_t park_employee;

// Variável local ao arquivo para manter os dados dos clientes
static client_args *clients_data = NULL;

void buy_coins(client_t* self) {
    // Função onde o cliente compra as moedas para usar nos brinquedos.
    self->coins = rand() % (MAX_COINS) + 1;
}

void wait_ticket(client_t* self) {
    // Função onde o cliente espera a liberação da bilheteria para adentrar no parque.
    sem_wait(&clients_ticket_booth_access[self->id - 1]);
}

void enjoy_toys(client_t* self) {
    /*
    Função onde o cliente aproveita os brinquedos do parque.
    1 - Escolhemos um brinquedo aleatório.
    2 - Gastamos uma moeda para brincar.
    3 - Aguardamos a liberação do brinquedo.
    Repetimos esses passos até que o cliente não tenha mais moedas.
    */

    while (self->coins > 0) {
        toy_t* toy = self->toys[rand() % self->number_toys];
        debug("[TOY] - Turista [%d] escolheu brincar no brinquedo [%d].\n", self->id, toy->id);

        self->coins--;
        debug("[CASH] - Turista [%d] gastou uma moeda. Restam [%d] moedas.\n", self->id, self->coins);

        debug("[WAITING] - Turista [%d] está aguardando para brincar no brinquedo [%d].\n", self->id, toy->id);
        pthread_mutex_lock(&toy->clients_to_enter_toy_mutex);
        toy->clients_to_enter_toy++;
        pthread_mutex_unlock(&toy->clients_to_enter_toy_mutex);

        sem_post(&toy->toy_perform_actions);  // Sinalizamos para o brinquedo que um cliente quer brincar

        sem_wait(&toy->clients_wanting_to_ride);  // Aguarda que o brinquedo o libere para entrar
        debug("[TOY] - O turista [%d] se divertiu brinquedo [%d].\n", self->id, toy->id);

    }
}

void queue_enter(client_t* self) {
    /*
    Função onde o cliente entra na fila da bilheteria e aguarda ser atendido por um
    'ticket', após o que ele compra suas moedas.
    */

    pthread_mutex_lock(&gate_queue_mutex);
    enqueue(gate_queue, self->id);
#ifndef NDEBUG
    debug("[WAITING] - Turista [%d] entrou na fila do portao principal\n", self->id);
#endif
    pthread_mutex_unlock(&gate_queue_mutex);

#ifdef NDEBUG
    debug("[WAITING] - Turista [%d] entrou na fila do portao principal\n", self->id);
#endif

    sem_post(&clients_in_queue_sem);  // Sinalizamos para a bilheteria que um cliente entrou na fila

    wait_ticket(self);

    buy_coins(self);
    debug("[CASH] - Turista [%d] comprou [%d] moedas.\n", self->id, self->coins);
}

void leave_park(void) {
    // Ao sair do parque, informa isso ao funcionário
    sem_post(&park_employee.employee_perform_action);
}

void* park_employee_action(void* arg) {
    /*
    Função que implementa a thread do funcionário que cuida dos clientes que
    estão saindo do parque.
    */

    park_employee_t* self = (park_employee_t*) arg;

    while (TRUE) {
        sem_wait(&self->employee_perform_action);  // Aguarda o sinal de um cliente que está saindo

        pthread_mutex_lock(&clients_to_leave_mutex);
        clients_to_leave--;

        /*
        Quando o último cliente sair do parque, o funcionário
        sinaliza as threads dos brinquedos para continuarem,
        possibilitando que elas finalizem suas execuções
        */
        if (clients_to_leave == 0) {
            pthread_mutex_unlock(&clients_to_leave_mutex);

            for (int i = 0; i < self->number_toys; i++) {
                sem_post(&self->toys[i]->toy_perform_actions);
            }
            break;
        }
        pthread_mutex_unlock(&clients_to_leave_mutex);
    }

    pthread_exit(NULL);
}

void* enjoy(void* arg) {
    // Thread que implementa o fluxo do cliente no parque.

    client_t* self = (client_t*) arg;

    queue_enter(self);  // Para entrar no parque, primeiro passamos pela fila e compramos moedas

#ifdef NDEBUG
    debug("Turista [%d] está aproveitando o parque...\n", self->id);
#endif

    enjoy_toys(self);  // Depois de comprar moedas, o cliente aproveita os brinquedos

    leave_park();  // Após as moedas acabarem, o cliente sai do parque

    debug("[EXIT] - O turista [%d] saiu do parque.\n", self->id);
    pthread_exit(NULL);
}

void open_gate(client_args* args) {
    /*
    Os portões são finalmente abertos, permitindo a entrada de clientes. Eles são
    representados por threads. Um funcionário, responsável por ficar de olho nos
    clientes que estão saindo, também é instanciado. Inicializamos várias estruturas
    que nos ajudarão a controlar os clientes e o funcionário (ver shared.c).
    */

    clients_data = args;
    pthread_mutex_init(&gate_queue_mutex, NULL);
    sem_init(&clients_in_queue_sem, 0, 0);

    clients_to_be_served = args->n;
    pthread_mutex_init(&clients_to_be_served_mutex, NULL);

    clients_ticket_booth_access = malloc(args->n * sizeof(sem_t));

    // É possível que nenhum cliente tenha visitado o parque hoje.
    if (args->n != 0) {
        park_employee.toys = args->clients[0]->toys;
        park_employee.number_toys = args->clients[0]->number_toys;
    }
    else {
        debug("[INFO] - Nenhum cliente visitou o parque hoje :(\n")
    }

    sem_init(&park_employee.employee_perform_action, 0, 0);
    pthread_create(&park_employee.thread, NULL, park_employee_action, &park_employee);

    clients_to_leave = args->n;
    pthread_mutex_init(&clients_to_leave_mutex, NULL);

    for (int i = 0; i < args->n; i++) {
        sem_init(&clients_ticket_booth_access[i], 0, 0);
        pthread_create(&args->clients[i]->thread, NULL, enjoy, args->clients[i]);
    }
}

void close_gate() {
   /*
    Depois que um cliente termina de aproveitar o parque (isto é, sua thread se
    encerra), consideramos que ele saiu. O funcionário do parque sai após o último
    cliente, pois não é mais necessário. Destruímos o que não for mais preciso.
   */

    for (int i = 0; i < clients_data->n; i++) {
        pthread_join(clients_data->clients[i]->thread, NULL);
        sem_destroy(&clients_ticket_booth_access[clients_data->clients[i]->id - 1]);
    }
    pthread_join(park_employee.thread, NULL);

    sem_destroy(&park_employee.employee_perform_action);

    free(clients_ticket_booth_access);
}
