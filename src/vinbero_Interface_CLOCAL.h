#ifndef _VINBERO_INTERFACE_CLOCAL_H
#define _VINBERO_INTERFACE_CLOCAL_H

#include <vinbero_common/vinbero_common_Module.h>
#include <vinbero_common/vinbero_common_ClData.h>

#define VINBERO_INTERFACE_CLOCAL_FUNCTIONS \
int vinbero_Interface_CLOCAL_init(struct vinbero_common_Module* module, struct vinbero_common_ClData* clData, void* args[]); \
int vinbero_Interface_CLOCAL_destroy(struct vinbero_common_Module* module, struct vinbero_common_ClData* clData)

#define VINBERO_INTERFACE_CLOCAL_FUNCTION_POINTERS \
int (*vinbero_Interface_CLOCAL_init)(struct vinbero_common_Module*, struct vinbero_common_ClData*, void*[]); \
int (*vinbero_Interface_CLOCAL_destroy)(struct vinbero_common_Module*, struct vinbero_common_ClData*)

struct vinbero_Interface_CLOCAL {
    VINBERO_INTERFACE_CLOCAL_FUNCTION_POINTERS;
};

#define VINBERO_INTERFACE_CLOCAL_DLSYM(interface, dlHandle, ret) do { \
    VINBERO_COMMON_MODULE_DLSYM(interface, dlHandle, vinbero_Interface_CLOCAL_init, ret); \
    if(*ret < 0) break; \
    VINBERO_COMMON_MODULE_DLSYM(interface, dlHandle, vinbero_Interface_CLOCAL_destroy, ret); \
    if(*ret < 0) break; \
} while(0)

#endif
