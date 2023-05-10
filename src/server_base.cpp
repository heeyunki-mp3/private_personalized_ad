#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <iostream>
#define SIZE 1024


void send_file(FILE *fp, int sockfd){
    char data[SIZE] = {0};

    while(fgets(data, SIZE, fp) != NULL) {
        if (send(sockfd, data, sizeof(data), 0) == -1) {
            perror("[-]Error in sending file.");
            exit(1);
        }
        bzero(data, SIZE);
    }
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    FILE *fp;
    struct sockaddr_in client_addr;
    struct hostent *client;
    const char *filename = "ads.csv";

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        perror("ERROR opening socket");
    client = gethostbyname(argv[1]);
    if (client == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    bzero((char *)&client_addr, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    bcopy((char *)client->h_addr,
          (char *)&client_addr.sin_addr.s_addr,
          client->h_length);
    client_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
        perror("ERROR connecting");
    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error in reading file.");
        exit(1);
    }

    send_file(fp, sockfd);
    printf("File data sent successfully.\n");

    printf("Closing the connection.\n");
    close(sockfd);

    return 0;
}