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
    exit_current_thread("I'm quit.");
}

int main()
{
    pthread_t thread = {0};
    pthread_attr_t attr = {0};
    void *thread_return = NULL;
    int result = 0;

    result = pthread_create(&thread, &attr, start_routine, NULL);
    if (result != 0)
    {
        printf("Failed to create pthread: %d\n", result);
        exit(EXIT_FAILURE);
    }

    pthread_join(thread, &thread_return);
    printf("Thread return: %s\n", (char *)thread_return);
}