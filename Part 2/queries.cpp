#include "queries.h"
#include "api_mapping.h"

#include <string>
#include <fstream>
#include <vector>
#include <cstdio>   // remove
#include <unistd.h> // read

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

void checkPassword (int newline, const std::string& query, const std::string& username, char buff[MAXLINE], int connfd) {
    std::string returnString = "NO";
    int secondnewline = query.find('\n', newline+1);
    int passwordlength = secondnewline - newline - 1;
    std::string password = query.substr(newline+1, passwordlength);
    std::string fileName = "users/" + username + ".txt";
    std::ifstream mainFile(fileName.c_str());
    if (mainFile) {
        std::string filePassword;
        getline(mainFile, filePassword);
        if (filePassword == password) returnString = "YES";
    }
    mainFile.close();
    sendMessage(returnString, buff, connfd);
}

void deleteUser (const std::string& username, char buff[MAXLINE], int connfd) {
    // Delete the file with the user
    std::string returnString = "";
    std::string fileName = "users/" + username + ".txt";
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
            if (temp != username) nameString += temp + ",";
    }
    nameFile.close();
    std::ofstream nameFile2("manifest/user.txt");
    nameFile2 << nameString;
    sendMessage(returnString, buff, connfd);
}

void createUser (int newline, const std::string& query, const std::string& username, char buff[MAXLINE], int connfd) {
    std::string returnString = "";
    int secondnewline = query.find('\n', newline+1);
    int passwordlength = secondnewline - newline - 1;
    std::string password = query.substr(newline+1, passwordlength);

    int thirdnewline = query.find('\n', secondnewline+1);
    int emaillength = thirdnewline - secondnewline - 1;
    std::string email = query.substr(secondnewline+1, emaillength);

    std::string fileName = "users/" + username + ".txt";
    std::ofstream mainFile(fileName.c_str());

    mainFile << password << "\n";
    mainFile << email << "\n";
    mainFile << "0\n0\n";

    std::ofstream mailFile("manifest/email.txt", std::ios_base::app);
    mailFile << email << ",";
    mailFile.close();

    std::ofstream userFile("manifest/user.txt", std::ios_base::app);
    userFile << username << ",";
    userFile.close();

    returnString += "YES";
    sendMessage(returnString, buff, connfd);
}

void createChirp (int newline, const std::string& query, const std::string& username, char buff[MAXLINE], int connfd) {
    std::string returnString = "";
    int secondnewline = query.find('\n', newline+1);
    int chirplength = secondnewline - newline - 1;
    std::string chirp = query.substr(newline+1, chirplength);

    std::string fileName = "users/" + username + ".txt";
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
}

void checkFriendParse (int newline, const std::string& query, const std::string& username, char buff[MAXLINE], int connfd) {
    int secondnewline = query.find('\n', newline+1);
    int friendlength = secondnewline - newline - 1;
    std::string friendName = query.substr(newline+1, friendlength);
    std::string fileName = "users/" + username + ".txt";
    std::ifstream mainFile(fileName.c_str());
    sendMessage(checkFriend(fileName, friendName)?"YES":"NO", buff, connfd);
}

void deleteChirpParse (int newline, const std::string& query, const std::string& username, char buff[MAXLINE], int connfd) {
    std::string fileName = "users/" + username + ".txt";
    int secondnewline = query.find('\n', newline+1);
    int valuelength = secondnewline - newline - 1;
    int chirpid = atoi(query.substr(newline+1, valuelength).c_str());
    deleteChirp(fileName, chirpid);
    std::string temp = "YES";
    sendMessage(temp, buff, connfd);
}

void addFriend (int newline, const std::string& query, const std::string& username, char buff[MAXLINE], int connfd) {
    std::string returnString = "";
    int secondnewline = query.find('\n', newline+1);
    int friendlength = secondnewline - newline - 1;
    std::string friendName = query.substr(newline+1, friendlength);

    std::string fileName = "users/" + username + ".txt";
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
}

void deleteFriendParse (int newline, const std::string& query, const std::string& username, char buff[MAXLINE], int connfd) {
    std::string fileName = "users/" + username + ".txt";
    int secondnewline = query.find('\n', newline+1);
    int friendlength = secondnewline - newline - 1;
    std::string friendName = query.substr(newline+1, friendlength);
    deleteFriend(fileName, friendName);
    std::string temp = "YES";
    sendMessage(temp, buff, connfd);
}

void populatePage (const std::string& username, char buff[MAXLINE], int connfd, char readbuff[MAXLINE]) {
    std::string fileName = "users/" + username + ".txt";
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
}

void moveUserUpParse (int newline, const std::string& query, const std::string& username, char buff[MAXLINE], int connfd) {
    std::string fileName = "users/" + username + ".txt";
    int secondnewline = query.find('\n', newline+1);
    int valuelength = secondnewline - newline - 1;
    int userid = atoi(query.substr(newline+1, valuelength).c_str());
    moveUserUp(fileName, userid);
    std::string temp = "YES";
    sendMessage(temp, buff, connfd);
}

void moveUserDownParse (int newline, const std::string& query, const std::string& username, char buff[MAXLINE], int connfd) {
    std::string fileName = "users/" + username + ".txt";
    int secondnewline = query.find('\n', newline+1);
    int valuelength = secondnewline - newline - 1;
    int userid = atoi(query.substr(newline+1, valuelength).c_str());
    moveUserDown(fileName, userid);
    std::string temp = "YES";
    sendMessage(temp, buff, connfd);
}
