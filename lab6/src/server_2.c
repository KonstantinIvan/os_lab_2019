#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <getopt.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <pthread.h>
#include "MultModulo.h" 

struct FactorialArgs {
    uint64_t begin;
    uint64_t end;
    uint64_t mod;
};

void *ThreadFactorial(void *args) {
    struct FactorialArgs *fargs = (struct FactorialArgs *)args;
    uint64_t ans = 1;
    for (uint64_t i = fargs->begin; i <= fargs->end; i++) {
        ans = MultModulo(ans, i, fargs->mod);
    }
    return (void *)(uint64_t *)ans;
}

uint64_t compute_factorial(uint64_t begin, uint64_t end, uint64_t mod, int tnum) {
    pthread_t threads[tnum];
    struct FactorialArgs args[tnum];

    if (tnum > (end - begin)){
        tnum = (end - begin);
    }

    for (int i = 0; i < tnum; i++) {
        args[i].begin = 1 + begin + i * (end - begin) / tnum;
        args[i].end = begin + (i + 1) * (end - begin) / tnum;
        args[i].mod = mod;

        if (i == 0){
            args[i].begin = args[i].begin - 1;
        }

        //ёprintf("args[i].begin=%lu, args[i].end=%lu, args[i].mod=%lu\n", args[i].begin, args[i].end, args[i].mod);

        if (pthread_create(&threads[i], NULL, ThreadFactorial, (void *)&args[i])) {
            printf("Error: pthread_create failed!\n");
            return 1;
        }
    }

    uint64_t total = 1;
    for (int i = 0; i < tnum; i++) {
        uint64_t result;
        pthread_join(threads[i], (void **)&result);
        //printf("Result = %lu\n", result);
        total = MultModulo(total, result, mod);
    }

    return total;
}

int main(int argc, char **argv) {
    int tnum = -1;
    int port = -1;

    while (true) {
        int current_optind = optind ? optind : 1;
        static struct option options[] = {{"port", required_argument, 0, 0},
                                         {"tnum", required_argument, 0, 0},
                                         {0, 0, 0, 0}};
        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);
        if (c == -1)
            break;
        switch (c) {
        case 0: {
            switch (option_index) {
            case 0:
                port = atoi(optarg);
                break;
            case 1:
                tnum = atoi(optarg);
                break;
            default:
                printf("Index %d is out of options\n", option_index);
            }
        } break;
        case '?':
            printf("Unknown argument\n");
            break;
        default:
            fprintf(stderr, "getopt returned character code 0%o?\n", c);
        }
    }

    if (port == -1 || tnum == -1) {
        fprintf(stderr, "Using: %s --port 20001 --tnum 4\n", argv[0]);
        return 1;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        fprintf(stderr, "Can not create server socket!");
        return 1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons((uint16_t)port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt_val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

    int err = bind(server_fd, (struct sockaddr *)&server, sizeof(server));
    if (err < 0) {
        fprintf(stderr, "Can not bind to socket!");
        return 1;
    }

    err = listen(server_fd, 128);
    if (err < 0) {
        fprintf(stderr, "Could not listen on socket\n");
        return 1;
    }

    printf("Server listening at %d\n", port);

    while (true) {
        struct sockaddr_in client;
        socklen_t client_len = sizeof(client);
        int client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);

        if (client_fd < 0) {
            fprintf(stderr, "Could not establish new connection\n");
            continue;
        }

        while (true) {
            unsigned int buffer_size = sizeof(uint64_t) * 3;
            char from_client[buffer_size];
            int read = recv(client_fd, from_client, buffer_size, 0);

            if (!read)
                break;
            if (read < 0) {
                fprintf(stderr, "Client read failed\n");
                break;
            }
            if (read < buffer_size) {
                fprintf(stderr, "Client send wrong data format\n");
                break;
            }

            uint64_t begin, end, mod;
            memcpy(&begin, from_client, sizeof(uint64_t));
            memcpy(&end, from_client + sizeof(uint64_t), sizeof(uint64_t));
            memcpy(&mod, from_client + 2 * sizeof(uint64_t), sizeof(uint64_t));

            fprintf(stdout, "Receive: %lu %lu %lu\n", begin, end, mod);

            uint64_t result = compute_factorial(begin, end, mod, tnum);

            //printf("Result = %lu\n", result);

            char buffer[sizeof(result)];
            memcpy(buffer, &result, sizeof(result));
            err = send(client_fd, buffer, sizeof(result), 0);
            if (err < 0) {
                fprintf(stderr, "Can't send data to client\n");
                break;
            }
        }

        shutdown(client_fd, SHUT_RDWR);
        close(client_fd);
    }

    return 0;
}
