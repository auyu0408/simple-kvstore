#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdlib.h>

#include "sock.h"

//print helping message
void help_p()
{
    printf("Commands\t\tDescription\n");
    printf("SET [key] [value]\tStore the key value pair ([key], [value]) into the database.\n");
    printf("GET [key] [value]\tGet value of [key] from the database.\n");
    printf("DELETE [key]\tDelete [key] and it's associated value from the database.\n");
    printf("Exit\t\t\tExit.\n");
    printf("> ");
    return;
}

//print wrong message
void wrong_p()
{
    printf("Unkown command\n");
    printf("> ");
    return;
}

int main(int argc, char **argv)
{
    int opt;
    char *server_host_name = NULL, *server_port = NULL;

    /* Parsing args */
    while ((opt = getopt(argc, argv, "h:p:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            server_host_name = malloc(strlen(optarg) + 1);
            strncpy(server_host_name, optarg, strlen(optarg));
            break;
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

    if (!server_host_name)
    {
        fprintf(stderr, "Error!, No host name provided!\n");
        exit(1);
    }

    if (!server_port)
    {
        fprintf(stderr, "Error!, No port number provided!\n");
        exit(1);
    }

    /* Open a client socket fd */
    int clientfd __attribute__((unused)) = open_clientfd(server_host_name, server_port);

    /* Start your coding client code here! */
    char str_temp[300];//temporary to save input
    char str_cp[300];//temporary to copy input since input would be edit by strtok
    char* d = " ";//split with space
    char *com_temp;//temporary to save command
    char *key_temp;//temporary to save key
    char *val_temp;//temporary to save value
    char *left;//temporary to save left thing, should be NULL
    char recv_buf[300];//temporary to put server message
    /*print starting message*/
    pid_t pid = getpid();
    sprintf(str_cp, "%u", pid);
    send(clientfd, str_cp, strlen(str_cp)+1, 0);
    printf("[INFO] Connected to %s:%s\n", server_host_name, server_port);
    printf("[INFO] Welcome! Please type HELP for avaliable commands.\n");
    printf(">");
    /*main loop*/
    while(1)
    {
        scanf("%[^\n]%*c", str_temp);//get input
        strcpy(str_cp, str_temp);
        com_temp = strtok(str_temp, d);
        /*
        compare which command we want
        HELP: print HELP message
        SET, GET, DELETE: send to server and print str we receieve.
        EXIT: break the loop
        None of above: print wrong message
        */
        if(strcmp(com_temp, "HELP")==0)
        {
            left = strtok(NULL, d);
            if(left != NULL)
                wrong_p();
            else
                help_p();

        }
        else if(strcmp(com_temp, "SET") == 0)
        {
            key_temp = strtok(NULL, d);
            if(key_temp == NULL)
                wrong_p();
            else
            {
                val_temp = strtok(NULL, d);
                if(val_temp == NULL)
                    wrong_p();
                else
                {
                    left = strtok(NULL, d);
                    if(left != NULL)
                        wrong_p();
                    else
                    {
                        send(clientfd, str_cp, strlen(str_cp)+1, 0);
                        recv(clientfd, recv_buf, 300, 0);
                        puts(recv_buf);
                        printf(">");
                    }
                }
            }
        }
        else if(strcmp(com_temp, "GET") == 0)
        {
            key_temp = strtok(NULL, d);
            if(key_temp == NULL)
                wrong_p();
            else
            {
                left = strtok(NULL, d);
                if(left != NULL)
                    wrong_p();
                else
                {
                    send(clientfd, str_cp, strlen(str_cp)+1, 0);
                    recv(clientfd, recv_buf, 300, 0);
                    puts(recv_buf);
                    printf(">");
                }
            }
        }
        else if(strcmp(com_temp, "DELETE") == 0)
        {
            key_temp = strtok(NULL, d);
            if(key_temp == NULL)
                wrong_p();
            else
            {
                left = strtok(NULL, d);
                if(left != NULL)
                    wrong_p();
                else
                {
                    send(clientfd, str_cp, strlen(str_cp)+1, 0);
                    recv(clientfd, recv_buf, 300, 0);
                    puts(recv_buf);
                    printf(">");
                }
            }
        }
        else if(strcmp(com_temp, "EXIT"))
        {
            key_temp = strtok(NULL,d);
            if(key_temp != NULL)
                wrong_p();
            else
            {
                send(clientfd, str_cp, strlen(str_cp)+1, 0);
                break;
            }
        }
        else
        {
            wrong_p();
        }
    }
    //close the socket
    shutdown(clientfd, SHUT_RDWR);
    close(clientfd);
    return 0;
}