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
int port;

void error(const char *msg) {
    perror(msg);
    exit(0);
};

void init_sockets(){
    struct sockaddr_in serv_addr, cli_addr;
    conn_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(conn_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
	error("Error binding socket to address");
    };
    
    listen(conn_sockfd, 1);
    printf("Listening for connections\n");

    socklen_t size = sizeof(cli_addr);
    cli_sockfd = accept(conn_sockfd, (struct sockaddr *) &cli_addr, &size);
};

int main() {
    return 0;
};
