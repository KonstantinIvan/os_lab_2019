#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void* thread_func1(void* arg); 
void* thread_func2(void* arg); 

void* thread_func1(void* arg) { 
    pthread_mutex_lock(&mutex1);
    printf("Thread 1: Держит мьютекс 1... \n");
    
    sleep(1);
    
    printf("Thread 1: Ожидает мьютекс 2...\n");
    pthread_mutex_lock(&mutex2);
    printf("Thread 1: Получен мьютекс 2!\n");
    
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    
    return NULL;
}

void* thread_func2(void* arg) { 
    pthread_mutex_lock(&mutex2);
    printf("Thread 2: Держит мьютекс 2...\n");
    
    sleep(1);
    
    printf("Thread 2: Ожидает мьютекс 1...\n");
    pthread_mutex_lock(&mutex1);
    printf("Thread 2: Получен мьютекс 1!\n");
    
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);
    
    return NULL; 
}

int main() {
    pthread_t thread1, thread2;

    if (pthread_create(&thread1, NULL, thread_func1, NULL) != 0) {
        perror("pthread_create");
        exit(1);
    }

    if (pthread_create(&thread2, NULL, thread_func2, NULL) != 0) {
        perror("pthread_create");
        exit(1);
    }

    if (pthread_join(thread1, NULL) != 0) {
        perror("pthread_join");
        exit(1);
    }

    if (pthread_join(thread2, NULL) != 0) {
        perror("pthread_join");
        exit(1);
    }

    return 0;
}
