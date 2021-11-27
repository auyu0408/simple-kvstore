#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <netdb.h>
#include <ctype.h>

#include <signal.h>
#include "types.h"
#include "sock.h"

pthread_rwlock_t rw_lock = PTHREAD_RWLOCK_INITIALIZER;//to prevent race condition

/*
thread_server()
get socket server input

argument:
parm: listenfd in socket

return:
if get "EXIT", leave process
*/
void *thread_server(void* parm)
{
    pthread_detach(pthread_self());
    int fd = (int) parm;
    char str[50];
    while(scanf("%[^\n]%*c", str))
    {
        if(strcmp(str,"EXIT") == 0)
            break;
    }
    shutdown(fd, SHUT_RDWR);
    close(fd);
    exit(0);
}

/*
thread_routine()
thread routine after thread create

argument:
param: fd, used for socket communication

return:
pthread_exit(0): exit thread in disjoin status
*/
void *thread_routine(void* param)
{
    int fd = *(int *) param;//for socket communication
    char recvbuf[300];//receieve msg from socket
    char sendbuf[300];//save msg would be send
    char *com_temp;//command temporary
    char *key_temp;//key temporary
    char *val_temp;//value temporary
    char val[105];//value temp use at "GET"
    char* d = " ";//split string
    FILE* file;//file pointer
    char filename[105];//filename temporary

    printf("[THREAD INFO] Thread %ld created, serving connection fd %d.\n", pthread_self(), fd);//print thread info
    pthread_detach(pthread_self());//make thread disjoint
    //start receieve command
    while(recv(fd, recvbuf, 300, 0))
    {
        //separate command, key for all recv
        com_temp = strtok(recvbuf, d);
        key_temp = strtok(NULL, d);
        sprintf(filename, "%s.txt", key_temp);//create filename
        /*
        compare command and execute it

        SET: get value and build data
        GET: open data and send value
        DELETE: delete data
        EXIT: leave the loop and exit thread
        */
        if(strcmp(com_temp, "SET") == 0)
        {
            val_temp = strtok(NULL, d);
            //when a thread write, other thread can't read and write
            pthread_rwlock_wrlock(&rw_lock);
            if(access(filename, 0) == 0)
            {
                //prepare error message
                pthread_rwlock_unlock(&rw_lock);
                sprintf(sendbuf, "Key is already exist!");
            }
            else
            {
                //build file and prepare send message
                file = fopen(filename, "w");
                fprintf(file, "%s", val_temp);
                fclose(file);
                pthread_rwlock_unlock(&rw_lock);
                sprintf(sendbuf, "[OK] Key value pair (%s, %s) is stored!", key_temp, val_temp);
            }
            send(fd, sendbuf, strlen(sendbuf)+1, 0);//send message
        }
        else if(strcmp(com_temp, "GET") == 0)
        {
            pthread_rwlock_rdlock(&rw_lock);
            if(access(filename, 0) == 0)
            {
                //open file to get value and prepare send msg
                file = fopen(filename, "r");
                fread(val, 101, 1, file);
                fclose(file);
                pthread_rwlock_unlock(&rw_lock);
                sprintf(sendbuf, "[OK] The value of %s is %s", key_temp, val);
            }
            else
            {
                //prepare error msg
                pthread_rwlock_unlock(&rw_lock);
                sprintf(sendbuf, "Can't find the key!");
            }
            send(fd, sendbuf, strlen(sendbuf)+1, 0);
        }
        else if(strcmp(com_temp, "DELETE") == 0)
        {
            pthread_rwlock_wrlock(&rw_lock);
            if(access(filename, 0) == 0)
            {
                //remove file and prepare msg
                remove(filename);
                pthread_rwlock_unlock(&rw_lock);
                sprintf(sendbuf,"[OK] Key \"%s\" is removed!", key_temp);
            }
            else
            {
                //prepare msg
                sprintf(sendbuf, "Can't find key!");
                pthread_rwlock_unlock(&rw_lock);
            }
            send(fd, sendbuf, strlen(sendbuf)+1, 0);
        }
        else if(strcmp(com_temp, "EXIT") == 0)
            break;
        else if(strcmp(com_temp, "KEEP") == 0)
            continue;
    }
    //shutdown(fd, SHUT_RDWR);
    //close(fd);
    pthread_exit(0);
}

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
    int* connfd;//connect fd for socket communication
    pthread_t tid;//thread id to create
    char recv_buf[50];//recv client pid

    //print server info
    printf("[INFO] Start with a database...\n");
    printf("[INFO] Initializing the server...\n");
    printf("[INFO] Server Initialized!\n");

    pthread_create(&tid, NULL, thread_server, (void *) listenfd);

    //accept client
    while(1)
    {
        connfd = malloc(sizeof(int));//each tread have different connfd
        //print already msg
        printf("[INFO] Listening on the port %s...\n", server_port);
        //if connection failed, print error
        if((*connfd = accept(listenfd, NULL, NULL)) == -1)
        {
            printf("accept socket error");
            free(connfd);
            continue;
        }
        //connection success, print client pid and create thread
        recv(*connfd, recv_buf, 50, 0);
        printf("[CLIENT CONNECTED] Connected to client (localhost, %s)\n", recv_buf);
        pthread_create(&tid, NULL, thread_routine, (void *)connfd);
    }
    close(listenfd);
    return 0;
}