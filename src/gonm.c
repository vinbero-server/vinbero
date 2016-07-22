#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "gonm_parent.h"
#include "gonm_child.h"

#define CHILD_COUNT 3
int main(int argc, char* argv[])
{
    struct gonm_parent_args* parent_args = malloc(sizeof(struct gonm_parent_args));
    parent_args->child_count = CHILD_COUNT;
    parent_args->child_args = malloc(sizeof(struct gonm_child_args));
    parent_args->child_args->address = "0.0.0.0";
    parent_args->child_args->port = 8080;
    parent_args->child_args->backlog = 1024;

    gonm_parent_start(parent_args);

    return 0;
}
