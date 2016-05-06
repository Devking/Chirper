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
#include <condition_variable>

#include <iostream>

int expectedmsgnumber = 1;
std::mutex msgnumlock;
std::condition_variable totalorderwait;

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
    // Get the message number
    int newline = query.find('\n');
    std::string msgnumstring = query.substr(0, newline);
    int msgnum = stoi(msgnumstring);

    // Based on the message number, will use the condition variable to wait until the correct
    // next message arrives
    std::unique_lock<std::mutex> condlock(msgnumlock);
    // If the expected message number is higher than the current message, just discard this message
    if (expectedmsgnumber > msgnum) return;
    // Stay here until the correct expectedmsgnum matches the current message number
    while (expectedmsgnumber != msgnum) totalorderwait.wait(condlock);
    // At this point, it means that the msgnum == expectedmsgnum
    // Increment the expectedmsgnum and notify everyone else to check their condition again
    std::cout << "Got through for msg " << msgnum << std::endl;
    expectedmsgnumber++;
    totalorderwait.notify_all();

    // Start to process the query
    // Firstnpack the query message itself
    query = query.substr(newline + 1);
    std::cout << query << std::endl;
    int space = query.find(' ');
    newline = query.find('\n');
    int fieldLength = newline - space - 1;
    std::string action = query.substr(0, space);
    std::string field = query.substr(space + 1, fieldLength);

    // Determine which action to take using the query mapping
    auto itr = actions.find(action);
    int actionID = (itr != actions.end()) ? itr->second : 0;
    switch (actionID) {
        case CHKEML: checkEmail       (field, buff, connfd, emailManifestMutex); break;

        case CHKUSR: sendMessage      (checkUser(field, userManifestMutex)
                                       ? "YES" : "NO", buff, connfd);            break;

        case CHKPWD: checkPassword    (newline, query, field, buff, connfd,
                                       mappingMutex, fileMutexes);               break;

        case CHKFND: checkFriendParse (newline, query, field, buff, connfd,
                                       mappingMutex, fileMutexes);               break;

        case CRTUSR: createUser       (newline, query, field, buff, connfd,
                                       mappingMutex, fileMutexes,
                                       emailManifestMutex, userManifestMutex);   break;

        case DELUSR: deleteUser       (field, buff, connfd,
                                       mappingMutex, fileMutexes,
                                       emailManifestMutex, userManifestMutex);   break;

        case CRTCHP: createChirp      (newline, query, field, buff, connfd,
                                       mappingMutex, fileMutexes);               break;

        case DELCHP: deleteChirpParse (newline, query, field, buff, connfd,
                                       mappingMutex, fileMutexes);               break;

        case ADDFND: addFriend        (newline, query, field, buff, connfd,
                                       mappingMutex, fileMutexes);               break;

        case DELFND: deleteFriendParse(newline, query, field, buff, connfd,
                                       mappingMutex, fileMutexes);               break;

        case POPLAT: populatePage     (field, buff, connfd, readbuff,
                                       userManifestMutex, mappingMutex,
                                       fileMutexes);                             break;

        case MOVEUP: moveUserUpParse  (newline, query, field, buff, connfd,
                                       mappingMutex, fileMutexes);               break;

        case MOVEDN: moveUserDownParse(newline, query, field, buff, connfd,
                                       mappingMutex, fileMutexes);               break;

        default:                                                                 break;
    }

    // Close the connection
    close(connfd);
}

// When running the server, you must specify the port to run it on
int main(int argc, char* argv[]) {
    // Check to see that a second parameter was specified
    if (argc < 2) {
        fprintf(stderr, "Not enough parameters specified! You must specify a port number. (e.g., './dataserver 9000')\n");
        exit(5);
    } else if (argc > 2) {
        fprintf(stderr, "Too many parameters. Please only specify a port number.");
        exit(6);
    }

    // Convert the second parameter (port number) to int
    // This assumes a valid string was used
    const int PORT_NUM = atoi(argv[1]);
    fprintf(stderr, "Data server running on port %d...\n", PORT_NUM);

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
