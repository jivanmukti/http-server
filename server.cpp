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
int epoll_fd = epoll_create1(0);

void error(const char *msg) {
    perror(msg);
    exit(0);
};

int register_event(int fd) {
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    int rc = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    if (rc == -1) {
	perror("epoll_ctl()");
	return -1;
    };
    return 0;
};

int init_sockets(){
    struct sockaddr_in serv_addr;
    int port = 3002;
    conn_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (conn_sockfd == -1) {
	perror("socket()");
	return 1;
    };
    int flags = fcntl(conn_sockfd, F_GETFL, 0);
    if (flags == -1) {
	perror("flags");
	return 1;
    };
    if(fcntl(conn_sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
	perror("fcntl()");
	return 1;
    };

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(conn_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
	perror("bind()");
	return 1;
    };

    if(listen(conn_sockfd, SOMAXCONN) < 0) {
	perror("listen()");
	return 1;
    };
    register_event(conn_sockfd);
    printf("Listening for connections\n");
    return 0;
};

int handle_client(int fd) {
    char buf[256];
    int close_fd = 0;
    while (1) {
	int read_bytes = read(fd, buf, sizeof(buf));
	if (read_bytes == -1) {
	    if (errno != EAGAIN) {
		perror("read()");
		return 1;
	    };
	    break;
	} else if (read_bytes == 0) {
	    printf("Client has closed connection\n");
	    close_fd = 1;
	    break;
	};

	int write_bytes = write(STDOUT_FILENO, buf, read_bytes);
	if (write_bytes < 0) {
	    perror("write()");
	    return 1;
	};
    };
    if (close_fd) {
	close(fd);
    };
    return 0;
};

int main() {
    init_sockets();
    struct sockaddr_in cli_addr;
    socklen_t size = sizeof(cli_addr);
    struct epoll_event events[MAX_EVENTS];
    while (1) {
        ep_ret = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
	if (ep_ret < 0) {
	    perror("epoll_wait()");
	    return 1;
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
		    cli_sockfd = accept(conn_sockfd, (struct sockaddr *) &cli_addr, &size);
		    if (cli_sockfd == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
			    break;
			} else {
			    perror("accept()");
			    return -1;
			};
		    };
		    printf("Connection accepted\n");
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
