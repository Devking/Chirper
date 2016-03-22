// Original code from Stevens Unix Network Programming, vol 1 with minor modifications by John Sterling
// Modifications on the modified code by Wells Santo

// This code will create a "web server", which web browsers can access as a client
// The server will be hosted on localhost:8000, and will serve HTTP messages to the client

// This code will *only* send a single file
// Of course, when the web page requests the css file, this page blindly resends the login.html file
// The next step is to check the HTTP request and return what it wants

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
    for ( ; ; ) {

        fprintf(stderr, "Server awaiting connection...\n");

        if ((connfd = accept(listenfd, (SA *) NULL, NULL)) == -1) {
            perror("accept failed");
            exit(4);
        }

        fprintf(stderr, "A client is connected!\n");

        // We had a connection! Do our task!

        // Typically, this is where we would wait to see what requests are being made
        // so that we can respond appropriately

        // Open the file and put its contents in 'msg'
        FILE* fp = fopen("web/login.html", "rb");
        if (!fp) {
            perror("Error opening file");
            exit(5);
        }
        fseek(fp, 0, SEEK_END);
        long fsize = ftell(fp);
        rewind(fp);
        char* msg = (char*) malloc(fsize);
        fread(msg, fsize, 1, fp);
        fclose(fp);

        // In this case, send our login.html file directly
        // printf("%s", msg);

        // buff is a string that we are printing to
        // snprintf(buff, sizeof(buff), "HTTP/1.1 200 OK\r\n\r\n"); // buff is maxlinelength
        sprintf(buff, "HTTP/1.1 200 OK\r\n\r\n");
        // will throw a warning due to variable length
        // also unsafe, since msg comes from the file
        sprintf(buff, msg);

        int len = strlen(buff);
        if (len != write(connfd, buff, strlen(buff))) {
            perror("write to connection failed");
        }

        // 6. Close the connection with the current client and go back for another.
        close(connfd);
    }
}
