#include <chrono>
#include <cstdint>
#include <filesystem>
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

using namespace std::chrono;

void write_file(int sockfd, const char* filename){
    int n;
    FILE *fp;
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
    int sockfd, new_sock, portno, n;
    struct sockaddr_in server_addr, new_addr;
    socklen_t addr_size;
    struct hostent *server;
    const char *filename = "ads_copy.csv";

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        perror("Error in socket");
        exit(1);
    }
    printf("Socket created successfully.\n");
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
    auto time_pre_r = high_resolution_clock::now();
    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("Client: Could not connect to server socket");
    }
    n = write(sockfd, "OBTAIN", 6);
    write_file(sockfd, filename);
    auto time_post_r = high_resolution_clock::now();
    auto time = duration_cast<microseconds>(time_post_r-time_pre_r).count();
    std::cout << "User: time of querying ads: " << time/1000 << " ms\n";
    std::uintmax_t size = std::filesystem::file_size(filename);
    std::cout << "User: ads file size: " << size << " bytes\n";
    printf("Data written in the file successfully.\n");

    return 0;
}
