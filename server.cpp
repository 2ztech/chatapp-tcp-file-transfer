#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#define MAX_LEN 200
#define NUM_COLORS 6
#define SIZE 1024

using namespace std;

struct terminal
{
	int id;
	string name;
	int socket;
	thread th;
};

vector<terminal> clients;
string def_col="\033[0m";
string colors[]={"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m","\033[36m"};
int seed=0;
mutex cout_mtx,clients_mtx;

string color(int code);
void set_name(int id, char name[]);
void shared_print(string str, bool endLine);
int broadcast_message(string message, int sender_id);
int broadcast_message(int num, int sender_id);
void end_connection(int id);
void handle_client(int client_socket, int id);

int main()
{   
	int server_socket;
	if((server_socket=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("socket: ");
		exit(-1);
	}

    int sockfd, new_sock;
	struct sockaddr_in server;
	server.sin_family=AF_INET;
	server.sin_port=htons(10000);
	server.sin_addr.s_addr=INADDR_ANY;
	bzero(&server.sin_zero,0);

	if((bind(server_socket,(struct sockaddr *)&server,sizeof(struct sockaddr_in)))==-1)
	{
		perror("bind error: ");
		exit(-1);
	}

	if((listen(server_socket,8))==-1)
	{
		perror("listen error: ");
		exit(-1);
	}

	struct sockaddr_in client;
	int client_socket;
	unsigned int len=sizeof(sockaddr_in);

	cout<<colors[NUM_COLORS-1]<<"\n\t  ====== Welcome to the chat-room ======   "<<endl<<def_col;

	while(1)
	{
		if((client_socket=accept(server_socket,(struct sockaddr *)&client,&len))==-1)
		{
			perror("accept error: ");
			exit(-1);
		}
		seed++;
		thread t(handle_client,client_socket,seed);
		lock_guard<mutex> guard(clients_mtx);
		clients.push_back({seed, string("Anonymous"),client_socket,(move(t))});
	}

	for(int i=0; i<clients.size(); i++)
	{
		if(clients[i].th.joinable())
			clients[i].th.join();
	}

	close(server_socket);
	return 0;
}

void write_file(int sockfd) {
    int n;
    std::ofstream fp;
    const char *filename = "received.txt";
    unsigned char buffer[SIZE];

    fp.open(filename, std::ios::binary);
    if (!fp.is_open()) {
        perror("[-]Error in creating file.");
        exit(1);
    }

    while ((n = recv(sockfd, buffer, SIZE, 0)) > 0) {
        fp.write(reinterpret_cast<const char *>(buffer), n);
        std::memset(buffer, 0, SIZE);
    }

    if (n < 0) {
        perror("[-]Error in receiving file.");
    } else {
        std::cout << "[+]Data written in the file successfully.\n";
    }

    fp.close();
}

string color(int code)
{
	return colors[code%NUM_COLORS];
}

// Set name of client
void set_name(int id, char name[])
{
	for(int i=0; i<clients.size(); i++)
	{
			if(clients[i].id==id)	
			{
				clients[i].name=string(name);
			}
	}	
}

// For synchronisation of cout statements
void shared_print(string str, bool endLine=true)
{	
	lock_guard<mutex> guard(cout_mtx);
	cout<<str;
	if(endLine)
			cout<<endl;
}

// Broadcast message to all clients except the sender
int broadcast_message(string message, int sender_id)
{
	char temp[MAX_LEN];
	strcpy(temp,message.c_str());
	for(int i=0; i<clients.size(); i++)
	{
		if(clients[i].id!=sender_id)
		{
			send(clients[i].socket,temp,sizeof(temp),0);
		}
	}		
}

// Broadcast a number to all clients except the sender
int broadcast_message(int num, int sender_id)
{
	for(int i=0; i<clients.size(); i++)
	{
		if(clients[i].id!=sender_id)
		{
			send(clients[i].socket,&num,sizeof(num),0);
		}
	}		
}

void end_connection(int id)
{
	for(int i=0; i<clients.size(); i++)
	{
		if(clients[i].id==id)	
		{
			lock_guard<mutex> guard(clients_mtx);
			clients[i].th.detach();
			clients.erase(clients.begin()+i);
			close(clients[i].socket);
			break;
		}
	}				
}

void handle_client(int client_socket, int id)
{
    char name[MAX_LEN],str[MAX_LEN];
    recv(client_socket,name,sizeof(name),0);
    set_name(id,name);    

    // Display welcome message
    string welcome_message=string(name)+string(" has joined");
    broadcast_message("#NULL",id);    
    broadcast_message(id,id);                                
    broadcast_message(welcome_message,id);    
    shared_print(color(id)+welcome_message+def_col);
    
    while(1)
    {
        memset(str, 0, sizeof(str)); // Clear the str buffer
        int bytes_received=recv(client_socket,str,sizeof(str),0);
        if(bytes_received<=0)
            return;
        if(strcmp(str,"#exit")==0)
        {
            // Display leaving message
            string message=string(name)+string(" has left");        
            broadcast_message("#NULL",id);            
            broadcast_message(id,id);                        
            broadcast_message(message,id);
            shared_print(color(id)+message+def_col);
            end_connection(id);                            
            return;
        }
        else if (strncmp(str, "/send", 5) == 0)
        {
            write_file(client_socket);
            std::cout << "[+]File data received successfully.\n";
        }
        else {
            broadcast_message(string(name),id);                    
            broadcast_message(id,id);        
            broadcast_message(string(str),id);
            shared_print(color(id)+name+" : "+def_col+str);        
        }
    }    
}