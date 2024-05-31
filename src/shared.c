#include "shared.h"

#include <queue.h>


// Você pode declarar novas funções (ou variaveis compartilhadas) aqui

pthread_mutex_t gate_queue_mutex;  // Mutex para manipular a fila
sem_t clients_in_queue_sem;  // Semáforo para sincronizar clientes e bilheteria

sem_t* clients_ticket_booth_access;  // Array de semáforos, um para cada cliente. Libera o cliente p/ ser atendido

int clients_to_be_served;  // Contagem de quantas pessoas ainda precisam ser atendidas
pthread_mutex_t clients_to_be_served_mutex;  // Mutex para manipular o contador clients_to_be_served

int clients_to_leave;     // Quantidade de clientes que ainda precisam sair.
pthread_mutex_t clients_to_leave_mutex;  // Mutex para manipular o contador clients_to_leave

/**********************************
 *          ATENÇÃO               *
 *   NÃO EDITAR ESSAS VARIAVEIS   *
 *          ATENÇÃO               *
 *********************************/
Queue* gate_queue = NULL;
