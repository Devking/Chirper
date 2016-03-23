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
#include <unordered_map>
#include <iostream>      // debugging purposes only
#include <fstream>       // opening/closing/writing/reading files
#include <cstdio>        // remove

#define MAXLINE  4096    // max text line length
#define SA       struct sockaddr
#define LISTENQ  1024    // 2nd argument to listen() -- size of listening queue
#define PORT_NUM 9000

#define CHKEML 1
#define CHKUSR 2
#define CHKPWD 3
#define CRTUSR 5
#define DELUSR 6

// A mapping for convenience for possible queries defined by the API
void initAPIMapping (std::unordered_map<std::string, int>& actions) {
    actions["CHKEML"] = CHKEML;
    actions["CHKUSR"] = CHKUSR;
    actions["CHKPWD"] = CHKPWD;
    actions["CRTUSR"] = CRTUSR;
    actions["DELUSR"] = DELUSR;
}

int main() {
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

    // 5. Block until someone connects.
    fprintf(stderr, "Server awaiting connection...\n");
    if ((connfd = accept(listenfd, (SA *) NULL, NULL)) == -1) {
        perror("accept failed");
        exit(4);
    }
    fprintf(stderr, "A client is connected!\n");
        
    for ( ; ; ) {

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
        std::string query = readbuff;

        // Break up requests based on specified API:
        // ACTION FIELD\n
        // OPTIONAL DATA
        int space = query.find(' ');
        int newline = query.find('\n');
        int fieldLength = newline - space - 1;
        std::string action = query.substr(0, space);
        std::string field = query.substr(space + 1, fieldLength);

        std::cout << "|" << action << "|" << "|" <<  field << "|" << std::endl;

        // Process the query / work with files
        // Be sure to check file existence for all files
        std::string returnString = "";

        // Determine which action to take
        std::unordered_map<std::string, int>::iterator itr = actions.find(action);
        int actionID = 0;
        if (itr != actions.end()) {
            actionID = itr->second;
        }

        switch (actionID) {
            // Query to check email
            case CHKEML: {
                std::ifstream emailFile("email.txt");

                if (!emailFile) {
                    std::ofstream emailFile("email.txt");
                    returnString += "NO";
                } else {
                    std::string email;
                    bool foundEmail = false;
                    while (getline(emailFile, email, ',')) if (email == field) foundEmail = true;
                    returnString += foundEmail ? "YES" : "NO";
                }
                break;
            }
            // Query to check user
            case CHKUSR: {
                std::ifstream userFile("user.txt");

                if (!userFile) {
                    std::ofstream userFile("user.txt");
                    returnString += "NO";
                } else {
                    std::string user;
                    bool foundUser = false;
                    while (getline(userFile, user, ',')) if (user == field) foundUser = true;
                    returnString += foundUser ? "YES" : "NO";
                }
                break;
            }
            // Check that the login is correct
            case CHKPWD: {

                //////////
                // Right now, grabbing the password field (after the \n) is not working
                //////////

                std::string password = query.substr(newline+1);
                std::string fileName = "users/" + field + ".txt";
                std::ifstream mainFile(fileName.c_str());
                if (mainFile) {
                    std::string filePassword;
                    getline(mainFile, filePassword);
                    std::cout << "|" << password << "|" << std::endl;
                    // Send a response saying whether login was successful or not
                    if (filePassword == password) {
                        returnString += "YES";
                    } else {
                        returnString += "NO";
                    }
                } else {
                    std::cout << "Did not find that user." << std::endl;
                    returnString += "NO";
                }
                break;
            }
            // Delete a user -- assume we really mean it when we call this
            case DELUSR: {
                // Delete the file with the user
                std::string fileName = "users/" + field + ".txt";
                const char* fileNameChar = fileName.c_str();
                if (remove(fileNameChar) != 0) {
                    std::cout << "Error deleting file!" << std::endl;
                }
                // Delete the email from the email text file
                // Delete the username from the username text file
                break;
            }
            // Create a user -- at this point, ensured that user does not exist
            case CRTUSR: {
                std::string password = query.substr(newline+1);
                std::string email = "test@test.com";
                std::string fileName = "users/" + field + ".txt";
                std::ofstream mainFile(fileName.c_str());
                // Now add password, email, and 0's to the file
                break;
            }

            // ofstream mainFile(file, std::ios_base::app)

            // The default case: if actionID is 0 (query doesn't exist)
            default:
                break;
        }

        // Send data back to the client
        const char* thing = returnString.c_str();
        sprintf(buff, "%s\n", thing);
        int len = strlen(buff);
        if (len != write(connfd, buff, strlen(buff))) {
            perror("write to connection failed");
        }

        // 6. Close the connection with the current client and go back for another.
        // Never close the connection because we're terrible people
        // close(connfd);
    }
}
