#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <pthread.h>
#include "MultModulo.h" 

struct Server {
    char ip[255];
    int port;
};

uint64_t k = -1;
uint64_t mod = -1;
int server_count = 0;

bool ConvertStringToUI64(const char *str, uint64_t *val) {
    char *end = NULL;
    unsigned long long i = strtoull(str, &end, 10);
    if (errno == ERANGE) {
        fprintf(stderr, "Out of uint64_t range: %s\n", str);
        return false;
    }
    if (errno != 0)
        return false;
    *val = i;
    return true;
}

void *client_thread(void *arg) {
    struct Server *server = (struct Server *)arg;
    static int thread_index = 0; // Статическая переменная для индекса потока
    int current_index = __sync_fetch_and_add(&thread_index, 1);

    int sck = socket(AF_INET, SOCK_STREAM, 0);
    if (sck < 0) {
        fprintf(stderr, "Socket creation failed!\n");
        return NULL;
    }

    struct hostent *hostname = gethostbyname(server->ip);
    if (hostname == NULL) {
        fprintf(stderr, "gethostbyname failed with %s\n", server->ip);
        close(sck);
        return NULL;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server->port);
    server_addr.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

    if (connect(sck, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Connection failed\n");
        close(sck);
        return NULL;
    }

    uint64_t begin = current_index * (k / server_count) + 1;
    uint64_t end = (current_index+1) * (k / server_count);
    if (current_index  == server_count - 1) {
        end = k;
    }

    char task[sizeof(uint64_t) * 3];
    memcpy(task, &begin, sizeof(uint64_t));
    memcpy(task + sizeof(uint64_t), &end, sizeof(uint64_t));
    memcpy(task + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));

    if (send(sck, task, sizeof(task), 0) < 0) {
        fprintf(stderr, "Send failed\n");
        close(sck);
        return NULL;
    }

    char response[sizeof(uint64_t)];
    if (recv(sck, response, sizeof(response), 0) < 0) {
        fprintf(stderr, "Receive failed\n");
        close(sck);
        return NULL;
    }

    uint64_t result;
    memcpy(&result, response, sizeof(uint64_t));

    close(sck);
    return (void *)(uint64_t *)result;
}

int main(int argc, char **argv) {
    char servers[255] = {'\0'};

    while (true) {
        int current_optind = optind ? optind : 1;
        static struct option options[] = {{"k", required_argument, 0, 0},
                                         {"mod", required_argument, 0, 0},
                                         {"servers", required_argument, 0, 0},
                                         {0, 0, 0, 0}};
        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);
        if (c == -1)
            break;
        switch (c) {
        case 0: {
            switch (option_index) {
            case 0:
                ConvertStringToUI64(optarg, &k);
                break;
            case 1:
                ConvertStringToUI64(optarg, &mod);
                break;
            case 2:
                memcpy(servers, optarg, strlen(optarg));
                break;
            default:
                printf("Index %d is out of options\n", option_index);
            }
        } break;
        case '?':
            printf("Arguments error\n");
            break;
        default:
            fprintf(stderr, "getopt returned character code 0%o?\n", c);
        }
    }

    if (k == -1 || mod == -1 || !strlen(servers)) {
        fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
                argv[0]);
        return 1;
    }

    FILE *fp = fopen(servers, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open servers file: %s\n", servers);
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);

    char *line = malloc(file_size + 1);
    if (line == NULL) {
        fprintf(stderr, "Failed to allocate memory for line\n");
        fclose(fp);
        return 1;
    }

    struct Server *to = malloc(sizeof(struct Server) * 256);

    while (fgets(line, file_size + 1, fp)) {
        char *ip, *port_str;
        ip = strtok(line, ":");
        port_str = strtok(NULL, ":");
        if (ip == NULL || port_str == NULL) {
            fprintf(stderr, "Invalid server format: %s\n", line);
            continue;
        }
        to[server_count].port = atoi(port_str);
        strncpy(to[server_count].ip, ip, sizeof(to[server_count].ip) - 1);
        server_count++;
    }

    free(line);
    fclose(fp);


    pthread_t *threads = malloc(sizeof(pthread_t) * server_count);
    for (int i = 0; i < server_count; i++) {
        if (pthread_create(&threads[i], NULL, client_thread, &to[i])) {
            fprintf(stderr, "Error: pthread_create failed!\n");
            return 1;
        }
    }

    uint64_t answer = 1;
    for (int i = 0; i < server_count; i++) {
        uint64_t result;
        pthread_join(threads[i], (void **)&result);
        //printf("Result = %lu\n", result);
        answer = MultModulo(answer, result, mod);
    }
    printf("answer: %lu\n", answer);

    free(to);
    free(threads);
    return 0;
}
