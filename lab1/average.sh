#!/bin/bash

if test "$#" -eq 0
then
    echo "Входные аргументы отсутствуют!"
    exit 1
fi

echo "Количество введённых аргументов: $#"

# Инициализация переменной для суммы
sum=0

# Цикл для суммирования всех аргументов
for arg in "$@"
do
    # Проверка, является ли аргумент числом с помощью expr
    if ! expr "$arg" + 0 &> /dev/null
    then
        echo "Ошибка: '$arg' не является числом."
        exit 1
    fi
    sum=$(echo "$sum + $arg" | bc)
done

average=$(echo "$sum / $#" | bc -l)

echo "Среднее арифметическое: $average"
