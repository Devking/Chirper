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
std::unordered_map<int, std::string> oldresponses;
std::mutex oldresponseslock;

// When a connection is accepted, each thread will perform this function to process the relevant query
void processQuery (int connfd, const std::unordered_map<std::string, int>& actions,
                   std::unordered_map<std::string, std::mutex*>& fileMutexes, std::mutex* mappingMutex,
                   std::mutex* userManifestMutex, std::mutex* emailManifestMutex) {

    // Read the received message/query itself
    char buff     [MAXLINE];
    char readbuff [MAXLINE];
    int result = read(connfd, readbuff, MAXLINE);
    std::string terminationstring = "\n\n\nREND";
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
    std::string sendnum = std::to_string(msgnum) + "\n";

    // Based on the message number, will use the condition variable to wait until the correct
    // next message arrives
    std::unique_lock<std::mutex> condlock(msgnumlock);

    // If the expected message number is higher than the current message, resend the response for that
    // message number and do nothing else
    if (expectedmsgnumber > msgnum) {
        sendMessage(sendnum, buff, connfd);
        oldresponseslock.lock();
        sendMessage(oldresponses[msgnum], buff, connfd);
        sendMessage(terminationstring, buff, connfd);
        oldresponseslock.unlock();
        close(connfd);
        return;
    }

    // If the msgnum is too high, then we need to wait here until the missing messages arrive at another thread
    while (expectedmsgnumber < msgnum) totalorderwait.wait(condlock);

    // At this point, it means that the msgnum == expectedmsgnum
    // We can start doing work on this query then

    // Increment the expectedmsgnum and notify everyone else to check their condition again
    std::cout << "Got through for msg " << msgnum << std::endl;

    // First send the message number of this current message back to the client
    sendMessage(sendnum, buff, connfd);

    // Start to process the query
    // Unpack the query message itself
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
        case CHKEML: messageToSend = checkEmail       (field, buff, connfd, emailManifestMutex); break;

        case CHKUSR: messageToSend = checkUser(field, userManifestMutex)
                                       ? "YES" : "NO";                           break;

        case CHKPWD: messageToSend = checkPassword    (newline, query, field, buff, connfd,
                                       mappingMutex, fileMutexes);               break;

        case CHKFND: messageToSend = checkFriendParse (newline, query, field, buff, connfd,
                                       mappingMutex, fileMutexes);               break;

        case CRTUSR: messageToSend = createUser       (newline, query, field, buff, connfd,
                                       mappingMutex, fileMutexes,
                                       emailManifestMutex, userManifestMutex);   break;

        case DELUSR: messageToSend = deleteUser       (field, buff, connfd,
                                       mappingMutex, fileMutexes,
                                       emailManifestMutex, userManifestMutex);   break;

        case CRTCHP: messageToSend = createChirp      (newline, query, field, buff, connfd,
                                       mappingMutex, fileMutexes);               break;

        case DELCHP: messageToSend = deleteChirpParse (newline, query, field, buff, connfd,
                                       mappingMutex, fileMutexes);               break;

        case ADDFND: messageToSend = addFriend        (newline, query, field, buff, connfd,
                                       mappingMutex, fileMutexes);               break;

        case DELFND: messageToSend = deleteFriendParse(newline, query, field, buff, connfd,
                                       mappingMutex, fileMutexes);               break;

        case POPLAT: messageToSend = populatePage     (field, buff, connfd, readbuff,
                                       userManifestMutex, mappingMutex,
                                       fileMutexes);                             break;

        case MOVEUP: messageToSend = moveUserUpParse  (newline, query, field, buff, connfd,
                                       mappingMutex, fileMutexes);               break;

        case MOVEDN: messageToSend = moveUserDownParse(newline, query, field, buff, connfd,
                                       mappingMutex, fileMutexes);               break;

        default:                                                                 break;
    }

    // Send message all at once
    sendMessage(messageToSend, buff, connfd);
    std::cout << "MESSAGE:\n" << messageToSend << "END MESSAGE" << std::endl;

    // Send termination string to notify web server that we're done sending the response
    sendMessage(terminationstring, buff, connfd);

    std::cout << "The query was processed successfully." << std::endl;

    // Need to get what we'd return as a response message and add it to the old responses map
    oldresponseslock.lock();
    oldresponses[msgnum] = messageToSend;
    oldresponseslock.unlock();

    std::cout << "Now awaiting the ACK." << std::endl;
    // Expect an ACK back from the web server to ensure that the message was received
    // If the message was received, we remove it from the unordered_map of responses to remember
    result = read(connfd, readbuff, MAXLINE);

    // Check to see if an ACK was actually received
    // If not, we do nothing (don't increment the expectmsgnum, since we don't know they got our response)
    if (result < 1) {
        perror("Read ACK failed; closing connection");
    // If the ACK was received, update the expectedmsgnum
    } else {
        std::string ack = readbuff;
        int endofline = ack.find('\n');
        int acknumber = stoi(ack.substr(0, endofline));
        std::cout << "Got ACK for " << acknumber << std::endl;
        // Remove the ACKed message from the map of responses to remember
        // This is because the ACK guarantees that the FE received our message
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
