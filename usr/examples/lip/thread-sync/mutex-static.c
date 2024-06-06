#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

static int shared_var = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void *start_routine(void *arg)
{
    pthread_detach(pthread_self());

    pthread_mutex_lock(&mtx);

    printf("start_routine running!");
    shared_var += 1;

    pthread_mutex_unlock(&mtx);

    return NULL;
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

    pthread_mutex_lock(&mtx);
    printf("main running!");
    shared_var += 1;
    pthread_mutex_unlock(&mtx);

    usleep(100);

}