#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
    int ticks = uptime();   // call uptime system call
    printf("Uptime: %d ticks\n", ticks);
    exit(0);
}
