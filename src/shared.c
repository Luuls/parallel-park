#include "shared.h"

#include <queue.h>


// Você pode declarar novas funções (ou variaveis compartilhadas) aqui


/**********************************
 *          ATENÇÃO               *
 *   NÃO EDITAR ESSAS VARIAVEIS   *
 *          ATENÇÃO               *
 *********************************/
Queue* gate_queue = NULL;

pthread_mutex_t gate_queue_mutex;  // Mutex para manipular a fila

sem_t* clients_semaphores;  // Semáforo de cada cliente da fila, libera entrada no parque