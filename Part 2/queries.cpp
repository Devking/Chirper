#include "queries.h"
#include "api_mapping.h"

#include <string>
#include <fstream>
#include <vector>

bool checkUser (const std::string& username) {
    std::ifstream userFile("manifest/user.txt");
    if (!userFile) {
        std::ofstream userFile("manifest/user.txt");
        userFile.close();
        return false;
    } else {
        std::string user;
        while (getline(userFile, user, ','))
            if (user == username) {
                userFile.close();
                return true;
            }
    }
    userFile.close();
    return false;
}

bool checkFriend (const std::string& fileName, const std::string& friendName) {
    std::ifstream mainFile(fileName.c_str());
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
            if (temp == friendName) {
                mainFile.close();
                return true;
            }
        }
        mainFile.close();
    }
    return false;
}

void deleteFriend (const std::string& fileName, const std::string& friendName) {
    std::ifstream mainFile(fileName.c_str());
    std::string fileString = "";
    std::string temp;
    getline(mainFile, temp);
    fileString += temp + "\n";
    getline(mainFile, temp);
    fileString += temp + "\n";
    getline(mainFile, temp);
    int noFriends = atoi(temp.c_str());
    fileString += std::to_string(noFriends - 1) + "\n";
    for (int i = 0; i < noFriends; i++) {
        getline(mainFile, temp);
        if (temp != friendName) fileString += temp + "\n";
    }
    while (getline(mainFile, temp))
        fileString += temp + "\n";
    mainFile.close();
    std::ofstream mainFile2(fileName.c_str());
    mainFile2 << fileString;
}

void checkValidFriends (const std::string& fileName) {
    std::ifstream mainFile(fileName.c_str());
    if (mainFile) {
        std::string temp;
        getline(mainFile, temp);
        getline(mainFile, temp);
        getline(mainFile, temp);
        int noFriends = atoi(temp.c_str());
        std::vector<std::string> friendsList;
        for (int i = 0; i < noFriends; i++) {
            getline(mainFile, temp);
            friendsList.push_back(temp);
        }
        mainFile.close();
        // Once you have the friends list, check if each is valid
        for (int i = 0; i < friendsList.size(); i++) {
            // If friend does not exist, then delete friend
            if (!checkUser(friendsList[i])) {
                deleteFriend(fileName, friendsList[i]);
            }
        }
    }
}

void deleteChirp (const std::string& fileName, int chirpid) {
    std::ifstream mainFile(fileName.c_str());
    std::string fileString = "";
    std::string temp;
    getline(mainFile, temp);
    fileString += temp + "\n";
    getline(mainFile, temp);
    fileString += temp + "\n";
    getline(mainFile, temp);
    fileString += temp + "\n";
    int noFriends = atoi(temp.c_str());
    for (int i = 0; i < noFriends; i++) {
        getline(mainFile, temp);
        fileString += temp + "\n";
    }
    getline(mainFile, temp);
    int noChirps = atoi(temp.c_str());
    fileString += std::to_string(noChirps - 1) + "\n";
    for (int i = 0; i < noChirps; i++) {
        getline(mainFile, temp);
        if (i != chirpid) fileString += temp + "\n";
    }
    mainFile.close();
    std::ofstream mainFile2(fileName.c_str());
    mainFile2 << fileString;
}

void moveUserUp (const std::string& fileName, int userid) {
    std::ifstream mainFile(fileName.c_str());
    std::string fileString = "";
    std::string temp;
    getline(mainFile, temp);
    fileString += temp + "\n";
    getline(mainFile, temp);
    fileString += temp + "\n";
    getline(mainFile, temp);
    fileString += temp + "\n";
    int noFriends = atoi(temp.c_str());
    if (userid > 0 && userid < noFriends) {
        std::string user;
        for (int i = 0; i < noFriends; i++) {
            getline(mainFile, temp);
            if (i == userid-1)
                user = temp;
            else if (i == userid)
                fileString += temp + "\n" + user + "\n";
            else
                fileString += temp + "\n";
        }
        while (getline(mainFile, temp))
            fileString += temp + "\n";
        mainFile.close();
        std::ofstream mainFile2(fileName.c_str());
        mainFile2 << fileString;
    }
}

void moveUserDown (const std::string& fileName, int userid) {
    std::ifstream mainFile(fileName.c_str());
    std::string fileString = "";
    std::string temp;
    getline(mainFile, temp);
    fileString += temp + "\n";
    getline(mainFile, temp);
    fileString += temp + "\n";
    getline(mainFile, temp);
    fileString += temp + "\n";
    int noFriends = atoi(temp.c_str());
    if (userid > -1 && userid < noFriends - 1) {
        std::string user;
        for (int i = 0; i < noFriends; i++) {
            getline(mainFile, temp);
            if (i == userid)
                user = temp;
            else if (i == userid + 1)
                fileString += temp + "\n" + user + "\n";
            else
                fileString += temp += "\n";
        }
         while (getline(mainFile, temp))
            fileString += temp + "\n";
        mainFile.close();
        std::ofstream mainFile2(fileName.c_str());
        mainFile2 << fileString;
    }
}

void checkEmail (const std::string& emailToFind, char buff[MAXLINE], int connfd) {
    std::string returnString = "";
    std::ifstream emailFile("manifest/email.txt");

    if (!emailFile) {
        std::ofstream emailFile("manifest/email.txt");
        returnString += "NO";
    } else {
        std::string email;
        bool foundEmail = false;
        while (getline(emailFile, email, ',')) if (email == emailToFind) foundEmail = true;
        returnString += foundEmail ? "YES" : "NO";
    }
    emailFile.close();
    sendMessage(returnString, buff, connfd);
}
