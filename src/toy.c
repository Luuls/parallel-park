/**
 * Esse arquivo tem como objetivo a implementação de um brinquedo em um parque.
 * Você pode criar novas funções dentro desse arquivo caso julgue necessário.
 * ATENÇÃO: NÃO APAGUE OU EDITE O NOME OU O(S) ARGUMENTO(S) DAS FUNÇÕES FORNECIDAS
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "toy.h"
#include "shared.h"

// Variável local ao arquivo para guardar os dados dos brinquedos
static toy_args *toys_data = NULL;

void initiate_toy(toy_t* toy) {
    // Diverte os clientes pela duração de tempo do brinquedo
    sleep(toy->duration);
}

void wait_for_more_people(void) {
    // Espera por algum tempo para que mais pessoas cheguem antes de começar
    sleep(1);
}

void* turn_on(void* args) {
    // Thread que o brinquedo usa durante a simulação do sistema
    toy_t* self = (toy_t*) args;

    debug("[ON] - O brinquedo  [%d] foi ligado.\n", self->id);

    while (TRUE) {
        /*
        A thread aguarda um sinal para começar a iteração. Esse sinal pode ser enviado por:
        1 - Um cliente. Nesse caso, o brinquedo deve permitir que o cliente brinque.
        2 - O funcionário do parque. Nesse caso, o brinquedo sabe que pode desligar.
        */
        sem_wait(&self->toy_perform_actions);
        
        pthread_mutex_lock(&clients_to_leave_mutex);
        if (clients_to_leave == 0) {
            pthread_mutex_unlock(&clients_to_leave_mutex);
            break;
        }
        pthread_mutex_unlock(&clients_to_leave_mutex);

        wait_for_more_people();

        /*
        O número de pessoas que vão brincar (clients_enjoying) é o mínimo entre
        a capacidade do brinquedo e o número de pessoas que querem brincar.
        */
        pthread_mutex_lock(&self->clients_to_enter_toy_mutex);
        int clients_enjoying = self->clients_to_enter_toy;
        pthread_mutex_unlock(&self->clients_to_enter_toy_mutex);
    
        if (clients_enjoying > self->capacity) {
            clients_enjoying = self->capacity;
        }

        debug("[TOY] - O brinquedo [%d] está divertindo [%d] pessoas!\n", self->id, clients_enjoying);
        initiate_toy(self);

        pthread_mutex_lock(&self->clients_to_enter_toy_mutex);
        self->clients_to_enter_toy -= clients_enjoying; 
        pthread_mutex_unlock(&self->clients_to_enter_toy_mutex);

        /*
        Sinaliza para os clientes que eles podem brincar e atualiza o semáforo
        toy_perform_actions com base em quantos entraram. Note que toy_perform_actions
        recebe um post a menos, pois já foi feito um sem_wait no início da iteração.
        */
        for (int i = 0; i < clients_enjoying - 1; i++) {
            sem_wait(&self->toy_perform_actions);
            sem_post(&self->clients_wanting_to_ride);
        }
        sem_post(&self->clients_wanting_to_ride);
    }

    debug("[OFF] - O brinquedo [%d] foi desligado.\n", self->id); // Altere para o id do brinquedo

    pthread_exit(NULL);
}

void open_toys(toy_args* args) {
    /*
    Essa função recebe como argumento informações e deve iniciar os brinquedos.
    Também é responsável por inicializar as estruturas de sincronização necessárias.
    */

    toys_data = args;

    for (int i = 0; i < args->n; i++) {
        sem_init(&args->toys[i]->toy_perform_actions, 0, 0);
        sem_init(&args->toys[i]->clients_wanting_to_ride, 0, 0);

        args->toys[i]->clients_to_enter_toy = 0;
        pthread_mutex_init(&args->toys[i]->clients_to_enter_toy_mutex, NULL);

        pthread_create(&args->toys[i]->thread, NULL, turn_on, (void*) args->toys[i]);
    }
}

// Desligando os brinquedos
void close_toys() {
    /*
    Esperamos que todos os brinquedos terminem suas execuções e destruímos o que for preciso.
    */

    for (int i = 0; i < toys_data->n; i++) {
        pthread_join(toys_data->toys[i]->thread, NULL);
        sem_destroy(&toys_data->toys[i]->toy_perform_actions);
        sem_destroy(&toys_data->toys[i]->clients_wanting_to_ride);
        pthread_mutex_destroy(&toys_data->toys[i]->clients_to_enter_toy_mutex);
    }

    pthread_mutex_destroy(&clients_to_leave_mutex);
}