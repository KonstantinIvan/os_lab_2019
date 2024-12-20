#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include "find_min_max.h"
#include "utils.h"

#include <signal.h>

void write_results_to_file(const char *filename, struct MinMax min_max) {
    FILE *file = fopen(filename, "w");
    if (file) {
      fprintf(file, "%d %d\n", min_max.min, min_max.max);
      fclose(file);
    } else{
        perror("Error opening file"); 
    }
} 

int main(int argc, char **argv) {
    int seed = -1;
    int array_size = -1;
    int pnum = -1;
    bool with_files = false;
    int timeout = -1; // Таймаут по умолчанию

    while (true) {
        int current_optind = optind ? optind : 1;

        static struct option options[] = {
            {"seed", required_argument, 0, 0},
            {"array_size", required_argument, 0, 0},
            {"pnum", required_argument, 0, 0},
            {"by_files", no_argument, 0, 'f'},
            {"timeout", required_argument, 0, 0},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "f", options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0:
                        seed = atoi(optarg);
                        break;
                    case 1:
                        array_size = atoi(optarg);
                        break;
                    case 2:
                        pnum = atoi(optarg);
                        break;
                    case 3:
                        with_files = true;
                        break;
                    case 4:
                        timeout = atoi(optarg); 
                        break;

                    default:
                        printf("Index %d is out of options\n", option_index);
                }
                break;
            case 'f':
                with_files = true;
                break;

            case '?':
                break;

            default:
                printf("getopt returned character code 0%o?\n", c);
        }
    }

    if (optind < argc) {
        printf("Has at least one no option argument\n");
        return 1;
    }

    if (seed == -1 || array_size == -1 || pnum == -1) {
        printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n", argv[0]);
        return 1;
    }

    int *array = malloc(sizeof(int) * array_size);
    GenerateArray(array, array_size, seed);
    int active_child_processes = 0;

    int pipes[pnum][2]; // Массив для pipes
    pid_t pids[pnum]; // Массив для хранения PID дочерних процессов

    for (int i = 0; i < pnum; i++) {
        pipe(pipes[i]); // Создание pipe
    }

    struct timeval start_time;
    gettimeofday(&start_time, NULL);
    
    puts("Массив данных:");
    for (int i = 0; i < pnum; i++) {
        pid_t child_pid = fork();
        if (child_pid >= 0) {
            // Успешное создание процесса
            active_child_processes += 1;
            pids[i] = child_pid; // Сохраняем PID дочернего процесса
            if (child_pid == 0) {
                // Дочерний процесс
                int part_size = array_size / pnum;
                unsigned int start_index = i * part_size;
                unsigned int end_index = (i == pnum - 1) ? array_size : start_index + part_size;

                struct MinMax min_max = GetMinMax(array, start_index, end_index);

                if (with_files) {
                    char filename[20];
                    snprintf(filename, sizeof(filename), "result_%d.txt", i);
                    write_results_to_file(filename, min_max);
                } else {
                    // Отправка результатов через pipe
                    write(pipes[i][1], &min_max, sizeof(min_max));
                    close(pipes[i][1]);
                }
                exit(0);
            }
        } else {
            printf("Fork failed!\n");
            free(array);
            return 1;
        }
    }

    // Проверка времени выполнения
    if (timeout > 0) {
        //alarm(timeout);
        sleep(timeout);
        // Завершение всех дочерних процессов
        for (int i = 0; i < pnum; i++) {
            if (pids[i] > 0) {
                kill(pids[i], SIGKILL); // Убиваем только дочерние процессы
            }
        }
    } else {
        while (active_child_processes > 0) {
            wait(NULL);
            active_child_processes -= 1;
        }
    }

    struct MinMax final_min_max;
    final_min_max.min = INT_MAX;
    final_min_max.max = INT_MIN;

    for (int i = 0; i < pnum; i++) {
        if (with_files) {
            // Чтение из файлов
            char filename[20];
            snprintf(filename, sizeof(filename), "result_%d.txt", i);
            FILE *file = fopen(filename, "r");
            struct MinMax min_max;
            fscanf(file, "%d %d", &min_max.min, &min_max.max);
            fclose(file);

            if (min_max.min < final_min_max.min) final_min_max.min = min_max.min;
            if (min_max.max > final_min_max.max) final_min_max.max = min_max.max;
        } else {
            // Чтение из pipes
            struct MinMax min_max;
            read(pipes[i][0], &min_max, sizeof(min_max)); // Чтение из pipe

            if (min_max.min < final_min_max.min) final_min_max.min = min_max.min;
            if (min_max.max > final_min_max.max) final_min_max.max = min_max.max;
            close(pipes[i][0]); // Закрываем конец чтения
        }
    }

    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);

    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

    free(array);

    // Используем final_min_max для вывода
    printf("Min: %d\n", final_min_max.min);
    printf("Max: %d\n", final_min_max.max);
    printf("Elapsed time: %fms\n", elapsed_time);
    fflush(NULL);
    return 0;
}
