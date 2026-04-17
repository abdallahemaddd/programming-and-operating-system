#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "auth.h"

#define PORT 8080
#define BUFFER_SIZE 2048

void encrypt_decrypt(char *data){
    char key = 'K';
    for(int i=0;i<strlen(data);i++){
        data[i] ^= key;
    }
}

void send_ls(int sock){
    FILE *fp = popen("ls", "r");
    char result[BUFFER_SIZE];
    int n = fread(result, 1, BUFFER_SIZE, fp);
    result[n] = '\0';
    pclose(fp);
    send(sock, result, strlen(result), 0);
}

void send_file(int sock, char *filename){
    FILE *fp = fopen(filename, "r");
    if(fp == NULL){
        send(sock, "File not found", 14, 0);
        return;
    }

    char content[BUFFER_SIZE];
    int n = fread(content, 1, BUFFER_SIZE, fp);
    content[n] = '\0';
    fclose(fp);

    send(sock, content, strlen(content), 0);
}

void *handle_client(void *arg){

    int new_socket = *(int*)arg;
    free(arg);

    char buffer[BUFFER_SIZE];

    int bytes = read(new_socket, buffer, BUFFER_SIZE);
    buffer[bytes] = '\0';

    encrypt_decrypt(buffer);

    if(strcmp(buffer, "secret123") != 0){
        send(new_socket, "AUTH_FAIL", 9, 0);
        close(new_socket);
        return NULL;
    }

    send(new_socket, "AUTH_OK", 7, 0);

    char username[50];
    char password[50];
    int level;

    read(new_socket, username, sizeof(username));
    read(new_socket, password, sizeof(password));

    if(authenticate_user(username, password, &level)){
        printf("User %s logged in (Level %d)\n", username, level);
        send(new_socket, "LOGIN_OK", 8, 0);
    } else {
        send(new_socket, "LOGIN_FAIL", 10, 0);
        close(new_socket);
        return NULL;
    }

    char command[100];
    char arg1[100];

    while(1){

        int cmd_bytes = read(new_socket, command, sizeof(command));
        if(cmd_bytes <= 0) break;

        printf("Command: %s\n", command);

        if(strcmp(command, "exit") == 0){
            break;
        }

        if(level == 1){
            if(strcmp(command, "ls") == 0){
                send_ls(new_socket);
            }
            else if(strcmp(command, "read") == 0){
                read(new_socket, arg1, sizeof(arg1));
                send_file(new_socket, arg1);
            }
            else if(strcmp(command, "whoami") == 0){
                send(new_socket, username, strlen(username), 0);
            }
            else{
                send(new_socket, "ACCESS DENIED", 13, 0);
            }
        }

        else if(level == 2){
            if(strcmp(command, "ls") == 0){
                send_ls(new_socket);
            }
            else if(strcmp(command, "read") == 0){
                read(new_socket, arg1, sizeof(arg1));
                send_file(new_socket, arg1);
            }
            else if(strcmp(command, "whoami") == 0){
                send(new_socket, username, strlen(username), 0);
            }
            else if(strcmp(command, "delete") == 0){
                send(new_socket, "DELETE NOT ALLOWED", 19, 0);
            }
            else{
                send(new_socket, "COMMAND OK", 10, 0);
            }
        }

        else if(level == 3){
            if(strcmp(command, "ls") == 0){
                send_ls(new_socket);
            }
            else if(strcmp(command, "read") == 0){
                read(new_socket, arg1, sizeof(arg1));
                send_file(new_socket, arg1);
            }
            else if(strcmp(command, "delete") == 0){
                read(new_socket, arg1, sizeof(arg1));
                remove(arg1);
                send(new_socket, "FILE DELETED", 12, 0);
            }
            else if(strcmp(command, "whoami") == 0){
                send(new_socket, username, strlen(username), 0);
            }
            else{
                send(new_socket, "FULL ACCESS", 11, 0);
            }
        }
    }

    close(new_socket);
    return NULL;
}

int main(){

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);

    printf("Server running...\n");

    while(1){
        new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);

        pthread_t tid;
        int *pclient = malloc(sizeof(int));
        *pclient = new_socket;

        pthread_create(&tid, NULL, handle_client, pclient);
        pthread_detach(tid);
    }

    return 0;
}