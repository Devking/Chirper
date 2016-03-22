// Original code from Stevens Unix Network Programming, vol 1 with minor modifications by John Sterling
// Modifications on the modified code by Wells Santo to send a HTTP 404 response

#include <stdio.h>       // perror, snprintf
#include <stdlib.h>      // exit
#include <unistd.h>      // close, write
#include <string.h>      // strlen
#include <strings.h>     // bzero
#include <sys/socket.h>  // socket, AF_INET, SOCK_STREAM,
                         // bind, listen, accept
#include <netinet/in.h>  // servaddr, INADDR_ANY, htons

#define MAXLINE     4096    // max text line length
#define BUFFSIZE    8192    // buffer size for reads and writes
#define SA          struct sockaddr
#define LISTENQ     1024    // 2nd argument to listen() -- how many people to listen to / listening queue
#define PORT_NUM    8000

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
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // specifies that the server will pick up traffic from any IP that your machine may service; this may very well just be "localhost"
    servaddr.sin_port        = htons(PORT_NUM);   // daytime server port #

    // 3. "Bind" that address object to our listening file descriptor
    if (bind(listenfd, (SA *) &servaddr, sizeof(servaddr)) == -1) {
        perror("Unable to bind port");
        exit(2);
    }

    // 4. Tell the system that we are going to use this sockect for
    //    listening and request a queue length
    if (listen(listenfd, LISTENQ) == -1) {
        perror("Unable to listen");
        exit(3);
    }

    // 5. Block until someone connects.
    //    We could provide a sockaddr if we wanted to know details of whom
    //    we are talking to.
    //    Last arg is where to put the size of the sockaddr if
    //    we asked for one
    for ( ; ; ) {
        // Print the following message to stderr
        fprintf(stderr, "Server awaiting connection...\n");

        if ((connfd = accept(listenfd, (SA *) NULL, NULL)) == -1) {
            perror("accept failed");
            exit(4);
        }

        fprintf(stderr, "A client is connected!\n");

        // We had a connection! Do our task!

        // In this case, send the HTTP response
        snprintf(buff, sizeof(buff), "HTTP/1.1 404 Not Found\r\n\r\n");

        int len = strlen(buff);
        if (len != write(connfd, buff, strlen(buff))) {
            perror("write to connection failed");
        }

        // 6. Close the connection with the current client and go back for another.
        close(connfd);
    }
}
