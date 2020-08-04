#include <bits/stdint-uintn.h>
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

const int MAX_EVENTS = 10;
int ep_ret;
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
    int flags = fcntl(conn_sockfd, F_GETFL, 0);
    fcntl(conn_sockfd, flags | SOCK_NONBLOCK);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(conn_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
	error("Error binding socket to address");
    };
    
    listen(conn_sockfd, 1);
    printf("Listening for connections\n");

};

void register_event(int epoll_fd) {
    struct epoll_event event;
    uint32_t ep_events;
    event.data.fd = cli_sockfd;
    event.events = ep_events;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, cli_sockfd, &event);
};

int main() {
    init_sockets();
    int epoll_fd = epoll_create1(0);
    struct epoll_event events;
    while (1) {
	cli_sockfd = accept(conn_sockfd, (struct sockaddr *) &cli_addr, &size);
	printf("Connection accepted\n");

	register_event(epoll_fd);
        ep_ret = epoll_wait(epoll_fd, &events, MAX_EVENTS, 0);
	if (ep_ret < 0) {
	    error("epoll_wait()");
	};
	for (int i = 0; i < ep_ret; i++) {
	    
	}


    };
    
    return 0;
};
