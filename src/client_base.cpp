#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <arpa/inet.h>
#define SIZE 1024

void write_file(int sockfd){
    int n;
    FILE *fp;
    const char *filename = "ads_copy.csv";
    char buffer[SIZE];

    fp = fopen(filename, "w");
    while (true) {
        n = recv(sockfd, buffer, SIZE, 0);
        if (n <= 0){
            break;
        }
        fprintf(fp, "%s", buffer);
        bzero(buffer, SIZE);
    }
    return;
}

int main(int argc, char *argv[]){
    int sockfd, new_sock, portno, e;
    struct sockaddr_in server_addr, new_addr;
    socklen_t addr_size;
    struct hostent *server;
    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        perror("Error in socket");
        exit(1);
    }
    printf("Server socket created successfully.\n");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    portno = atoi(argv[2]);

    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portno);
    bcopy((char *)server->h_addr,
          (char *)&server_addr.sin_addr.s_addr,
          server->h_length);

    e = bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (e < 0) {
        perror("Error in bind");
        exit(1);
    }
    printf("Binding successful.\n");

    if (listen(sockfd, 10) == 0){
        printf("Listening...\n");
    } else {
        perror("Error in listening");
        exit(1);
    }

    addr_size = sizeof(new_addr);
    new_sock = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size);
    write_file(new_sock);
    printf("Data written in the file successfully.\n");

    return 0;
}
