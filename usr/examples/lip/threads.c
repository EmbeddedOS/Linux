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
    printf("start_routine tid: %ld\n", pthread_self());
    exit_current_thread(NULL);
}

int main()
{
    pthread_t thread = {0};
    pthread_attr_t attr = {0};
    int result = 0;

    result = pthread_create(&thread, &attr, start_routine, NULL);
    if (result != 0)
    {
        printf("Failed to create pthread: %d\n", result);
        exit(EXIT_FAILURE);
    }

    usleep(100);
}