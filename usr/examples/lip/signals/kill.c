#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    pid_t pid = atoi(argv[1]);
    int sig = atoi(argv[2]);

    printf("Send sig %d to %d result: %d\n", sig, pid, kill(pid, sig));
}