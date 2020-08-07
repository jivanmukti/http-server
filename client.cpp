#include <unistd.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <termios.h>

struct termios saved_attributes;
#define PORT 'p'
int port;

/* System call errors */
void error(char *msg) {
    perror(msg);
    exit(0);
};

/* Restores default terminal attributes */
void restore_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &saved_attributes);
};
  
/* Saves the attributes of the terminal */
void save_attributes() {
    tcgetattr(STDIN_FILENO, &saved_attributes);
};

/* Sets new attributes of the terminal */
void set_attributes() {
    struct termios attributes;
    tcgetattr(STDIN_FILENO, &attributes);
    attributes.c_iflag = ISTRIP;
    attributes.c_oflag = 0;
    attributes.c_lflag = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &attributes);
};

/* Verify that input is coming from stdin */
void verify_input() {
    if (!isatty(STDIN_FILENO)) {
	fprintf(stderr, "Input is not a terminal.\n");
	exit(2);
    }
};

/* Create new terminal mode */
void set_terminal_mode() {
    verify_input();
    save_attributes();
    atexit(restore_terminal);
    set_attributes();
};

/* Handles socket creation and server connection */
int connect_to_server() {
    int sockfd;
    struct hostent *hostinfo;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0); /* Create socket */
    if (sockfd < 0) {
    	perror("Error creating socket");
    };

    hostinfo = gethostbyname("Localhost");
    if (hostinfo == NULL) {
	perror("Error with hostinfo");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr = *(struct in_addr *) hostinfo->h_addr_list[0];
    serv_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
	perror("Trouble connecting to server.\n");
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
    connect_to_server();
};
