#include <errno.h>
#include <fastdl.h>
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <libgenc/genc_Tree.h>
#include <libgenc/genc_ArrayList.h>
#include <vinbero/vinbero_interface_MODULE.h>
#include <vinbero/vinbero_interface_BASIC.h>
#include <vinbero_common/vinbero_common_Log.h>
#include <vinbero_common/vinbero_common_Call.h>
#include <vinbero_common/vinbero_common_Config.h>
#include <vinbero_common/vinbero_common_Module.h>
#include <vinbero_common/vinbero_common_Status.h>
#include <vinbero_common/vinbero_common_Object.h>
#include "vinbero_core.h"
#include "vinbero_Version.h"

static pthread_key_t vinbero_core_tlKey;

struct vinbero_core {
    uid_t setUid;
    gid_t setGid;
};

static void vinbero_core_sigIntHandler(int signal_number) {
    VINBERO_COMMON_LOG_TRACE2();
    exit(EXIT_FAILURE);
}

static void vinbero_core_exitHandler() {
    VINBERO_COMMON_LOG_TRACE2();
    if(syscall(SYS_gettid) == getpid()) {
        jmp_buf* jumpBuffer = pthread_getspecific(vinbero_core_tlKey);
        if(jumpBuffer != NULL)
            longjmp(*jumpBuffer, 1);
    }
}

int vinbero_core_registerSignalHandlers() {
    VINBERO_COMMON_LOG_TRACE2();
    struct sigaction signalAction;
    signalAction.sa_handler = vinbero_core_sigIntHandler;
    signalAction.sa_flags = SA_RESTART;
    if(sigfillset(&signalAction.sa_mask) == -1)
        return -errno;
    if(sigaction(SIGINT, &signalAction, NULL) == -1)
        return -errno;
    signalAction.sa_handler = SIG_IGN;
    signalAction.sa_flags = SA_RESTART;
    if(sigfillset(&signalAction.sa_mask) == -1)
        return -errno;
    if(sigaction(SIGPIPE, &signalAction, NULL) == -1)
        return -errno;
    return VINBERO_COMMON_STATUS_SUCCESS;
}

void vinbero_core_registerExitHandler() {
    atexit(vinbero_core_exitHandler);
}

int vinbero_core_checkConfig(struct vinbero_common_Config* config, const char* moduleId) {
    VINBERO_COMMON_LOG_TRACE2();
    int ret;
    if((ret = vinbero_common_Config_check(config, moduleId)) < VINBERO_COMMON_STATUS_SUCCESS) {
        VINBERO_COMMON_LOG_ERROR("Module %s has wrong config or doesn't exist", moduleId);
        return ret;
    }

    struct vinbero_common_Object* childModuleIds;
    if((ret = vinbero_common_Config_getChildModuleIds(config, moduleId, &childModuleIds)) < VINBERO_COMMON_STATUS_SUCCESS)
        return ret;

    GENC_TREE_NODE_FOR_EACH_CHILD(childModuleIds, index) {
        const struct vinbero_common_Object* childModuleId = GENC_TREE_NODE_GET_CHILD(childModuleIds, index);
        if((ret = vinbero_core_checkConfig(config, VINBERO_COMMON_OBJECT_CONSTRING(childModuleId))) < VINBERO_COMMON_STATUS_SUCCESS)
            return ret;
    }
    return VINBERO_COMMON_STATUS_SUCCESS;
}

int vinbero_core_initLocalModule(struct vinbero_common_Module* module, struct vinbero_common_Config* config) {
    VINBERO_COMMON_LOG_TRACE2();
    GENC_TREE_NODE_SET_PARENT(module, NULL);
    module->id = "core";
    module->config = config;
    vinbero_common_Module_init(module, "core", VINBERO_VERSION, true);
    module->localModule.pointer = malloc(1 * sizeof(struct vinbero_core));
    struct vinbero_core* localModule = module->localModule.pointer;
    vinbero_common_Config_getInt(module->config, module, "vinbero.setUid", (int*)&localModule->setUid, (int)geteuid());
    vinbero_common_Config_getInt(module->config, module, "vinbero.setGid", (int*)&localModule->setGid, (int)getegid());
    return VINBERO_COMMON_STATUS_SUCCESS;
}

static int vinbero_core_loadModule(struct vinbero_common_Module* module) {
    struct vinbero_common_Object* paths;
    VINBERO_COMMON_CONFIG_MGET_REQ(module->config, module, "paths", ARRAY, &paths);
    if(paths == NULL) {
        return VINBERO_COMMON_ERROR_INVALID_CONFIG;
    }
    if(GENC_TREE_NODE_CHILD_COUNT(paths) < 1) {
        return VINBERO_COMMON_ERROR_INVALID_CONFIG;
    }
    int ret = VINBERO_COMMON_STATUS_SUCCESS;
    GENC_TREE_NODE_FOR_EACH_CHILD(paths, index) {
        struct vinbero_common_Object* path = GENC_TREE_NODE_GET_CHILD(paths, index);
        if(!VINBERO_COMMON_OBJECT_IS_CONSTRING(path)) {
            return VINBERO_COMMON_ERROR_INVALID_CONFIG;
        }
        if(fastdl_open(&module->dlHandle, VINBERO_COMMON_OBJECT_CONSTRING(path), RTLD_LAZY | RTLD_GLOBAL) == 0) {
            module->path = VINBERO_COMMON_OBJECT_CONSTRING(path);
            return VINBERO_COMMON_STATUS_SUCCESS;
        }
        ret = VINBERO_COMMON_ERROR_DLOPEN;
    }
    return ret;
}

int vinbero_core_loadChildModules(struct vinbero_common_Module* module) {
    VINBERO_COMMON_LOG_TRACE2();
    int ret;
    struct vinbero_common_Object* childModuleIds;
    if((ret = vinbero_common_Config_getChildModuleIds(module->config, module->id, &childModuleIds)) < VINBERO_COMMON_STATUS_SUCCESS) {
        return ret;
    }

    GENC_TREE_NODE_FOR_EACH_CHILD(childModuleIds, index) {
        struct vinbero_common_Module* childModule = malloc(sizeof(struct vinbero_common_Module));
        GENC_TREE_NODE_INIT(childModule);
        GENC_TREE_NODE_ADD_CHILD(module, childModule);
        childModule->id = VINBERO_COMMON_OBJECT_CONSTRING(GENC_TREE_NODE_GET_CHILD(childModuleIds, index));
        childModule->config = module->config;
        ret = vinbero_core_loadModule(childModule);
        if(ret == VINBERO_COMMON_ERROR_INVALID_CONFIG) {
            VINBERO_COMMON_LOG_ERROR("Content of config file is invalid");
            return ret;
        }
        if(ret == VINBERO_COMMON_ERROR_DLOPEN) {
            VINBERO_COMMON_LOG_ERROR("%s", fastdl_error()); // dlerror is not thread safe
            return ret;
        }
        VINBERO_COMMON_LOG_DEBUG("Module %s is loaded from %s", childModule->id, childModule->path);
        if((ret = vinbero_core_loadChildModules(childModule)) < VINBERO_COMMON_STATUS_SUCCESS)
            return ret;
    }

    return VINBERO_COMMON_STATUS_SUCCESS;
}

int vinbero_core_initChildModules(struct vinbero_common_Module* module) {
    VINBERO_COMMON_LOG_TRACE2();
    int ret;
    GENC_TREE_NODE_FOR_EACH_CHILD(module, index) {
        struct vinbero_common_Module* childModule = GENC_TREE_NODE_GET_CHILD(module, index);
        childModule->childrenRequired = true;
        VINBERO_COMMON_CALL(MODULE, init, childModule, &ret, childModule);
        if(ret < VINBERO_COMMON_STATUS_SUCCESS)
            return ret;
        if(childModule->childrenRequired == true && GENC_TREE_NODE_CHILD_COUNT(childModule) == 0) {
            VINBERO_COMMON_LOG_ERROR("Module %s must have at least one child", childModule->name);
            return VINBERO_COMMON_ERROR_INVALID_MODULE;
        }
        if(childModule->name == NULL) {
            VINBERO_COMMON_LOG_ERROR("Module %s has no name", childModule->id);
            return VINBERO_COMMON_ERROR_INVALID_MODULE;
        }
        if(childModule->version == NULL) {
            VINBERO_COMMON_LOG_ERROR("Module %s has no version", childModule->id);
            return VINBERO_COMMON_ERROR_INVALID_MODULE;
        }
        if((ret = vinbero_core_initChildModules(childModule)) < 0)
            return ret;
    }
    return VINBERO_COMMON_STATUS_SUCCESS;
}

int vinbero_core_rInitChildModules(struct vinbero_common_Module* module) {
    VINBERO_COMMON_LOG_TRACE2();
    int ret;
    GENC_TREE_NODE_FOR_EACH_CHILD(module, index) {
        struct vinbero_common_Module* childModule = GENC_TREE_NODE_GET_CHILD(module, index);
        if((ret = vinbero_core_rInitChildModules(childModule)) < 0)
            return ret;

        VINBERO_COMMON_CALL(MODULE, rInit, childModule, &ret, childModule);
        if(ret < VINBERO_COMMON_STATUS_SUCCESS)
            return ret;
    }
    return VINBERO_COMMON_STATUS_SUCCESS;
}

static int vinbero_core_destroyChildModules(struct vinbero_common_Module* module) {
    VINBERO_COMMON_LOG_TRACE2();
    int ret;
    GENC_TREE_NODE_FOR_EACH_CHILD(module, index) {
        struct vinbero_common_Module* childModule = GENC_TREE_NODE_GET_CHILD(module, index);
        VINBERO_COMMON_CALL(MODULE, destroy, childModule, &ret, childModule);
        if(ret < VINBERO_COMMON_STATUS_SUCCESS)
            return ret;
        if((ret = vinbero_core_destroyChildModules(childModule)) < 0)
            return ret;
    }
    return VINBERO_COMMON_STATUS_SUCCESS;
}

static int vinbero_core_rDestroyChildModules(struct vinbero_common_Module* module) {
    VINBERO_COMMON_LOG_TRACE2();
    int ret;
    GENC_TREE_NODE_FOR_EACH_CHILD(module, index) {
        struct vinbero_common_Module* childModule = GENC_TREE_NODE_GET_CHILD(module, index);
        if((ret = vinbero_core_rDestroyChildModules(childModule)) < 0)
            return ret;
        VINBERO_COMMON_CALL(MODULE, rDestroy, childModule, &ret, childModule);
        if(ret < VINBERO_COMMON_STATUS_SUCCESS)
            return ret;
        GENC_TREE_NODE_FREE(childModule);
        free(childModule);
    }
    return VINBERO_COMMON_STATUS_SUCCESS;
}

int vinbero_core_setGid(struct vinbero_common_Module* module) {
    VINBERO_COMMON_LOG_TRACE2();
    struct vinbero_core* localModule = module->localModule.pointer;
    if(setgid(localModule->setGid) < 0) {
        VINBERO_COMMON_LOG_ERROR("setgid(...) failed");
        return -errno;
    }
    return VINBERO_COMMON_STATUS_SUCCESS;
}

int vinbero_core_setUid(struct vinbero_common_Module* module) {
    VINBERO_COMMON_LOG_TRACE2();
    struct vinbero_core* localModule = module->localModule.pointer;
    if(setgid(localModule->setUid) < 0) {
        VINBERO_COMMON_LOG_ERROR("setuid(...) failed");
        return -errno;
    }
    return VINBERO_COMMON_STATUS_SUCCESS;
}

int vinbero_core_start(struct vinbero_common_Module* module) {
    VINBERO_COMMON_LOG_TRACE2();
    int ret;
    jmp_buf* jumpBuffer = malloc(1 * sizeof(jmp_buf));
    if(setjmp(*jumpBuffer) == 0) {
        VINBERO_COMMON_LOG_TRACE2();
        pthread_key_create(&vinbero_core_tlKey, NULL);
        pthread_setspecific(vinbero_core_tlKey, jumpBuffer);
        GENC_TREE_NODE_FOR_EACH_CHILD(module, index) {
            struct vinbero_common_Module* childModule = GENC_TREE_NODE_GET_CHILD(module, index);
            VINBERO_COMMON_CALL(BASIC, service, childModule, &ret, childModule);
            if(ret < VINBERO_COMMON_STATUS_SUCCESS) {
                VINBERO_COMMON_LOG_ERROR("vinbero_interface_BASIC_service() failed");
                break;
            }
        }
    }
    VINBERO_COMMON_LOG_TRACE2();
    free(jumpBuffer);
    pthread_setspecific(vinbero_core_tlKey, NULL);
    pthread_key_delete(vinbero_core_tlKey);
    vinbero_core_destroyChildModules(module);
    vinbero_core_rDestroyChildModules(module);
    return VINBERO_COMMON_STATUS_SUCCESS;
}
