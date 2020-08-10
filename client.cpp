#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <termios.h>

#define HOST 'h'
#define PORT 'p'
#define MAX_EVENTS 10

const char eof = 0x04;
const char CTRL_C = 0x03;
const char CR = 0x0D;
const char LF = 0x0A;
const char CRLF[2] = { CR, LF };
struct termios saved_attributes;
char *hostname;

int port = 80;
int epoll_fd = epoll_create1(0);

void error(const char *msg) {
    perror(msg);
    exit(0);
};

void restore_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &saved_attributes);
};
  
void set_attributes() {
    struct termios attributes;
    tcgetattr(STDIN_FILENO, &attributes);
    attributes.c_iflag = ISTRIP;
    attributes.c_oflag = 0;
    attributes.c_lflag = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &attributes);
};

void verify_input() {
    if (!isatty(STDIN_FILENO)) {
	fprintf(stderr, "Input is not a terminal.\n");
	exit(2);
    }
};

void set_terminal_mode() {
    verify_input();
    tcgetattr(STDIN_FILENO, &saved_attributes);
    atexit(restore_terminal);
    set_attributes();
};

// Listen for events from stdin and server
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

// Print to console and send to server
int send_data(int server_fd) {
    int flag = 0;
    struct epoll_event events[MAX_EVENTS];
    int ep_ret;
    char buf[256];
    char header[] = "POST HTTP/1.1\r\nHost: localhost\r\n";
    char GET_header[] = "GET / HTTP/1.1\r\nHost: ";
    char GET_suffix[] = "\r\nUser-Agent: curl/7.68.0\r\nAccept */*\r\n\r\n";
    char request_copy[512];
    strcpy(request_copy, GET_header);
    strncat(request_copy, hostname, strlen(hostname)); 
    strncat(request_copy, GET_suffix, strlen(GET_suffix)); 
    write(server_fd, request_copy, strlen(request_copy));
    // printf("REQUEST VVVVV\n");
    // write(STDOUT_FILENO, request_copy, strlen(request_copy));

    char server_buf[2024];
    while (1) {
	int read_bytes = read(server_fd, server_buf, sizeof(buf));
	write(STDOUT_FILENO, server_buf, read_bytes);
    };

    // while (1 && !flag) {
    //     ep_ret = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    // 	if (ep_ret < 0) {
    // 	    perror("epoll_wait()");
    // 	    return 1;
    // 	};
    // 	for (int i = 0; i < ep_ret; i++) {
    // 	    if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
    // 		fprintf(stderr, "Error with event\n");
    // 		close(events[i].data.fd);
    // 		continue;
    // 	    } else if (events[i].data.fd == STDIN_FILENO) {
    // 		char stdin_buf[256];
    // 		int read_bytes = read(STDIN_FILENO, stdin_buf, sizeof(stdin_buf));
    // 		if (read_bytes < 0) {
    // 		    perror("read()");
    // 		    continue;
    // 		};
    // 		char copy[512];
    // 		strcpy(copy, header);
    // 		strncat(copy, stdin_buf, read_bytes); 
    // 		write(server_fd, copy, strlen(copy));
    // 	    } else if (events[i].data.fd == server_fd) {
    // 		char server_buf[256];
    // 		int read_bytes = read(server_fd, server_buf, sizeof(buf));
    // 		write(STDOUT_FILENO, server_buf, read_bytes);
    // 	    } else {
    // 		printf("Unknown file descriptor: %d\n", events[i].data.fd);
    // 	    };
    // 	};
    // };
    return 0;
};

int connect_to_server() {
    int sockfd;
    struct hostent *hostinfo;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0); /* Create socket */
    if (sockfd < 0) {
    	error("Error creating socket");
    };
    register_event(sockfd);

    hostinfo = gethostbyname(hostname);
    if (hostinfo == NULL) {
	error("Error with hostinfo");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr = *(struct in_addr *) hostinfo->h_addr_list[0];
    serv_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
	error("Trouble connecting to server");
    };
    
    return sockfd;
};

void process_args(int argc, char *argv[]) {
    while (1) {
	int option_index = 0;
	static struct option long_options[] = {{"port", required_argument, 0, PORT},
					       {"host", required_argument, 0, HOST},
					       {0, 0, 0, 0}};
	int c = getopt_long(argc, argv, "", long_options, &option_index);
	if (c == -1) {
	    break;
	}
	switch (c) {
	case HOST:
	    hostname = optarg;
	    break;
	case PORT:
	    port = atoi(optarg);
	    break;
	case '?':
	    fprintf(stderr, "Invalid argument. Arguments are: --port=portnumber \n");
	    exit(1);
	default:
	    break;
	}
    }
    if (!*hostname) {
	printf("Required argument: --host\n");
	exit(1);
    }
};

int main(int argc, char *argv[]) {
    process_args(argc, argv);
    int server_fd = connect_to_server();
    register_event(STDIN_FILENO);
    send_data(server_fd);
    printf("Closing connection\n");
    close(server_fd);
    exit(0);
};


