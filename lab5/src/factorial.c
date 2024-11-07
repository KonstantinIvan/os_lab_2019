#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>

int factmod(int n, int p);
void *factorial_foo(void *);
void show_end_result(int k, int result);
int result_factorial = 1; 
int mod = 0;
int pnum = 0;
int k = 0;
int k_original = 0;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int *result_factorial;
    int *k;
} factorial_args_t;

int main(int argc, char **argv) {
    int c;
    while (argc > 1) {
        if (strcmp(argv[1], "-k") == 0 && argc > 2) {
            k = atoi(argv[2]);
            k_original = k;
            argv += 2; argc -= 2;
        } else if (strcmp(argv[1], "--pnum") == 0 && argc > 2) {
            pnum = atoi(argv[2]);
            argv += 2; argc -= 2;
        } else if (strcmp(argv[1], "--mod") == 0 && argc > 2) {
            mod = atoi(argv[2]);
            argv += 2; argc -= 2;
        } else {
            fprintf(stderr, "Usage: %s -k <k> --pnum <pnum> --mod <mod>\n", argv[0]);
            return 1;
        }
    }

    if (k <= 0 || pnum <= 0 || mod <= 0) {
        fprintf(stderr, "Invalid arguments\n");
        return 1;
    }

    factorial_args_t args[pnum];
    pthread_t threads[pnum];
    for (int i = 0; i < pnum; i++) {
        args[i].result_factorial = &result_factorial;
        args[i].k = &k;
        if (pthread_create(&threads[i], NULL, factorial_foo, &args[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    for (int i = 0; i < pnum; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            return 1;
        }
    }

    show_end_result(k_original, result_factorial % mod);
    return 0;
}

void *factorial_foo(void *arg) {
    factorial_args_t *args = (factorial_args_t *)arg;
    int *result_factorial = args->result_factorial;
    int *k = args->k;

    pthread_mutex_lock(&mut);

    *result_factorial = (*result_factorial * ((*k / mod) % 2 ? mod - 1 : 1)) % mod;
    for (int i = 2; i <= *k % mod; ++i)
        *result_factorial = (*result_factorial * i) % mod;
    *k /= mod;

    pthread_mutex_unlock(&mut);

    return NULL;
}

void show_end_result(int k, int result) {
    printf("%d! mod %d = %d\n", k, mod, result);
}
