#include <err.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <libgonc/gonc_hmap.h>
#include "gonm_worker.h"
#include "gonm_context.h"

void* gonm_worker_start(void* worker_args)
{
    for(int client_socket;;)
    {
        pthread_mutex_lock(((struct gonm_worker_args*)worker_args)->server_socket_mutex);
        if((client_socket = accept(((struct gonm_worker_args*)worker_args)->server_socket, NULL, NULL)) == -1)
        {
            pthread_mutex_unlock(((struct gonm_worker_args*)worker_args)->server_socket_mutex);
            warn("%s:%d", __FILE__, __LINE__);
            continue;
        }
        pthread_mutex_unlock(((struct gonm_worker_args*)worker_args)->server_socket_mutex);

        write(client_socket, "HTTP/1.1 200 OK\r\nServer: gonm\r\nContent-Length: 5\r\nContent-Type: text/plain\r\n\r\nHELLO\r\n", sizeof("HTTP/1.1 200 OK\r\nServer: gonm\r\nContent-Length: 5\r\nContent-Type: text/plain\r\n\r\nHELLO\r\n"));        
//        struct gonm_module_object_list* module_object_list = gonm_module_init(((struct gonm_worker_args*)worker_args)->context, ((struct gonm_worker_args*)worker_args)->module_args_list);
//        gonm_module_service(((struct gonm_worker_args*)worker_args)->context, GONC_LIST_HEAD(module_object_list), client_socket);
//        GONM_MODULE_OBJECT_LIST_DESTROY(module_object_list);
        
        if(close(client_socket) == -1)
            warn("%s:%d", __FILE__, __LINE__);
    }
    return NULL;
}
