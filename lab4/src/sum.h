#ifndef SUM_H
#define SUM_H

#include <stdint.h>

struct SumArgs {
    int *array;       // Указатель на массив
    int begin;       // Начальный индекс
    int end;         // Конечный индекс
};

unsigned long long int Sum(struct SumArgs *args);

#endif // SUM_H
