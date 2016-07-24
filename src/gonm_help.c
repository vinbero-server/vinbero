#include "gonm_help.h"
#include <stdio.h>

void gonm_help_print()
{
    printf("Usage: gonm [OPTION]...\n");
    printf("Modular TCP Server.\n");
    printf("Options:\n");
    printf("  -h --help                Print this help message.\n");
    printf("  -u --set-uid             Set User ID(string/number). default: current euid\n");
    printf("  -a --address             Set IP address. default: 0.0.0.0\n");
    printf("  -p --port                Set port number. default: 80\n");
    printf("  -b --backlog             Set backlog size. default: 1024\n");
    printf("  -r --reuse-port          Set reuse_port(0/1). default: 0\n");
    printf("  -w --worker-count        Set worker count. default: 4\n");
    printf("  -m --module-args         Set module arguments. first argument must be the path of module.\n");
}
