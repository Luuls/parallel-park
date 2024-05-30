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


// Thread que o brinquedo vai usar durante toda a simulacao do sistema
void* turn_on(void* args) {
    toy_t* self = (toy_t*) args;

    debug("[ON] - O brinquedo  [%d] foi ligado.\n", self->id); // Altere para o id do brinquedo

    while (TRUE) {
        sem_wait(&self->toy_perform_actions);
        
        pthread_mutex_lock(&clients_to_leave_mutex);
        if (clients_to_leave == 0) {
            pthread_mutex_unlock(&clients_to_leave_mutex);
            break;
        }
        pthread_mutex_unlock(&clients_to_leave_mutex);

        debug("[TOY] - Brinquedo [%d] está realizando ações.\n", self->id);
    }

    debug("[OFF] - O brinquedo [%d] foi desligado.\n", self->id); // Altere para o id do brinquedo

    pthread_exit(NULL);
}


// Essa função recebe como argumento informações e deve iniciar os brinquedos.
void open_toys(toy_args* args) {
    // Sua lógica aqui
    for (int i = 0; i < args->n; i++) {
        sem_init(&args->toys[i]->toy_perform_actions, 0, 0);
        sem_init(&args->toys[i]->clients_wanting_to_ride, 0, 0);
        pthread_create(&args->toys[i]->thread, NULL, turn_on, (void*) args->toys[i]);
    }
}

// Desligando os brinquedos
void close_toys(toy_args* args) {
    for (int i = 0; i < args->n; i++) {
        sem_destroy(&args->toys[i]->toy_perform_actions);
        sem_destroy(&args->toys[i]->clients_wanting_to_ride);
        pthread_join(args->toys[i]->thread, NULL);
    }

    pthread_mutex_destroy(&clients_to_leave_mutex);
}