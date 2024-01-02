#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#define SIZE 1024

void send_file(FILE *fp, int sockfd) {
    int n;
    char data[SIZE] = {0};

    while (fgets(data, SIZE, fp) != NULL) {
        if (send(sockfd, data, strlen(data), 0) == -1) {
            perror("[-]Error in sending file.");
            exit(1);
        }
        std::memset(data, 0, SIZE);
    }
}

int main() {
    const char *ip = "127.0.0.1";
    int port = 8080;
    int e;

    int sockfd;
    struct sockaddr_in server_addr;
    FILE *fp;
    char filename[100]; // Adjust the size as needed for the file path

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("[-]Error in socket");
        exit(1);
    }
    std::cout << "[+]Client socket created successfully.\n";

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    e = connect(sockfd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr));
    if (e == -1) {
        perror("[-]Error in connection");
        exit(1);
    }
    std::cout << "[+]Connected to Server.\n";

    std::cout << "Enter the command: ";
    char command[10];
    std::cin.getline(command, sizeof(command));
    command[strcspn(command, "\n")] = '\0'; // Remove trailing newline if present

    if (strcmp(command, "/send") == 0) {
        int valid = 0;
        while (!valid) {
            std::cout << "Enter the file path: ";
            std::cin.getline(filename, sizeof(filename));
            filename[strcspn(filename, "\n")] = '\0'; // Remove trailing newline if present

            fp = fopen(filename, "r");
            if (fp == NULL) {
                perror("[-]Error in reading file. Please enter a valid file path.");
            } else {
                valid = 1;
            }
        }

        send_file(fp, sockfd);
        std::cout << "[+]File data sent successfully.\n";
    } else {
        std::cout << "Invalid command. No file sent.\n";
    }

    std::cout << "[+]Closing the connection.\n";
    close(sockfd);

    return 0;
}

