#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <signal.h>
#include <mutex>
#define MAX_LEN 200
#define NUM_COLORS 6
#define SIZE 1024

using namespace std;

bool exit_flag=false;
thread t_send, t_recv;
int client_socket;
string def_col="\033[0m";
string colors[]={"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"};

void catch_ctrl_c(int signal);
string color(int code);
int eraseText(int cnt);
void send_message(int client_socket);
void recv_message(int client_socket);

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

int main()
{
	if((client_socket=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("socket: ");
		exit(-1);
	}

	struct sockaddr_in client;
	FILE *fp;
    char filename[100]; // Adjust the size as needed for the file path
	client.sin_family=AF_INET;
	client.sin_port=htons(10000); // Port no. of server
	client.sin_addr.s_addr=INADDR_ANY;
	//client.sin_addr.s_addr=inet_addr("127.0.0.1"); // Provide IP address of server
	bzero(&client.sin_zero,0);

	if((connect(client_socket,(struct sockaddr *)&client,sizeof(struct sockaddr_in)))==-1)
	{
		perror("connect: ");
		exit(-1);
	}
	signal(SIGINT, catch_ctrl_c);
	char name[MAX_LEN];
	cout<<"Enter your name : ";
	cin.getline(name,MAX_LEN);
	send(client_socket,name,sizeof(name),0);

	cout<<colors[NUM_COLORS-1]<<"\n\t  ====== Welcome to the chat-room ======   "<<endl<<def_col;

	thread t1(send_message, client_socket);
	thread t2(recv_message, client_socket);

	t_send=move(t1);
	t_recv=move(t2);

	if(t_send.joinable())
		t_send.join();
	if(t_recv.joinable())
		t_recv.join();
			
	return 0;
}

// Handler for "Ctrl + C"
void catch_ctrl_c(int signal) 
{
	char str[MAX_LEN]="#exit";
	send(client_socket,str,sizeof(str),0);
	exit_flag=true;
	t_send.detach();
	t_recv.detach();
	close(client_socket);
	exit(signal);
}

string color(int code)
{
	return colors[code%NUM_COLORS];
}

// Erase text from terminal
int eraseText(int len) {
    for(int i=0;i<len;i++)
        cout<<"\b \b";
    return 0;
}

// Send message to everyone
void send_message(int client_socket) {
    while (1) {
        cout << colors[1] << "You : " << def_col;
        char str[MAX_LEN];
        cin.getline(str, MAX_LEN);
        send(client_socket, str, sizeof(str), 0);
        if (strcmp(str, "/up") == 0) {
    char filename[MAX_LEN];
    FILE *fp = NULL;
    int valid = 0;
    while (!valid) {
        cout << "Enter the file path: ";
        cin.getline(filename, sizeof(filename));
        filename[strcspn(filename, "\n")] = 0; // Remove trailing newline if present

        // Open the file
        FILE *fp = fopen(filename, "rb");
        if (fp == NULL) {
            perror("[-]Error in reading file.");
            exit(1);
        }

        // Get the file size
        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        // Send the file size to the server
        send(client_socket, &file_size, sizeof(file_size), 0);

        // Then send the file data
        char data[SIZE] = {0};
        int total_sent = 0;
        size_t n;
        while ((n = fread(data, sizeof(char), SIZE, fp)) > 0) {
            if (send(client_socket, data, n, 0) == -1) {
                perror("[-]Error in sending file.");
                exit(1);
            }
            memset(data, 0, SIZE);
            total_sent += n;
        }

        // Send the EOF marker
        char eof_marker[] = "EOF";
        send(client_socket, eof_marker, sizeof(eof_marker), 0);

        if (total_sent < file_size) {
            std::cout << "[-]Failed to send the entire file.\n";
        } else {
            std::cout << "[+]File data sent successfully.\n";
        }

        fclose(fp);

        } if (strcmp(str, "#exit") == 0) {
            exit_flag = true;
            close(client_socket);
            return;
        }
    }
    }
}

// Receive message
void recv_message(int client_socket) {
    
    while(1)
    {
        if(exit_flag)
            return;
        char name[MAX_LEN], str[MAX_LEN];
        int color_code;
        int bytes_received=recv(client_socket,name,sizeof(name),0);
        if(bytes_received<=0)
            continue;
        recv(client_socket,&color_code,sizeof(color_code),0);
        recv(client_socket,str,sizeof(str),0);
        eraseText(6);
        if(strcmp(name,"#NULL")!=0)
            cout<<color(color_code)<<name<<" : "<<def_col<<str<<endl;
        else
            cout<<color(color_code)<<str<<endl;
        cout<<colors[1]<<"You : "<<def_col;
        fflush(stdout);
    }	
}