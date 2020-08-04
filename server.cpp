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
int epoll_fd = epoll_create1(0);
struct sockaddr_in serv_addr, cli_addr;
socklen_t size = sizeof(cli_addr);

void error(const char *msg) {
    perror(msg);
    exit(0);
};
void register_event(int fd) {
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    int rc = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    if (rc == -1) {
	error("epoll_ctl");
    };
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

    listen(conn_sockfd, SOMAXCONN);
    register_event(conn_sockfd);
    printf("Listening for connections\n");

};


int accept_clients(int conn_sockfd) {
    cli_sockfd = accept(conn_sockfd, (struct sockaddr *) &cli_addr, &size);
    printf("Connection accepted\n");
    return cli_sockfd;
};

void handle_client(int fd) {
    char buf[256];
    int close_fd = 0;
    while (1) {
	int read_bytes = read(fd, buf, sizeof(buf));
	if (read_bytes == -1) {
	    if (errno != EAGAIN) {
		fprintf(stderr, "Error reading from client\n");
	    };
	    break;
	} else if (read_bytes == 0) {
	    printf("Client has closed connection\n");
	    close_fd = 1;
	    break;
	};

	int write_bytes = write(STDOUT_FILENO, buf, sizeof(buf));
	if (write_bytes < 0) {
	    error("write()");
	};
    };
    if (close_fd) {
	close(fd);
    };
};

int main() {
    init_sockets();
    struct epoll_event events[MAX_EVENTS];
    while (1) {
        ep_ret = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
	if (ep_ret < 0) {
	    error("epoll_wait()");
	};
	for (int i = 0; i < ep_ret; i++) {
	    // If error, hangup, or not ready 
	    if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
		fprintf(stderr, "Error with event\n");
		close(events[i].data.fd);
		continue;
	    } else if (events[i].data.fd == conn_sockfd) {
		// We loop until no more pending connections in queue
		while(1) {
		    int cli_sockfd = accept_clients(conn_sockfd);
		    if (cli_sockfd < 0) {
			break;
		    };
		    register_event(cli_sockfd);
		};
	    } else {
		// Take care of read/write to client
		handle_client(events[i].data.fd);
	    };
	}
    };
    
    return 0;
};
