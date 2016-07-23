#include <arpa/inet.h>
#include <err.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "gonm_master.h"
#include "gonm_context.h"

void gonm_master_start(struct gonm_master_args* master_args)
{
    struct sockaddr_in server_address;
    memset(server_address.sin_zero, 0, sizeof(server_address.sin_zero));
    server_address.sin_family = AF_INET;
    inet_aton(master_args->address, &server_address.sin_addr);
    server_address.sin_port = htons(master_args->port);

    if((master_args->worker_args->server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
        err(EXIT_FAILURE, "%s:%d", __FILE__, __LINE__);

    if(setsockopt(master_args->worker_args->server_socket, SOL_SOCKET, SO_REUSEADDR, &(const int){1}, sizeof(const int)) == -1)
        err(EXIT_FAILURE, "%s:%d", __FILE__, __LINE__);

    if(setsockopt(master_args->worker_args->server_socket, SOL_SOCKET, SO_REUSEPORT, &(const int){1}, sizeof(const int)) == -1)
        err(EXIT_FAILURE, "%s:%d", __FILE__, __LINE__);

    if(bind(master_args->worker_args->server_socket, (struct sockaddr*)&server_address, sizeof(struct sockaddr)) == -1)
        err(EXIT_FAILURE, "%s:%d", __FILE__, __LINE__);

    if(listen(master_args->worker_args->server_socket, master_args->backlog) == -1)
        err(EXIT_FAILURE, "%s:%d", __FILE__, __LINE__);

    pthread_t* threads = malloc(master_args->worker_count * sizeof(pthread_t));
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);
    master_args->worker_args->server_socket_mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(master_args->worker_args->server_socket_mutex, NULL);
    master_args->worker_args->context = malloc(sizeof(struct gonm_context));
    GONC_HMAP_INIT(master_args->worker_args->context, 101);
    struct gonm_worker_args** worker_args_array = malloc(master_args->worker_count * sizeof(struct gonm_worker_args*));

    for(size_t index = 0; index != master_args->worker_count; ++index)
    {
        worker_args_array[index] = malloc(sizeof(struct gonm_worker_args));
        memcpy(worker_args_array[index], master_args->worker_args, sizeof(struct gonm_worker_args));
        if(pthread_create(threads + index, &thread_attr, gonm_worker_start, worker_args_array[index]) != 0)
            err(EXIT_FAILURE, "%s:%d", __FILE__, __LINE__);
    }

    for(size_t index = 0; index != master_args->worker_count; ++index)
    {
        pthread_join(threads[index], NULL);
        free(worker_args_array[index]);
    }

    free(worker_args_array);
    for(size_t index = 0; index != GONC_HMAP_CAPACITY(master_args->worker_args->context); ++index)
    {
        if(master_args->worker_args->context->gonc_hmap.elements[index] != NULL)
            free(GONC_HMAP_INDEX(master_args->worker_args->context, index));
    }
    GONC_HMAP_FREE_ELEMENTS(master_args->worker_args->context);
    free(master_args->worker_args->context);
    pthread_mutex_destroy(master_args->worker_args->server_socket_mutex);
    free(master_args->worker_args->server_socket_mutex);
    free(threads);
}