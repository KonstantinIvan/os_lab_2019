#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid;

    for (int i = 0; i < 5; i++) {
        pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        } else if (pid == 0) {
            printf("Дочерний процесс %d с PID %d\n", i + 1, getpid());
            sleep(1); 
            exit(0); 
        }
    }

    printf("Родительский процесс с PID %d\n", getpid());

    sleep(1);

    printf("Теперь в системе должны быть зомби-процессы.\n");
    printf("Нажмите Enter, чтобы завершить родительский процесс и очистить зомби.\n");
    getchar();

    printf("Родительский процесс завершен.\n");
    return 0;
}
