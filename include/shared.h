#ifndef __SHARED_H__
#define __SHARED_H__

#include <defs.h>

// Você pode declarar novas funções (ou variaveis compartilhadas) aqui

extern pthread_mutex_t gate_queue_mutex;
extern sem_t clients_in_queue_sem;

extern sem_t* clients_access_controls;

extern int remaining_clients;
extern pthread_mutex_t remaining_clients_mutex;


/**********************************
 *          ATENÇÃO               *
 *   NÃO EDITAR ESSAS VARIAVEIS   *
 *          ATENÇÃO               *
 *********************************/
extern Queue* gate_queue;

#endif