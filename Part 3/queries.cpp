// Implementation of possible queries to be processed by the data server
// Code written by Wells Lucas Santo and Patrick Kingchatchaval

#include "queries.h"
#include "mappings.h"

#include <string>
#include <fstream>
#include <vector>
#include <cstdio>   // remove (file)
using namespace std;

// Check if a user exists in the user manifest file
bool checkUser (const string& username) {
    ifstream userFile("manifest/user.txt");
    if (!userFile) {
        ofstream userFile("manifest/user.txt");
        userFile.close();
        return false;
    } else {
        string user;
        while (getline(userFile, user, ','))
            if (user == username) {
                userFile.close();
                return true;
            }
    }
    userFile.close();
    return false;
}

bool checkFriend (const string& fileName, const string& friendName) {
    ifstream mainFile(fileName.c_str());
    if (mainFile) {
        string temp;
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

void deleteFriend (const string& fileName, const string& friendName) {
    ifstream mainFile(fileName.c_str());
    string fileString = "";
    string temp;
    getline(mainFile, temp);
    fileString += temp + "\n";
    getline(mainFile, temp);
    fileString += temp + "\n";
    getline(mainFile, temp);
    int noFriends = atoi(temp.c_str());
    fileString += to_string(noFriends - 1) + "\n";
    for (int i = 0; i < noFriends; i++) {
        getline(mainFile, temp);
        if (temp != friendName) fileString += temp + "\n";
    }
    while (getline(mainFile, temp))
        fileString += temp + "\n";
    mainFile.close();
    ofstream mainFile2(fileName.c_str());
    mainFile2 << fileString;
}

void checkValidFriends (const string& fileName) {
    ifstream mainFile(fileName.c_str());
    if (mainFile) {
        string temp;
        getline(mainFile, temp);
        getline(mainFile, temp);
        getline(mainFile, temp);
        int noFriends = atoi(temp.c_str());
        vector<string> friendsList;
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

void deleteChirp (const string& fileName, int chirpid) {
    ifstream mainFile(fileName.c_str());
    string fileString = "";
    string temp;
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
    fileString += to_string(noChirps - 1) + "\n";
    for (int i = 0; i < noChirps; i++) {
        getline(mainFile, temp);
        if (i != chirpid) fileString += temp + "\n";
    }
    mainFile.close();
    ofstream mainFile2(fileName.c_str());
    mainFile2 << fileString;
}

void moveUserUp (const string& fileName, int userid) {
    ifstream mainFile(fileName.c_str());
    string fileString = "";
    string temp;
    getline(mainFile, temp);
    fileString += temp + "\n";
    getline(mainFile, temp);
    fileString += temp + "\n";
    getline(mainFile, temp);
    fileString += temp + "\n";
    int noFriends = atoi(temp.c_str());
    if (userid > 0 && userid < noFriends) {
        string user;
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
        ofstream mainFile2(fileName.c_str());
        mainFile2 << fileString;
    }
}

void moveUserDown (const string& fileName, int userid) {
    ifstream mainFile(fileName.c_str());
    string fileString = "";
    string temp;
    getline(mainFile, temp);
    fileString += temp + "\n";
    getline(mainFile, temp);
    fileString += temp + "\n";
    getline(mainFile, temp);
    fileString += temp + "\n";
    int noFriends = atoi(temp.c_str());
    if (userid > -1 && userid < noFriends - 1) {
        string user;
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
        ofstream mainFile2(fileName.c_str());
        mainFile2 << fileString;
    }
}

void checkEmail (const string& emailToFind, char buff[MAXLINE], int connfd) {
    string returnString = "";
    ifstream emailFile("manifest/email.txt");
    if (!emailFile) {
        ofstream emailFile("manifest/email.txt");
        returnString += "NO";
    } else {
        string email;
        bool foundEmail = false;
        while (getline(emailFile, email, ',')) if (email == emailToFind) foundEmail = true;
        returnString += foundEmail ? "YES" : "NO";
    }
    emailFile.close();
    sendMessage(returnString, buff, connfd);
}

void checkPassword (int newline, const string& query, const string& username, char buff[MAXLINE], int connfd) {
    string returnString = "NO";
    int secondnewline = query.find('\n', newline+1);
    int passwordlength = secondnewline - newline - 1;
    string password = query.substr(newline+1, passwordlength);
    string fileName = "users/" + username + ".txt";
    ifstream mainFile(fileName.c_str());
    if (mainFile) {
        string filePassword;
        getline(mainFile, filePassword);
        if (filePassword == password) returnString = "YES";
    }
    mainFile.close();
    sendMessage(returnString, buff, connfd);
}

void deleteUser (const string& username, char buff[MAXLINE], int connfd) {
    // Delete the file with the user
    string returnString = "";
    string fileName = "users/" + username + ".txt";
    ifstream mainFile(fileName.c_str());
    string email = "";
    if (mainFile) {
        getline(mainFile, email);
        getline(mainFile, email);
    }
    mainFile.close();
    if (remove(fileName.c_str()) != 0)
        perror("deleting user file failed");
    // Delete the email from the email text file
    ifstream mailFile("manifest/email.txt");
    string mailString = "";
    if (mailFile) {
        string temp;
        while (getline(mailFile, temp, ','))
            if (temp != email) mailString += temp + ",";
    }
    mailFile.close();
    ofstream mailFile2("manifest/email.txt");
    mailFile2 << mailString;
    mailFile2.close();
    // Delete the username from the username text file
    ifstream nameFile("manifest/user.txt");
    string nameString = "";
    if (nameFile) {
        string temp;
        while (getline(nameFile, temp, ','))
            if (temp != username) nameString += temp + ",";
    }
    nameFile.close();
    ofstream nameFile2("manifest/user.txt");
    nameFile2 << nameString;
    sendMessage(returnString, buff, connfd);
}

void createUser (int newline, const string& query, const string& username, char buff[MAXLINE], int connfd) {
    string returnString = "";
    int secondnewline = query.find('\n', newline+1);
    int passwordlength = secondnewline - newline - 1;
    string password = query.substr(newline+1, passwordlength);
    int thirdnewline = query.find('\n', secondnewline+1);
    int emaillength = thirdnewline - secondnewline - 1;
    string email = query.substr(secondnewline+1, emaillength);
    string fileName = "users/" + username + ".txt";
    ofstream mainFile(fileName.c_str());
    mainFile << password << "\n";
    mainFile << email << "\n";
    mainFile << "0\n0\n";
    ofstream mailFile("manifest/email.txt", ios_base::app);
    mailFile << email << ",";
    mailFile.close();
    ofstream userFile("manifest/user.txt", ios_base::app);
    userFile << username << ",";
    userFile.close();
    returnString += "YES";
    sendMessage(returnString, buff, connfd);
}

void createChirp (int newline, const string& query, const string& username, char buff[MAXLINE], int connfd) {
    string returnString = "";
    int secondnewline = query.find('\n', newline+1);
    int chirplength = secondnewline - newline - 1;
    string chirp = query.substr(newline+1, chirplength);
    string fileName = "users/" + username + ".txt";
    ifstream mainFile(fileName.c_str());
    string fileString = "";
    if (mainFile) {
        string temp;
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
        fileString += to_string(noChirps) + "\n";
        // Append the new chirp
        fileString += chirp + "\n";
        // Append the rest of the old file
        while (getline(mainFile, temp)) {
            fileString += temp + "\n";
        }
        mainFile.close();
        ofstream mainFile2(fileName.c_str());
        mainFile2 << fileString;
        mainFile2.close();
        returnString += "YES";
    } else {
        returnString += "NO";
    }
    sendMessage(returnString, buff, connfd);
}

void checkFriendParse (int newline, const string& query, const string& username, char buff[MAXLINE], int connfd) {
    int secondnewline = query.find('\n', newline+1);
    int friendlength = secondnewline - newline - 1;
    string friendName = query.substr(newline+1, friendlength);
    string fileName = "users/" + username + ".txt";
    ifstream mainFile(fileName.c_str());
    sendMessage(checkFriend(fileName, friendName)?"YES":"NO", buff, connfd);
}

void deleteChirpParse (int newline, const string& query, const string& username, char buff[MAXLINE], int connfd) {
    string fileName = "users/" + username + ".txt";
    int secondnewline = query.find('\n', newline+1);
    int valuelength = secondnewline - newline - 1;
    int chirpid = atoi(query.substr(newline+1, valuelength).c_str());
    deleteChirp(fileName, chirpid);
    string temp = "YES";
    sendMessage(temp, buff, connfd);
}

void addFriend (int newline, const string& query, const string& username, char buff[MAXLINE], int connfd) {
    string returnString = "";
    int secondnewline = query.find('\n', newline+1);
    int friendlength = secondnewline - newline - 1;
    string friendName = query.substr(newline+1, friendlength);
    string fileName = "users/" + username + ".txt";
    ifstream mainFile(fileName.c_str());
    string fileString = "";
    if (mainFile) {
        string temp;
        // Get the first two lines, they don't change
        getline(mainFile, temp);
        fileString += temp + "\n";
        getline(mainFile, temp);
        fileString += temp + "\n";
        // Get the # of friends line and increment by 1
        getline(mainFile,temp);
        int noFriends = atoi(temp.c_str());
        noFriends++;
        fileString += to_string(noFriends) + "\n";
        // Append the new friend
        fileString += friendName + "\n";
        // Append the rest of the old file
        while (getline(mainFile, temp)) {
            fileString += temp + "\n";
        }
        mainFile.close();
        ofstream mainFile2(fileName.c_str());
        mainFile2 << fileString;
        mainFile2.close();
        returnString += "YES";
    } else {
        returnString += "NO";
    }
    sendMessage(returnString, buff, connfd);
}

void deleteFriendParse (int newline, const string& query, const string& username, char buff[MAXLINE], int connfd) {
    string fileName = "users/" + username + ".txt";
    int secondnewline = query.find('\n', newline+1);
    int friendlength = secondnewline - newline - 1;
    string friendName = query.substr(newline+1, friendlength);
    deleteFriend(fileName, friendName);
    string temp = "YES";
    sendMessage(temp, buff, connfd);
}

void populatePage (const string& username, char buff[MAXLINE], int connfd, char readbuff[MAXLINE]) {
    string fileName = "users/" + username + ".txt";
    // This will first make sure that the friend's list is valid
    checkValidFriends(fileName);
    // Assumes file exist, loop through and send relevant data
    ifstream mainFile(fileName.c_str());
    string temp;
    // Send the second line of the file (email)
    getline(mainFile, temp);
    getline(mainFile, temp);
    temp += "\n";
    sendMessage(temp, buff, connfd);
    // Get the number of friends
    getline(mainFile, temp);
    int noFriends = atoi(temp.c_str());
    temp += "\n";
    sendMessage(temp, buff, connfd);
    // Keep track of list of friends
    vector<string> friendsList;
    for (int i = 0; i < noFriends; i++) {
        getline(mainFile, temp);
        friendsList.push_back(temp);
        temp += "\n";
        sendMessage(temp, buff, connfd);
    }
    getline(mainFile, temp);
    int noChirps = atoi(temp.c_str());
    temp += "\n";
    sendMessage(temp, buff, connfd);
    // Send this user's own chirps
    for (int i = 0; i < noChirps; i++) {
        getline(mainFile, temp);
        temp += "\n";
        sendMessage(temp, buff, connfd);
    }
    mainFile.close();
    // Go through the friends list and send the chirps of each friend
    for (int i = 0; i < friendsList.size(); i++) {
        string friendFileName = "users/" + friendsList[i] + ".txt";
        ifstream friendFile(friendFileName.c_str());
        for (int j = 0; j < 3; j++) getline(friendFile, temp);
        int noFriends = atoi(temp.c_str());
        for (int k = 0; k < noFriends; k++) getline(friendFile, temp);
        // Get number of chirps
        getline(friendFile, temp);
        temp += "\n";
        sendMessage(temp, buff, connfd);
        int noChirps = atoi(temp.c_str());
        // Send this friend's chirps
        for (int m = 0; m < noChirps; m++) {
            getline(friendFile, temp);
            temp += "\n";
            sendMessage(temp, buff, connfd);
        }
        friendFile.close();
    }
}

void moveUserUpParse (int newline, const string& query, const string& username, char buff[MAXLINE], int connfd) {
    string fileName = "users/" + username + ".txt";
    int secondnewline = query.find('\n', newline+1);
    int valuelength = secondnewline - newline - 1;
    int userid = atoi(query.substr(newline+1, valuelength).c_str());
    moveUserUp(fileName, userid);
    string temp = "YES";
    sendMessage(temp, buff, connfd);
}

void moveUserDownParse (int newline, const string& query, const string& username, char buff[MAXLINE], int connfd) {
    string fileName = "users/" + username + ".txt";
    int secondnewline = query.find('\n', newline+1);
    int valuelength = secondnewline - newline - 1;
    int userid = atoi(query.substr(newline+1, valuelength).c_str());
    moveUserDown(fileName, userid);
    string temp = "YES";
    sendMessage(temp, buff, connfd);
}
