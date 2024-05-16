#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

volatile bool received_sigint = false;

void sigint_handler(int sig)
{
    received_sigint = true;
    printf("SIGINT\n");
}

int main()
{
    if (signal(SIGINT, sigint_handler) == SIG_ERR)
    {
        printf("Failed to disposition SIGINT!\n");
        exit(EXIT_FAILURE);
    }

    printf("PID: %d\n", getpid());
    printf("SIGINT information: %s\n", strsignal(SIGINT));
    printf("SIGTERM information: %s\n", strsignal(SIGTERM));

    /* Wait for sig interrupt from outside. */
    while (!received_sigint);

    /* raise a terminate signal by itself. */
    raise(SIGTERM);
}