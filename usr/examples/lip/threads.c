#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void* start_routine(void *arg)
{
    printf("start_routine\n");
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