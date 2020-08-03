#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>

// Set up connection
// Process request
// 	validate request headers
// 	content
// 	send back response 
/* Initializes socket to accept connections */

int conn_sockfd;
int cli_sockfd;
int port = 3000;
struct sockaddr_in serv_addr, cli_addr;
socklen_t size = sizeof(cli_addr);

void error(const char *msg) {
    perror(msg);
    exit(0);
};

void init_sockets(){
    conn_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(conn_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
	error("Error binding socket to address");
    };
    
    listen(conn_sockfd, 1);
    printf("Listening for connections\n");

};

int main() {
    init_sockets();
    while (1) {
	cli_sockfd = accept(conn_sockfd, (struct sockaddr *) &cli_addr, &size);
	printf("Connection accepted\n");
	int flags = fnctl(cli_sockfd, F_GETFL, 0);
	fcntl(cli_sockfd, flags | SOCK_NONBLOCK);

    };
    // accept multiple clients
    // send back response
    
    return 0;
};
