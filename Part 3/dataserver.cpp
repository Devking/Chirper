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

#include "mappings.h"
#include "queries.h"

#include <string>
#include <unordered_map>
#include <functional>

#include <thread>
#include <mutex>

// When a connection is accepted, each thread will perform this function to process the relevant query
void processQuery (int connfd, const std::unordered_map<std::string, int>& actions, 
                   std::unordered_map<std::string, std::mutex*>& fileMutexes, std::mutex* mappingMutex, 
                   std::mutex* userManifestMutex, std::mutex* emailManifestMutex) {
    // Read the received message/query itself
    char buff     [MAXLINE];
    char readbuff [MAXLINE];
    int result = read(connfd, readbuff, MAXLINE);
    if (result < 1) {
        perror("Read message failed");
        exit(5);
    }
    // Break up the client's message based on API-defined formatting
    std::string query = readbuff;
    int space = query.find(' ');
    int newline = query.find('\n');
    int fieldLength = newline - space - 1;
    std::string action = query.substr(0, space);
    std::string field = query.substr(space + 1, fieldLength);
    // Determine which action to take using the query mapping
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
    // Close the connection
    close(connfd);
}

int main() {
    // Get API mapping for query codes
    std::unordered_map<std::string, int> actions;
    initAPIMapping(actions);

    // Create unordered map for mapping user files to mutexes
    std::unordered_map<std::string, std::mutex*> fileMutexes;
    initMutexMapping(fileMutexes);

    std::mutex mappingMutex;       // Create mutex for locking the unordered map
    std::mutex userManifestMutex;  // Create mutex for the user manifest text file
    std::mutex emailManifestMutex; // Create mutex for the email manifest text file

    // Initialize variables for sockets
    int    listenfd, connfd;     // Unix file descriptors
    struct sockaddr_in servaddr; // Note C use of struct

    // Create the socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Unable to create a socket");
        exit(1);
    }

    // Set up the socket address struct (specify IPv4, IP addresses, and port #)
    memset(&servaddr, 0, sizeof(servaddr));       // zero it.
    servaddr.sin_family      = AF_INET;           // Specify the family
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // specifies IPs we will service from
    servaddr.sin_port        = htons(PORT_NUM);   // daytime server port #

    // "Bind" that address object to our listening file descriptor
    if (bind(listenfd, (SA *) &servaddr, sizeof(servaddr)) == -1) {
        perror("Unable to bind port");
        exit(2);
    }

    // Tell the system that we are going to use this socket for listening and request a queue length
    if (listen(listenfd, LISTENQ) == -1) {
        perror("Unable to listen");
        exit(3);
    }

    // Loop forever to accept multiple connection (one connection per query)
    for ( ; ; ) {
        fprintf(stderr, "Server awaiting connection...\n");

        // Block until a Python server connects.
        if ((connfd = accept(listenfd, (SA *) NULL, NULL)) == -1) {
            perror("Connection Accept Failed");
            exit(4);
        }
        fprintf(stderr, "A Python client is connected!\n");

        // Spin off a new thread to process the query of the current connection
        std::thread newThread(processQuery, connfd, std::cref(actions), std::ref(fileMutexes), 
                              &mappingMutex, &userManifestMutex, &emailManifestMutex);
        newThread.detach();
    }
}
