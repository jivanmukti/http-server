#include <unistd.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <termios.h>

const char eof = 0x04;
const char CTRL_C = 0x03;
const char CR = 0x0D;
const char LF = 0x0A;
const char CRLF[2] = { CR, LF };
struct termios saved_attributes;
#define PORT 'p'
int port;

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

// Print to console and send to server
void send_data(int server_fd) {
    int flag = 0;
    while (1 && !flag) {
	char buf[256];
	int read_bytes = read(STDIN_FILENO, buf, sizeof(buf));
	for (int i = 0; i < read_bytes; i++) {
	    if (buf[i] == CR || buf[i] == LF) {
		write(STDOUT_FILENO, &CRLF, 2);
	    } else if (buf[i] == eof || buf[i] == CTRL_C) {
		flag = 1;
		break;
	    } else {
		write(STDOUT_FILENO, &buf[i], 1);
	    };
	}
	write(server_fd, &buf, read_bytes);
    };
};

int connect_to_server() {
    int sockfd;
    struct hostent *hostinfo;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0); /* Create socket */
    if (sockfd < 0) {
    	error("Error creating socket");
    };

    hostinfo = gethostbyname("Localhost");
    if (hostinfo == NULL) {
	error("Error with hostinfo");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr = *(struct in_addr *) hostinfo->h_addr_list[0];
    serv_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
	error("Trouble connecting to server");
    }
    return sockfd;
};

void process_args(int argc, char *argv[]) {
    while (1) {
	int option_index = 0;
	static struct option long_options[] = {{"port", required_argument, 0, PORT},
					       {0, 0, 0, 0}};
	int c = getopt_long(argc, argv, "", long_options, &option_index);
	if (c == -1) {
	    break;
	}
	switch (c) {
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
    if (!port) {
	printf("Required argument: --port\n");
	exit(1);
    }
};

int main(int argc, char *argv[]) {
    process_args(argc, argv);
    int server_fd = connect_to_server();
    send_data(server_fd);
    printf("Closing connection\n");
    close(server_fd);
    exit(0);
};
