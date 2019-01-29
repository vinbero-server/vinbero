#ifndef _VINBERO_CORE_H
#define _VINBERO_CORE_H

#include <vinbero_common/vinbero_common_Module.h>
#include <vinbero_common/vinbero_common_Config.h>

int vinbero_core_registerSignalHandlers();
void vinbero_core_registerExitHandler();
int vinbero_core_checkConfig(struct vinbero_common_Config* config, const char* moduleId);
int vinbero_core_initLocalModule(struct vinbero_common_Module* module, struct vinbero_common_Config* config);
int vinbero_core_loadChildModules(struct vinbero_common_Module* module);
int vinbero_core_initChildModules(struct vinbero_common_Module* module);
int vinbero_core_rInitChildModules(struct vinbero_common_Module* module);
int vinbero_core_start(struct vinbero_common_Module* module);
int vinbero_core_setGid(struct vinbero_common_Module* module);
int vinbero_core_setUid(struct vinbero_common_Module* module);

#endif
