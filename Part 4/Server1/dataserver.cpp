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

int expectedmsgnumber = 1; // Message number to enforce total ordering
std::mutex msgnumlock;     // Lock to access / update expectedmsgnumber
std::condition_variable totalorderwait; // Condition variable to enforce total ordering

std::unordered_map<int, std::string> oldresponses; // Keep track of unacked responses
std::mutex oldresponseslock; // Lock to access 'oldresponses'

// Different threads will run this function to process different queries
void processQuery (int connfd, const std::unordered_map<std::string, int>& actions,
                   std::unordered_map<std::string, std::mutex*>& fileMutexes) {

    // Read the received message/query itself
    char buff     [MAXLINE];
    char readbuff [MAXLINE];
    int result = read(connfd, readbuff, MAXLINE);
    std::string terminationstring = "\n\n\nREND";
    if (result < 1) {
        perror("Read message failed");
        exit(5);
    }

    // Break up the web server's message based on API-defined formatting
    std::string query = readbuff;

    // Get the message number of the received message
    int newline = query.find('\n');
    std::string msgnumstring = query.substr(0, newline);
    int msgnum = stoi(msgnumstring);
    std::string sendnum = std::to_string(msgnum) + "\n";

    // Use condition variable to enforce total ordering of messages
    std::unique_lock<std::mutex> condlock(msgnumlock);

    // Outdated message number received: resend the expected response we've stored
    if (expectedmsgnumber > msgnum) {
        // Send old response with correct message number
        sendMessage(sendnum, buff, connfd);
        oldresponseslock.lock();
        sendMessage(oldresponses[msgnum], buff, connfd);
        sendMessage(terminationstring, buff, connfd);
        oldresponseslock.unlock();
        // Wait for ACK from web server
        result = read(connfd, readbuff, MAXLINE);
        // If we got an ACK, be sure to remove from our responses storage
        if (result < 1) {
            perror("Read ACK failed; closing connection");
        } else {
            std::string ack = readbuff;
            int endofline = ack.find('\n');
            int acknumber = stoi(ack.substr(0, endofline));
            oldresponseslock.lock();
            if (oldresponses.find(acknumber) != oldresponses.end())
                oldresponses.erase(acknumber);
            oldresponseslock.unlock();
        }
        // Close the connection and return
        close(connfd);
        return;
    }

    // Message number too large: Use condition variable to wait here until right message is processed
    while (expectedmsgnumber < msgnum) totalorderwait.wait(condlock);

    // Send the message number of this current message back to the client
    sendMessage(sendnum, buff, connfd);

    // Start to process the query
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
    std::string messageToSend = "";
    switch (actionID) {
        case CHKEML: messageToSend = checkEmail       (field); break;
        case CHKUSR: messageToSend = checkUserParse   (field); break;
        case CHKPWD: messageToSend = checkPassword    (newline, query, field, fileMutexes); break;
        case CHKFND: messageToSend = checkFriendParse (newline, query, field, fileMutexes); break;
        case CRTUSR: messageToSend = createUser       (newline, query, field, fileMutexes); break;
        case DELUSR: messageToSend = deleteUser       (field, fileMutexes); break;
        case CRTCHP: messageToSend = createChirp      (newline, query, field, fileMutexes); break;
        case DELCHP: messageToSend = deleteChirpParse (newline, query, field, fileMutexes); break;
        case ADDFND: messageToSend = addFriend        (newline, query, field, fileMutexes); break;
        case DELFND: messageToSend = deleteFriendParse(newline, query, field, fileMutexes); break;
        case POPLAT: messageToSend = populatePage     (field, fileMutexes); break;
        case MOVEUP: messageToSend = moveUserUpParse  (newline, query, field, fileMutexes); break;
        case MOVEDN: messageToSend = moveUserDownParse(newline, query, field, fileMutexes); break;
        default: break;
    }

    // Send the expected response back to the web server
    sendMessage(messageToSend, buff, connfd);

    // Send termination string to notify web server that we're done sending the response
    sendMessage(terminationstring, buff, connfd);

    std::cout << "The query was processed successfully." << std::endl;

    // Keep track of this response, in case it's needed again in the future
    oldresponseslock.lock();
    oldresponses[msgnum] = messageToSend;
    oldresponseslock.unlock();

    // Wait for ACK from the web server to verify our response was received
    std::cout << "Now awaiting the ACK." << std::endl;
    result = read(connfd, readbuff, MAXLINE);

    // Check to see if an ACK was received
    // If not, we do nothing and let the socket close
    if (result < 1) {
        perror("Read ACK failed; closing connection");
    // If the ACK was received, update the next expected message number
    } else {
        std::string ack = readbuff;
        int endofline = ack.find('\n');
        int acknumber = stoi(ack.substr(0, endofline));
        std::cout << "Got ACK for " << acknumber << std::endl;
        // If we received an ACK, that means the web server got our response;
        // we therefore no longer need to store our response for future use
        oldresponseslock.lock();
        if (oldresponses.find(acknumber) != oldresponses.end())
            oldresponses.erase(acknumber);
        oldresponseslock.unlock();
        // Finally, since the client has ACKed us, telling us that the message number is right,
        // we can increment the expected message number and tell everyone to check if they can now go
        expectedmsgnumber = expectedmsgnumber > acknumber ? expectedmsgnumber + 1 : acknumber + 1;
        totalorderwait.notify_all();
        std::cout << "The ACK processing was successful; moving on" << std::endl;
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
        std::thread newThread(processQuery, connfd, std::cref(actions), std::ref(fileMutexes));
        newThread.detach();
    }
}
