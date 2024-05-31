#ifndef __SHARED_H__
#define __SHARED_H__

#include <defs.h>

// Você pode declarar novas funções (ou variaveis compartilhadas) aqui

extern pthread_mutex_t gate_queue_mutex;
extern sem_t clients_in_queue_sem;

extern sem_t* clients_ticket_booth_access;

extern int clients_to_be_served;
extern pthread_mutex_t clients_to_be_served_mutex;

extern int clients_to_leave;     // Quantidade de clientes que ainda precisam sair.
extern pthread_mutex_t clients_to_leave_mutex;  // Mutex para manipular o contador clients_to_leave

/**********************************
 *          ATENÇÃO               *
 *   NÃO EDITAR ESSAS VARIAVEIS   *
 *          ATENÇÃO               *
 *********************************/
extern Queue* gate_queue;

#endif