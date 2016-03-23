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
#define CHKFND 4
#define CRTUSR 5
#define DELUSR 6
#define CRTCHP 7
#define ADDFND 8

// A mapping for convenience for possible queries defined by the API
void initAPIMapping (std::unordered_map<std::string, int>& actions) {
    actions["CHKEML"] = CHKEML;
    actions["CHKUSR"] = CHKUSR;
    actions["CHKPWD"] = CHKPWD;
    actions["CHKFND"] = CHKFND;
    actions["CRTUSR"] = CRTUSR;
    actions["DELUSR"] = DELUSR;
    actions["CRTCHP"] = CRTCHP;
    actions["ADDFND"] = ADDFND;
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

    // 5. Block until the Python server connects.
    fprintf(stderr, "Server awaiting connection...\n");
    if ((connfd = accept(listenfd, (SA *) NULL, NULL)) == -1) {
        perror("accept failed");
        exit(4);
    }
    fprintf(stderr, "The Python client is connected!\n");

    // 
    for ( ; ; ) {
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
                emailFile.close();
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
                userFile.close();
                break;
            }
            // Check that the login is correct
            case CHKPWD: {
                int secondnewline = query.find('\n', newline+1);
                int passwordlength = secondnewline - newline - 1;
                std::string password = query.substr(newline+1, passwordlength);
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
                mainFile.close();
                break;
            }
            // Delete a user -- assume we really mean it when we call this
            case DELUSR: {
                // Delete the file with the user
                std::string fileName = "users/" + field + ".txt";
                std::ifstream mainFile(fileName.c_str());
                std::string email = "";
                if (mainFile) {
                    getline(mainFile, email);
                    getline(mainFile, email);
                }
                mainFile.close();
                if (remove(fileName.c_str()) != 0)
                    std::cout << "Error deleting file!" << std::endl;
                // Delete the email from the email text file
                std::ifstream mailFile("email.txt");
                std::string mailString = "";
                if (mailFile) {
                    std::string temp;
                    while (getline(mailFile, temp, ','))
                        if (temp != email) mailString += temp + ",";
                    std::cout << "|" << email << "|" << std::endl;
                }
                mailFile.close();
                std::ofstream mailFile2("email.txt");
                mailFile2 << mailString;
                mailFile2.close();
                // Delete the username from the username text file
                std::ifstream nameFile("user.txt");
                std::string nameString = "";
                if (nameFile) {
                    std::string temp;
                    while (getline(nameFile, temp, ','))
                        if (temp != field) nameString += temp + ",";
                    std::cout << field << std::endl;
                }
                nameFile.close();
                std::ofstream nameFile2("user.txt");
                nameFile2 << nameString;
                break;
            }
            // Create a user -- at this point, ensured that user does not exist
            case CRTUSR: {
                int secondnewline = query.find('\n', newline+1);
                int passwordlength = secondnewline - newline - 1;
                std::string password = query.substr(newline+1, passwordlength);

                int thirdnewline = query.find('\n', secondnewline+1);
                int emaillength = thirdnewline - secondnewline - 1;
                std::string email = query.substr(secondnewline+1, emaillength);

                std::string fileName = "users/" + field + ".txt";
                std::ofstream mainFile(fileName.c_str());

                mainFile << password << "\n";
                mainFile << email << "\n";
                mainFile << "0\n0\n";

                std::ofstream mailFile("email.txt", std::ios_base::app);
                mailFile << email << ",";
                mailFile.close();

                std::ofstream userFile("user.txt", std::ios_base::app);
                userFile << field << ",";
                userFile.close();

                returnString += "YES";
                break;
            }

            // Create a chirp
            case CRTCHP: {
                int secondnewline = query.find('\n', newline+1);
                int chirplength = secondnewline - newline - 1;
                std::string chirp = query.substr(newline+1, chirplength);
                std::cout << "|" << chirp << "|" << std::endl;

                std::string fileName = "users/" + field + ".txt";
                std::ifstream mainFile(fileName.c_str());
                std::string fileString = "";
                if (mainFile) {
                    std::string temp;
                    // Get the first two lines, they don't change
                    getline(mainFile, temp);
                    fileString += temp + "\n";
                    getline(mainFile, temp);
                    fileString += temp + "\n";
                    // Get the # of friends line
                    getline(mainFile, temp);
                    fileString += temp + "\n";
                    int noFriends = atoi(temp.c_str());
                    // Skip all of the friends lines
                    for (int i = 0; i < noFriends; i++) {
                        getline(mainFile, temp);
                        fileString += temp + "\n";
                    }
                    // Get the # of chirps line and increment by 1
                    getline(mainFile, temp);
                    int noChirps = atoi(temp.c_str());
                    noChirps++;
                    fileString += std::to_string(noChirps) + "\n";
                    // Append the new chirp
                    fileString += chirp + "\n";
                    // Append the rest of the old file
                    while (getline(mainFile, temp)) {
                        fileString += temp + "\n";
                    }
                    mainFile.close();
                    std::ofstream mainFile2(fileName.c_str());
                    mainFile2 << fileString;
                    mainFile2.close();
                    returnString += "YES";
                } else {
                    returnString += "NO";
                }
                break;
            }

            // Add friend - At this point, confirmed it's not a duplicate friend
            case ADDFND: {
                int secondnewline = query.find('\n', newline+1);
                int friendlength = secondnewline - newline - 1;
                std::string friendName = query.substr(newline+1, friendlength);
                std::cout << "|" << friendName << "|" << std::endl;

                std::string fileName = "users/" + field + ".txt";
                std::ifstream mainFile(fileName.c_str());
                std::string fileString = "";
                if (mainFile) {
                    std::string temp;
                    // Get the first two lines, they don't change
                    getline(mainFile, temp);
                    fileString += temp + "\n";
                    getline(mainFile, temp);
                    fileString += temp + "\n";
                    // Get the # of friends line and increment by 1
                    getline(mainFile,temp);
                    int noFriends = atoi(temp.c_str());
                    noFriends++;
                    fileString += std::to_string(noFriends) + "\n";
                    // Append the new friend
                    fileString += friendName + "\n";
                    // Append the rest of the old file
                    while (getline(mainFile, temp)) {
                        fileString += temp + "\n";
                    }
                    mainFile.close();
                    std::ofstream mainFile2(fileName.c_str());
                    mainFile2 << fileString;
                    mainFile2.close();
                    returnString += "YES";
                } else {
                    returnString += "NO";
                }
                break;
            }

            // Check if friend exists in the friend's list already or not
            case CHKFND: {
                int secondnewline = query.find('\n', newline+1);
                int friendlength = secondnewline - newline - 1;
                std::string friendName = query.substr(newline+1, friendlength);
                std::cout << "|" << friendName << "|" << std::endl;

                std::string fileName = "users/" + field + ".txt";
                std::ifstream mainFile(fileName.c_str());
                bool friendExists = false;
                if (mainFile) {
                    std::string temp;
                    // Get the first two lines, they aren't needed
                    getline(mainFile, temp);
                    getline(mainFile, temp);
                    // Get the # of friends line
                    getline(mainFile, temp);
                    int noFriends = atoi(temp.c_str());
                    // Loop over the friends
                    for (int i = 0; i < noFriends; i++) {
                        getline(mainFile, temp);
                        if (temp == friendName) friendExists = true;
                    }
                    mainFile.close();
                }
                if (friendExists)
                    returnString += "YES";
                else
                    returnString += "NO";
                break;
            }

            // The default case: if actionID is 0 (query doesn't exist)
            default:
                break;
        }

        // Send data back to the client
        const char* thing = returnString.c_str();
        sprintf(buff, "%s", thing);
        int len = strlen(buff);
        if (len != write(connfd, buff, strlen(buff))) {
            perror("write to connection failed");
        }
    }

    // May never get to this point, since we will always wait for more messages from the Python client
    close(connfd);
}
