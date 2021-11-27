#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <netdb.h>
#include <ctype.h>

#include "types.h"
#include "sock.h"

void *thread_routine(void*);
pthread_rwlock_t rw_lock = PTHREAD_RWLOCK_INITIALIZER;

int main(int argc, char **argv)
{
    char *server_port = 0;
    int opt = 0;
    /* Parsing args */
    while ((opt = getopt(argc, argv, "p:")) != -1)
    {
        switch (opt)
        {
        case 'p':
            server_port = malloc(strlen(optarg) + 1);
            strncpy(server_port, optarg, strlen(optarg));
            break;
        case '?':
            fprintf(stderr, "Unknown option \"-%c\"\n", isprint(optopt) ?
                    optopt : '#');
            return 0;
        }
    }

    if (!server_port)
    {
        fprintf(stderr, "Error! No port number provided!\n");
        exit(1);
    }

    /* Open a listen socket fd */
    int listenfd __attribute__((unused)) = open_listenfd(server_port);
    /* Start coding your server code here! */
    int* connfd;
    pthread_t tid;
    char recv_buf[100];
    printf("[INFO] Start with a clean database...\n");
    printf("[INFO] Initializing the server...\n");
    printf("[INFO] Server Initialized!\n");
    while(1)
    {
        connfd = malloc(sizeof(int));
        printf("[INFO] Listening on the port %s...\n", server_port);
        if((*connfd = accept(listenfd, NULL, NULL)) == -1)
        {
            printf("accept socket error");//print error
            free(connfd);
            continue;
        }
        //print connected
        recv(*connfd, recv_buf, 100, 0);
        printf("[CLIENT CONNECTED] Connected to client (localhost, %s)\n", recv_buf);
        pthread_create(&tid, NULL, thread_routine, (void *)connfd);
    }
    close(listenfd);
    return 0;
}

void *thread_routine(void* param)
{
    int fd = *(int *) param;
    printf("[THREAD INFO] Thread %ld created, serving connection fd %d.\n", pthread_self(), fd);
    char recvbuf[300];
    char sendbuf[300];
    char *com_temp;
    char *key_temp;
    char *val_temp;
    char* d = " ";
    int n=0;
    FILE* file;
    pthread_detach(pthread_self());
    while(1)
    {
        n = recv(fd, recvbuf, 300, 0);
        if(n > 0)
        {
            com_temp = strtok(recvbuf, d);
            if(strcmp(com_temp, "SET") == 0)
            {
                key_temp = strtok(NULL, d);
                val_temp = strtok(NULL, d);
                pthread_rwlock_wrlock(&rw_lock);
                if(access(key_temp, 0) == 0)
                {
                    sprintf(sendbuf, "Key is already exist!");
                    pthread_rwlock_unlock(&rw_lock);
                }
                else
                {
                    file = fopen(key_temp, "w");
                    fwrite(val_temp, 1, sizeof(val_temp), file);
                    fclose(file);
                    pthread_rwlock_unlock(&rw_lock);
                    sprintf(sendbuf, "[OK] Key value pair (%s, %s) is stored!", key_temp, val_temp);
                }
                send(fd, sendbuf, strlen(sendbuf)+1, 0);
            }
            else if(strcmp(com_temp, "GET") == 0)
            {
                key_temp = strtok(NULL, d);
                pthread_rwlock_rdlock(&rw_lock);
                if(access(key_temp, 0) == 0)
                {
                    file = fopen(key_temp, "r");
                    fread(val_temp, 101, sizeof(char), file);
                    fclose(file);
                    pthread_rwlock_unlock(&rw_lock);
                    sprintf(sendbuf, "[OK] The value of %s is %s", key_temp, val_temp);
                }
                else
                {
                    pthread_rwlock_unlock(&rw_lock);
                    sprintf(sendbuf, "Can't find the key!");
                }
                send(fd, sendbuf, strlen(sendbuf)+1, 0);
            }
            else if(strcmp(com_temp, "DELETE") == 0)
            {
                key_temp = strtok(NULL, d);
                pthread_rwlock_wrlock(&rw_lock);
                if(access(key_temp, 0) == 0)
                {
                    remove(key_temp);
                    pthread_rwlock_unlock(&rw_lock);
                    sprintf(sendbuf,"[OK] Key \"%s\" is removed!", key_temp);
                }
                else
                {
                    sprintf(sendbuf, "Can't find key!");
                    pthread_rwlock_unlock(&rw_lock);
                }
                send(fd, sendbuf, strlen(sendbuf)+1, 0);
            }
            else if(strcmp(com_temp, "EXIT") == 0)
                break;
        }
    }
    shutdown(fd, SHUT_RDWR);
    close(fd);
    pthread_exit(0);
}