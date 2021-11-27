#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdlib.h>

#include "sock.h"

/*
help_p()
print helping message

argument:
none

return:
none
*/
void help_p()
{
    printf("Commands\t\tDescription\n");
    printf("SET [key] [value]\tStore the key value pair ([key], [value]) into the database.\n");
    printf("GET [key] [value]\tGet value of [key] from the database.\n");
    printf("DELETE [key]\tDelete [key] and it's associated value from the database.\n");
    printf("Exit\t\t\tExit.\n");
    return;
}

/*
contact_server()
send and recv msg from server socket
print message we receieve

argument:
msg: message want to send
connectfd: our clientfd

variable:
recv_buf: buffer to save receieve message

return:
none
*/
void contact_server(char msg[], int connectfd)
{
    char recv_buf[300];
    memset(recv_buf, 0, 300);
    send(connectfd, msg, strlen(msg)+1, 0);
    recv(connectfd, recv_buf, 300, 0);
    printf("%s\n", recv_buf);
    return;
}

/*
wrong_p()
print wronging message

variable:
none

return:
none
*/
void wrong_p()
{
    printf("Unkown command\n");
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
    char str_temp[300];//input temporary
    char str_cp[300];//copy temporary since input would be edit by strtok
    char* d = " ";//str used to split
    char *com_temp;//command temporary
    char *key_temp;//key temporary
    char *val_temp;//value temporary
    char *left;//other thing temporary, should be NULL

    //send pid and print greeting msg
    pid_t pid = getpid();
    sprintf(str_cp, "%u", pid);
    send(clientfd, str_cp, strlen(str_cp)+1, 0);
    printf("[INFO] Connected to %s:%s\n", server_host_name, server_port);
    printf("[INFO] Welcome! Please type HELP for avaliable commands.\n");

    //main loop
    while(send(clientfd, "KEEP", strlen("KEEP")+1, 0) != -1)
    {
        //get input and split command
        printf(">");
        scanf("%[^\n]%*c", str_temp);
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
            val_temp = strtok(NULL, d);
            left = strtok(NULL, d);
            if(key_temp == NULL)
                wrong_p();
            else
            {
                if(val_temp == NULL)
                    wrong_p();
                else
                {
                    if(left != NULL)
                        wrong_p();
                    else
                        contact_server(str_cp, clientfd);
                }
            }
        }
        else if(strcmp(com_temp, "GET") == 0)
        {
            key_temp = strtok(NULL, d);
            left = strtok(NULL, d);
            if(key_temp == NULL)
                wrong_p();
            else
            {
                if(left != NULL)
                    wrong_p();
                else
                    contact_server(str_cp, clientfd);
            }
        }
        else if(strcmp(com_temp, "DELETE") == 0)
        {
            key_temp = strtok(NULL, d);
            left = strtok(NULL, d);
            if(key_temp == NULL)
                wrong_p();
            else
            {
                if(left != NULL)
                    wrong_p();
                else
                    contact_server(str_cp, clientfd);
            }
        }
        else if(strcmp(com_temp, "EXIT") == 0)
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
            wrong_p();
    }
    //close the socket
    shutdown(clientfd, SHUT_RDWR);
    close(clientfd);
    return 0;
}