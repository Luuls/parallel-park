#ifndef __SHARED_H__
#define __SHARED_H__

#include <defs.h>

// Você pode declarar novas funções (ou variaveis compartilhadas) aqui



/**********************************
 *          ATENÇÃO               *
 *   NÃO EDITAR ESSAS VARIAVEIS   *
 *          ATENÇÃO               *
 *********************************/
extern Queue* gate_queue;

extern pthread_mutex_t gate_queue_mutex;

extern sem_t* clients_semaphores;

#endif