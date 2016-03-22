// Original code from Stevens Unix Network Programming, vol 1 with minor modifications by John Sterling
// Modifications on the modified code by Wells Santo

// This code acts as a data server that serves data to the Python web server
// Runs on localhost:9000

#include <stdio.h>       // perror, snprintf
#include <stdlib.h>      // exit
#include <unistd.h>      // close, write
#include <string.h>      // strlen
#include <strings.h>     // bzero
#include <sys/socket.h>  // socket, AF_INET, SOCK_STREAM, bind, listen, accept
#include <netinet/in.h>  // servaddr, INADDR_ANY, htons

#include <string>
#include <iostream>      // debugging purposes only
#include <fstream>       // opening/closing/writing/reading files

#define MAXLINE     4096    // max text line length
#define SA          struct sockaddr
#define LISTENQ     1024    // 2nd argument to listen() -- size of listening queue
#define PORT_NUM    9000

int main() {
    int    listenfd, connfd;     // Unix file descriptors
    struct sockaddr_in servaddr; // Note C use of struct
    char   buff[MAXLINE];

    // 1. Create the socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Unable to create a socket");
        exit(1);
    }

    // 2. Set up the socket address struct (specify IPv4, IP addresses, and port #)
    memset(&servaddr, 0, sizeof(servaddr));       // zero it.
    servaddr.sin_family      = AF_INET;           // Specify the family
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // specifies IPs we will service from
    servaddr.sin_port        = htons(PORT_NUM);   // daytime server port #

    // 3. "Bind" that address object to our listening file descriptor
    if (bind(listenfd, (SA *) &servaddr, sizeof(servaddr)) == -1) {
        perror("Unable to bind port");
        exit(2);
    }

    // 4. Tell the system that we are going to use this socket for listening and request a queue length
    if (listen(listenfd, LISTENQ) == -1) {
        perror("Unable to listen");
        exit(3);
    }

    // 5. Block until someone connects.
    for ( ; ; ) {
        fprintf(stderr, "Server awaiting connection...\n");
        if ((connfd = accept(listenfd, (SA *) NULL, NULL)) == -1) {
            perror("accept failed");
            exit(4);
        }
        fprintf(stderr, "A client is connected!\n");

        ///////////////////////////////////////////////////////
        // At this point, we have a connection! Do our task! //
        ///////////////////////////////////////////////////////

        // Get the client's request and store it in readbuff
        char readbuff[MAXLINE];

        int result = read(connfd, readbuff, MAXLINE);
        if (result < 1) {
            perror("read failed");
            exit(5);
        }

        // Break up the client's request
        std::string test = readbuff;

        // Break up requests based on specified API:
        // ACTION FIELD\n
        // OPTIONAL DATA
        int space = test.find(' ');
        int newline = test.find('\n');
        int fieldLength = newline - space - 2;
        std::string action = test.substr(0, space);
        std::string field = test.substr(space + 1, fieldLength);

        std::cout << "|" << action << "|" << "|" <<  field << "|" << std::endl;

        // Process the query / work with files
        // Be sure to check file existence for all files
        std::string returnString = "";

        // Query to check email
        if (action == "CHECKEMAIL") {
        	std::ifstream emailFile("email.txt");

        	// If email file does not exist, create one and tell client that email does not exist yet
        	if (!emailFile) {
        		std::ofstream emailFile("email.txt");
        		returnString += "NO";

        	// If email file does exist, loop through it and find whether the email exists or not
        	} else {
        		std::string email;
        		bool foundEmail = false;
        		while(getline(emailFile, email, ',')) {
        			std::cout << email << std::endl;
        			if (email == field) foundEmail = true;
        		}
        		returnString += foundEmail ? "YES" : "NO";
        	}
        }

        // Send data back to the client
        const char* thing = returnString.c_str();
        sprintf(buff, "%s\n", thing);
        int len = strlen(buff);
        if (len != write(connfd, buff, strlen(buff))) {
            perror("write to connection failed");
        }

        // 6. Close the connection with the current client and go back for another.
        close(connfd);
    }
}
