// Socket code from Stevens Unix Network Programming, vol 1 with minor modifications by John Sterling
// Remaining code by Wells Santo and Patrick Kingchatchaval
// This code acts as a data server (on localhost:9000) that serves data to the Python web server

#include <stdio.h>       // perror, snprintf
#include <stdlib.h>      // exit
#include <unistd.h>      // close, write
#include <string.h>      // strlen
#include <strings.h>     // bzero
#include <sys/socket.h>  // socket, AF_INET, SOCK_STREAM, bind, listen, accept
#include <netinet/in.h>  // servaddr, INADDR_ANY, htons

#include <string>
#include <unordered_map>
#include "api_mapping.h"
#include "queries.h"

int main() {
    // Get API mapping for query codes
    std::unordered_map<std::string, int> actions;
    initAPIMapping(actions);

    // 0. Init variables for sockets
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

    // 5. Block until the Python server connects.
    //**// Need to update this to take one connection per query
    //**// Also spin off the thread once the connection is accepted?
    //**// Pass a handling function to the thread which also takes the connfd int
    fprintf(stderr, "Server awaiting connection...\n");
    if ((connfd = accept(listenfd, (SA *) NULL, NULL)) == -1) {
        perror("accept failed");
        exit(4);
    }
    fprintf(stderr, "The Python client is connected!\n");

    // 6. Wait for the client's query and process it
    for ( ; ; ) {
        char readbuff[MAXLINE];
        int result = read(connfd, readbuff, MAXLINE);
        if (result < 1) {
            perror("read failed");
            exit(5);
        }
        // Break up the client's request based on API-defined formatting
        std::string query = readbuff;
        int space = query.find(' ');
        int newline = query.find('\n');
        int fieldLength = newline - space - 1;
        std::string action = query.substr(0, space);
        std::string field = query.substr(space + 1, fieldLength);
        // Determine which action to take using the query map
        auto itr = actions.find(action);
        int actionID = (itr != actions.end()) ? itr->second : 0;
        switch (actionID) {
            case CHKEML: checkEmail       (field, buff, connfd);                       break;
            case CHKUSR: sendMessage      (checkUser(field)?"YES":"NO", buff, connfd); break;
            case CHKPWD: checkPassword    (newline, query, field, buff, connfd);       break;
            case CHKFND: checkFriendParse (newline, query, field, buff, connfd);       break;
            case CRTUSR: createUser       (newline, query, field, buff, connfd);       break;
            case DELUSR: deleteUser       (field, buff, connfd);                       break;
            case CRTCHP: createChirp      (newline, query, field, buff, connfd);       break;
            case DELCHP: deleteChirpParse (newline, query, field, buff, connfd);       break;
            case ADDFND: addFriend        (newline, query, field, buff, connfd);       break;
            case DELFND: deleteFriendParse(newline, query, field, buff, connfd);       break;
            case POPLAT: populatePage     (field, buff, connfd, readbuff);             break;
            case MOVEUP: moveUserUpParse  (newline, query, field, buff, connfd);       break;
            case MOVEDN: moveUserDownParse(newline, query, field, buff, connfd);       break;
            default:                                                                   break;
        }
    }

    // 7. Close the connection
    close(connfd);
}
