#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


void exit_current_thread(void *result)
{
    /* Cleanup current thread here. */
    pthread_exit(result);
}

void* start_routine(void *arg)
{
    /* Do something. */
    printf("start_routine\n");
    exit_current_thread(NULL);
}

int main()
{
    pthread_t thread = {0};
    pthread_attr_t attr = {0};
    void *thread_return = NULL;
    int result = 0;

    result = pthread_attr_init(&attr); /* Assigns default values. */
    if (result != 0)
    {
        printf("Failed to init thread attribute: %d\n", result);
        exit(EXIT_FAILURE);
    }

    result = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (result != 0)
    {
        printf("Failed to set detached attribute: %d\n", result);
        exit(EXIT_FAILURE);
    }

    result = pthread_create(&thread, &attr, start_routine, NULL);
    if (result != 0)
    {
        printf("Failed to create pthread: %d\n", result);
        exit(EXIT_FAILURE);
    }

    result = pthread_attr_destroy(&attr); /* No longer needed */
    if (result != 0)
    {
        printf("Failed to destroy attribute: %d\n", result);
        exit(EXIT_FAILURE);
    }

    usleep(100);
}