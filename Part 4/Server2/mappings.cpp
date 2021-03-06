#include "mappings.h"

#include <unordered_map>
#include <string>
#include <string.h> // strlen
#include <unistd.h> // write
#include <fstream>
#include <mutex>

// A mapping to whitelist possible queries defined by the API
// Macros are defined in the mappings.h header
void initAPIMapping (std::unordered_map<std::string, int>& actions) {
    actions["CHKEML"] = CHKEML;
    actions["CHKUSR"] = CHKUSR;
    actions["CHKPWD"] = CHKPWD;
    actions["CHKFND"] = CHKFND;
    actions["CRTUSR"] = CRTUSR;
    actions["DELUSR"] = DELUSR;
    actions["CRTCHP"] = CRTCHP;
    actions["DELCHP"] = DELCHP;
    actions["ADDFND"] = ADDFND;
    actions["DELFND"] = DELFND;
    actions["POPLAT"] = POPLAT;
    actions["MOVEUP"] = MOVEUP;
    actions["MOVEDN"] = MOVEDN;
}

// Before the server even creates the socket, it needs to make sure that we have a lock
// for every user text file that already exists in the system
void initMutexMapping (std::unordered_map<std::string, std::mutex*>& fileMutexes) {
    // Open the user manifest file.
    // A mutex is not needed, since this happens before the multithreading.
    std::ifstream userFile("manifest/user.txt");
    // If user manifest file does not exist, make it.
    if (!userFile) {
        std::ofstream userFile("manifest/user.txt");
        userFile.close();
    // Otherwise, loop through the user file and add each user to the map
    } else {
        std::string user;
        while (getline(userFile, user, ',')) {
            fileMutexes[user] = new std::mutex;
        }
    }
    userFile.close();
}

// Send a message over the network
void sendMessage (const std::string& returnString, char buff[MAXLINE], int connfd) {
    const char* thing = returnString.c_str();
    sprintf(buff, "%s", thing);
    int len = strlen(buff);
    try {
        if (len != write(connfd, buff, strlen(buff)))
            perror("write to connection failed");
    } catch (const std::exception& e) {
        perror("hello");
    }
}
