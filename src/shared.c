#include "shared.h"

#include <queue.h>


// Você pode declarar novas funções (ou variaveis compartilhadas) aqui

pthread_mutex_t gate_queue_mutex;  // Mutex para manipular a fila
sem_t clients_in_queue_sem;  // Semáforo para sincronizar clientes e bilheteria

sem_t* clients_access_controls;  // Array de semáforos, um para cada cliente da fila. Libera entrada no parque

int remaining_clients;  // Contagem de quantas pessoas ainda precisam ser atendidas
pthread_mutex_t remaining_clients_mutex;  // Mutex para manipular o contador remaining_clients

/**********************************
 *          ATENÇÃO               *
 *   NÃO EDITAR ESSAS VARIAVEIS   *
 *          ATENÇÃO               *
 *********************************/
Queue* gate_queue = NULL;
