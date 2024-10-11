#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> // Добавлено для waitpid

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s seed arraysize\n", argv[0]);
        return 1;
    }

    pid_t pid = fork(); // Создание нового процесса

    if (pid < 0) {
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        // Это дочерний процесс
        execlp("./sequential_min_max", "sequential_min_max", argv[1], argv[2], (char *)NULL);
        perror("execlp failed"); // Если execlp возвращает, значит произошла ошибка
        return 1;
    } else {
        // Это родительский процесс
        int status;
        waitpid(pid, &status, 0); // Ожидание завершения дочернего процесса
        if (WIFEXITED(status)) {
            printf("Child exited with status %d\n", WEXITSTATUS(status));
        }
    }

    return 0;
}
