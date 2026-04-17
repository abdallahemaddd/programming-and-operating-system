#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 2048

void encrypt_decrypt(char *data){
    char key = 'K';
    for(int i=0;i<strlen(data);i++){
        data[i] ^= key;
    }
}

int main(){

    int sock;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];

    sock = socket(AF_INET,SOCK_STREAM,0);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);

    connect(sock,(struct sockaddr*)&server_address,sizeof(server_address));

    char keymsg[] = "secret123";
    encrypt_decrypt(keymsg);
    send(sock,keymsg,strlen(keymsg),0);

    int bytes = read(sock,buffer,BUFFER_SIZE);
    buffer[bytes] = '\0';
    printf("Server: %s\n",buffer);

    if(strcmp(buffer,"AUTH_OK") != 0){
        close(sock);
        return 0;
    }

    char username[50];
    char password[50];

    printf("Username: ");
    scanf("%s", username);

    printf("Password: ");
    scanf("%s", password);

    send(sock, username, sizeof(username), 0);
    send(sock, password, sizeof(password), 0);

    bytes = read(sock,buffer,BUFFER_SIZE);
    buffer[bytes] = '\0';
    printf("Server: %s\n",buffer);

    if(strcmp(buffer,"LOGIN_OK") != 0){
        close(sock);
        return 0;
    }

    char command[100];
    char arg1[100];

    while(1){

        printf("\nEnter command (or exit): ");
        scanf("%s", command);

        send(sock, command, sizeof(command), 0);

        if(strcmp(command, "exit") == 0){
            break;
        }

        if(strcmp(command, "read") == 0 || strcmp(command, "delete") == 0){
            printf("Enter filename: ");
            scanf("%s", arg1);
            send(sock, arg1, sizeof(arg1), 0);
        }

        bytes = read(sock, buffer, BUFFER_SIZE);
        buffer[bytes] = '\0';

        printf("\nServer response:\n%s\n", buffer);
    }

    close(sock);
    return 0;
}