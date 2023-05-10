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
            perror("Error in sending file.");
            exit(1);
        }
        bzero(data, SIZE);
    }
}

int main(int argc, char *argv[])
{
    std::cout << "Main: Initializing the server socket" << std::endl;

    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[2048];
    char buffer_sending[2048];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }
    std::cout << "Socket: port recognized" << std::endl;


    // create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        perror("ERROR opening socket");
    std::cout << "Socket: opened new socket" << std::endl;

    // clear address structure
    bzero((char *)&serv_addr, sizeof(serv_addr));

    portno = atoi(argv[1]);

    /* setup the host_addr structure for use in bind call */
    // server byte order
    serv_addr.sin_family = AF_INET;

    // automatically be filled with current host's IP address
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    // convert short integer value for port must be converted into network byte order
    serv_addr.sin_port = htons(portno);

    if ((::bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
        perror("ERROR on binding");
    std::cout << "Socket: binding successful"<< std::endl;

    std::cout << "Socket: listening to incoming connection" << std::endl;
    listen(sockfd, 5);
    // The accept() call actually accepts an incoming connection
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0)
        perror("ERROR on accept");

    int obtain_len = 6;
    char *obtain_buffer = (char *)malloc(obtain_len);
    n = read(newsockfd, obtain_buffer, obtain_len);
    if (n < 0)
        perror("ERROR reading from socket");
    std::string obtain_str(obtain_buffer, obtain_len);
    if (obtain_str.compare("OBTAIN") != 0) {
        perror("ERROR getting OBTAIN from user");
    }

    FILE *fp;
    const char *filename = "ads.csv";

    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error in reading file.");
        exit(1);
    }

    send_file(fp, newsockfd);
    printf("File data sent successfully.\n");

    printf("Closing the connection.\n");
    close(sockfd);

    return 0;
}
