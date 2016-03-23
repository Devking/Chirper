// Original code from Stevens Unix Network Programming, vol 1 with minor modifications by John Sterling
// Modifications on the modified code by Wells Santo and Patrick Kingchatchaval

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
#include <fstream>       // opening/closing/writing/reading files
#include <cstdio>        // remove
#include <vector>

#include "api_mapping.h"
#include "queries.h"

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

        // Break up requests based on specified API
        int space = query.find(' ');
        int newline = query.find('\n');
        int fieldLength = newline - space - 1;
        std::string action = query.substr(0, space);
        std::string field = query.substr(space + 1, fieldLength);

        // Determine which action to take using the query map
        auto itr = actions.find(action);
        int actionID = (itr != actions.end()) ? itr->second : 0;

        switch (actionID) {
            case CHKEML: checkEmail(field, buff, connfd); break;
            case CHKUSR: sendMessage(checkUser(field)?"YES":"NO", buff, connfd); break;
            // Check that the login is correct
            case CHKPWD: {
                std::string returnString = "";
                int secondnewline = query.find('\n', newline+1);
                int passwordlength = secondnewline - newline - 1;
                std::string password = query.substr(newline+1, passwordlength);
                std::string fileName = "users/" + field + ".txt";
                std::ifstream mainFile(fileName.c_str());
                if (mainFile) {
                    std::string filePassword;
                    getline(mainFile, filePassword);
                    // Send a response saying whether login was successful or not
                    if (filePassword == password) {
                        returnString += "YES";
                    } else {
                        returnString += "NO";
                    }
                } else {
                    returnString += "NO";
                }
                mainFile.close();
                sendMessage(returnString, buff, connfd);
                break;
            }
            // Delete a user -- assume we really mean it when we call this
            case DELUSR: {
                // Delete the file with the user
                std::string returnString = "";
                std::string fileName = "users/" + field + ".txt";
                std::ifstream mainFile(fileName.c_str());
                std::string email = "";
                if (mainFile) {
                    getline(mainFile, email);
                    getline(mainFile, email);
                }
                mainFile.close();
                if (remove(fileName.c_str()) != 0)
                    perror("deleting user file failed");
                // Delete the email from the email text file
                std::ifstream mailFile("manifest/email.txt");
                std::string mailString = "";
                if (mailFile) {
                    std::string temp;
                    while (getline(mailFile, temp, ','))
                        if (temp != email) mailString += temp + ",";
                }
                mailFile.close();
                std::ofstream mailFile2("manifest/email.txt");
                mailFile2 << mailString;
                mailFile2.close();
                // Delete the username from the username text file
                std::ifstream nameFile("manifest/user.txt");
                std::string nameString = "";
                if (nameFile) {
                    std::string temp;
                    while (getline(nameFile, temp, ','))
                        if (temp != field) nameString += temp + ",";
                }
                nameFile.close();
                std::ofstream nameFile2("manifest/user.txt");
                nameFile2 << nameString;
                sendMessage(returnString, buff, connfd);
                break;
            }
            // Create a user -- at this point, ensured that user does not exist
            case CRTUSR: {
                std::string returnString = "";
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

                std::ofstream mailFile("manifest/email.txt", std::ios_base::app);
                mailFile << email << ",";
                mailFile.close();

                std::ofstream userFile("manifest/user.txt", std::ios_base::app);
                userFile << field << ",";
                userFile.close();

                returnString += "YES";
                sendMessage(returnString, buff, connfd);
                break;
            }

            // Create a chirp
            case CRTCHP: {
                std::string returnString = "";
                int secondnewline = query.find('\n', newline+1);
                int chirplength = secondnewline - newline - 1;
                std::string chirp = query.substr(newline+1, chirplength);

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
                sendMessage(returnString, buff, connfd);
                break;
            }

            // Add friend - At this point, confirmed it's not a duplicate friend
            case ADDFND: {
                std::string returnString = "";
                int secondnewline = query.find('\n', newline+1);
                int friendlength = secondnewline - newline - 1;
                std::string friendName = query.substr(newline+1, friendlength);

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
                sendMessage(returnString, buff, connfd);
                break;
            }

            // Check if friend exists in the friend's list already or not
            case CHKFND: {
                std::string returnString = "";
                int secondnewline = query.find('\n', newline+1);
                int friendlength = secondnewline - newline - 1;
                std::string friendName = query.substr(newline+1, friendlength);

                std::string fileName = "users/" + field + ".txt";
                std::ifstream mainFile(fileName.c_str());
                if (checkFriend(fileName, friendName))
                    returnString += "YES";
                else
                    returnString += "NO";
                sendMessage(returnString, buff, connfd);
                break;
            }

            // Populate the main page for the user
            case POPLAT: {
                std::string fileName = "users/" + field + ".txt";
                // This will first make sure that the friend's list is valid
                checkValidFriends(fileName);
                // Assumes file exist, loop through and send relevant data
                std::ifstream mainFile(fileName.c_str());
                std::string temp;
                // Send the second line of the file (email)
                getline(mainFile, temp);
                getline(mainFile, temp);
                sendMessage(temp, buff, connfd);
                read(connfd, readbuff, MAXLINE);
                // Get the number of friends
                getline(mainFile, temp);
                int noFriends = atoi(temp.c_str());
                sendMessage(temp, buff, connfd);
                read(connfd, readbuff, MAXLINE);
                // Keep track of list of friends
                std::vector<std::string> friendsList;
                for (int i = 0; i < noFriends; i++) {
                    getline(mainFile, temp);
                    friendsList.push_back(temp);
                    sendMessage(temp, buff, connfd);
                    read(connfd, readbuff, MAXLINE);
                }
                getline(mainFile, temp);
                int noChirps = atoi(temp.c_str());
                sendMessage(temp, buff, connfd);
                read(connfd, readbuff, MAXLINE);
                // Send my own chirps
                for (int i = 0; i < noChirps; i++) {
                    getline(mainFile, temp);
                    sendMessage(temp, buff, connfd);
                    read(connfd, readbuff, MAXLINE); // stuck
                }
                mainFile.close();

                // Go through the friends list and send the chirps of each friend
                for (int i = 0; i < friendsList.size(); i++) {
                    std::string friendFileName = "users/" + friendsList[i] + ".txt";
                    std::ifstream friendFile(friendFileName.c_str());
                    getline(friendFile, temp);
                    getline(friendFile, temp);
                    getline(friendFile, temp);
                    int noFriends = atoi(temp.c_str());
                    for (int i = 0; i < noFriends; i++)
                        getline(friendFile, temp);
                    // Get number of chirps
                    getline(friendFile, temp);
                    sendMessage(temp, buff, connfd);
                    read(connfd, readbuff, MAXLINE);
                    int noChirps = atoi(temp.c_str());
                    // Send this friend's chirps
                    for (int i = 0; i < noChirps; i++) {
                        getline(friendFile, temp);
                        sendMessage(temp, buff, connfd);
                        read(connfd, readbuff, MAXLINE);
                    }
                    friendFile.close();
                }
                break;
            }

            case DELFND: {
                std::string fileName = "users/" + field + ".txt";
                int secondnewline = query.find('\n', newline+1);
                int friendlength = secondnewline - newline - 1;
                std::string friendName = query.substr(newline+1, friendlength);
                deleteFriend(fileName, friendName);
                std::string temp = "YES";
                sendMessage(temp, buff, connfd);
                break;
            }

            case DELCHP: {
                std::string fileName = "users/" + field + ".txt";
                int secondnewline = query.find('\n', newline+1);
                int valuelength = secondnewline - newline - 1;
                int chirpid = atoi(query.substr(newline+1, valuelength).c_str());
                deleteChirp(fileName, chirpid);
                std::string temp = "YES";
                sendMessage(temp, buff, connfd);
                break;
            }

            case MOVEUP: {
                std::string fileName = "users/" + field + ".txt";
                int secondnewline = query.find('\n', newline+1);
                int valuelength = secondnewline - newline - 1;
                int userid = atoi(query.substr(newline+1, valuelength).c_str());
                moveUserUp(fileName, userid);
                std::string temp = "YES";
                sendMessage(temp, buff, connfd);
                break;
            }

            case MOVEDN: {
                std::string fileName = "users/" + field + ".txt";
                int secondnewline = query.find('\n', newline+1);
                int valuelength = secondnewline - newline - 1;
                int userid = atoi(query.substr(newline+1, valuelength).c_str());
                moveUserDown(fileName, userid);
                std::string temp = "YES";
                sendMessage(temp, buff, connfd);
                break;
            }

            // If query received doesn't exist, just ignore it
            default: break;
        }
    }

    // May never get to this point, since we will always wait for more messages from the Python client
    close(connfd);
}
