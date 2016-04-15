// Implementation of queries to be processed by the data server
// Code written by Wells Lucas Santo and Patrick Kingchatchaval

#include "queries.h"
#include "mappings.h"

#include <string>
#include <fstream>
#include <vector>
#include <cstdio>   // remove() file
#include <mutex>
#include <unordered_map>
using namespace std;

///////////////////////////////////////////////////////////////////////////////
// Query  1: CHKEML (Check Email)
///////////////////////////////////////////////////////////////////////////////

// Check if email already exists in the system
void checkEmail (const string& emailToFind, char buff[MAXLINE], int connfd, 
                 std::mutex* emailManifestMutex) {
    // Obtain lock to access the email manifest file
    emailManifestMutex->lock();
    ifstream emailFile("manifest/email.txt");
    // If the email manifest file does not exist, create it
    if (!emailFile) {
        ofstream emailFile("manifest/email.txt");
        sendMessage("NO", buff, connfd);
    // Otherwise, loop through the file to find if the email exists
    } else {
        string email;
        bool foundEmail = false;
        while (getline(emailFile, email, ',')) 
            if (email == emailToFind) foundEmail = true;
        sendMessage(foundEmail ? "YES" : "NO", buff, connfd);
    }
    emailFile.close();
    // Release the email manifest file lock
    emailManifestMutex->unlock();
}

///////////////////////////////////////////////////////////////////////////////
// Query  2: CHKUSR (Check User)
///////////////////////////////////////////////////////////////////////////////

// Check if user already exists in the system
bool checkUser (const string& username, std::mutex* userManifestMutex) {
    // Obtain lock to access the user manifest file
    userManifestMutex->lock();
    ifstream userFile("manifest/user.txt");
    bool foundUser = false;
    // If user manifest file does not exist, make it
    if (!userFile) {
        ofstream userFile("manifest/user.txt");
    // Otherwise, we will loop through the file and check if the user exists
    } else {
        string user;
        while (getline(userFile, user, ','))
            if (user == username) foundUser = true;
    }
    userFile.close();
    // Release the user manifest file lock
    userManifestMutex->unlock();
    return foundUser;
}

///////////////////////////////////////////////////////////////////////////////
// Query  3: CHKPWD (Check Password)
///////////////////////////////////////////////////////////////////////////////

// Check if the password is correct for a specific user
void checkPassword (int newline, const string& query, const string& username, 
                    char buff[MAXLINE], int connfd, std::mutex* mappingMutex,
                    std::unordered_map<std::string, std::mutex*>& fileMutexes) {
    string returnString = "NO";
    // Break up the query message to get the password
    int secondnewline = query.find('\n', newline+1);
    int passwordlength = secondnewline - newline - 1;
    string password = query.substr(newline+1, passwordlength);
    // Obtain the lock to access the user->mutex map
    mappingMutex->lock();
    // See if the user exists in the map
    if (fileMutexes.find(username) != fileMutexes.end()) {
        // Obtain the lock to access the user file
        fileMutexes.find(username)->second->lock();
        string fileName = "users/" + username + ".txt";
        ifstream mainFile(fileName.c_str());
        if (mainFile) {
            string filePassword;
            getline(mainFile, filePassword);
            if (filePassword == password) returnString = "YES";
        }
        mainFile.close();
        // Release the lock to access the user file
        fileMutexes.find(username)->second->unlock();
    }
    // Release the lock to access the user->mutex map
    mappingMutex->unlock();
    // Send either YES or NO if the password matches
    sendMessage(returnString, buff, connfd);
}

///////////////////////////////////////////////////////////////////////////////
// Query  4: CHKFND (Check Friend)
///////////////////////////////////////////////////////////////////////////////

// Check if friend already exists in the friend list
bool checkFriend (const string& username, const string& friendName, std::mutex* mappingMutex,
                  std::unordered_map<std::string, std::mutex*>& fileMutexes) {
    bool friendExists = false;
    // Obtain the lock to access the user->mutex map
    mappingMutex->lock();
    // See if the user exists in the map
    if (fileMutexes.find(username) != fileMutexes.end()) {
        // Obtain the lock to access the user file
        fileMutexes.find(username)->second->lock();
        string fileName = "users/" + username + ".txt";
        ifstream mainFile(fileName.c_str());
        if (mainFile) {
            string temp;
            // Get the line with the # of friends
            for (int i = 0; i < 3; i++) getline(mainFile, temp);
            int noFriends = atoi(temp.c_str());
            // Loop over the friends to see if there's a match
            for (int j = 0; j < noFriends; j++) {
                getline(mainFile, temp);
                if (temp == friendName) friendExists = true;
            }
        }
        mainFile.close();
        // Release the lock to access the user file
        fileMutexes.find(username)->second->unlock();
    }
    // Release the lock to access the user->mutex map
    mappingMutex->unlock();
    return friendExists;
}

// A driver function for the checkFriend() method to parse the query first
void checkFriendParse (int newline, const string& query, const string& username, 
                       char buff[MAXLINE], int connfd, std::mutex* mappingMutex,
                       std::unordered_map<std::string, std::mutex*>& fileMutexes) {
    // Parse the query to get the friend name to check
    int secondnewline = query.find('\n', newline+1);
    int friendlength = secondnewline - newline - 1;
    string friendName = query.substr(newline+1, friendlength);
    // Respond to the query based on whether the friend exists in the list or not
    sendMessage(checkFriend(username, friendName, mappingMutex, fileMutexes) 
                ? "YES" : "NO", buff, connfd);
}

///////////////////////////////////////////////////////////////////////////////
// Query  5: CRTUSR (Create User)
///////////////////////////////////////////////////////////////////////////////

// Create a new user
void createUser (int newline, const string& query, const string& username, 
                 char buff[MAXLINE], int connfd, std::mutex* mappingMutex,
                  std::unordered_map<std::string, std::mutex*>& fileMutexes,
                  std::mutex* emailManifestMutex, std::mutex* userManifestMutex) {
    // Break up the query to get the password
    int secondnewline = query.find('\n', newline+1);
    int passwordlength = secondnewline - newline - 1;
    string password = query.substr(newline+1, passwordlength);
    // Break up the query to get the email address
    int thirdnewline = query.find('\n', secondnewline+1);
    int emaillength = thirdnewline - secondnewline - 1;
    string email = query.substr(secondnewline+1, emaillength);
    
    // Get the lock to access the user->mutex map
    mappingMutex->lock();
    // Add the current user to the user->mutex map
    fileMutexes[username] = new mutex;
    // Get the lock to access the user file
    fileMutexes[username]->lock();
    // Create the user file and initialize it
    string fileName = "users/" + username + ".txt";
    ofstream mainFile(fileName.c_str());
    mainFile << password << "\n";
    mainFile << email << "\n";
    mainFile << "0\n0\n";
    // Release the lock to access the user file
    fileMutexes[username]->unlock();
    // Release the lock to access the user->mutex map
    mappingMutex->unlock();

    // Update the email manifest file (lock & unlock)
    emailManifestMutex->lock();
    ofstream mailFile("manifest/email.txt", ios_base::app);
    mailFile << email << ",";
    mailFile.close();
    emailManifestMutex->unlock();

    // Update the user manifest file (lock & unlock)
    userManifestMutex->lock();
    ofstream userFile("manifest/user.txt", ios_base::app);
    userFile << username << ",";
    userFile.close();
    userManifestMutex->unlock();
}

///////////////////////////////////////////////////////////////////////////////
// Query  6: DELUSR (Delete User)
///////////////////////////////////////////////////////////////////////////////

// Delete a user
void deleteUser (const string& username, char buff[MAXLINE], int connfd, std::mutex* mappingMutex,
                  std::unordered_map<std::string, std::mutex*>& fileMutexes,
                  std::mutex* emailManifestMutex, std::mutex* userManifestMutex) {
    string email;
    // Get the lock to access the user->mutex map
    mappingMutex->lock();
    // Check if the user is still in the map
    if (fileMutexes.find(username) != fileMutexes.end()) {
        // Get the lock to access the user file
        fileMutexes[username]->lock();
        // Open the user file to get their email
        string fileName = "users/" + username + ".txt";
        ifstream mainFile(fileName.c_str());
        for (int i = 0; i < 2; i++) getline(mainFile, email);
        mainFile.close();
        // Delete the file itself
        if (remove(fileName.c_str()) != 0)
            perror("deleting user file failed");
        // Release the lock to access the user file
        fileMutexes[username]->unlock();
        // Remove the user from the mutex map
        delete fileMutexes[username];
        fileMutexes.erase(fileMutexes.find(username));
    } else {
        return;
    }
    // Release the lock to access the user->mutex map
    mappingMutex->unlock();

    // Delete the email from the email text file (lock & unlock)
    emailManifestMutex->lock();
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
    emailManifestMutex->unlock();

    // Delete the username from the username text file (lock & unlock)
    userManifestMutex->lock();
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
    userManifestMutex->unlock();
}

///////////////////////////////////////////////////////////////////////////////
// Query  7: CRTCHP (Create Chirp)
///////////////////////////////////////////////////////////////////////////////

// Post a chirp for a user
void createChirp (int newline, const string& query, const string& username, 
                  char buff[MAXLINE], int connfd, std::mutex* mappingMutex,
                  std::unordered_map<std::string, std::mutex*>& fileMutexes) {
    // Get the chirp content
    int secondnewline = query.find('\n', newline+1);
    int chirplength = secondnewline - newline - 1;
    string chirp = query.substr(newline+1, chirplength);
    // Get the lock to access the user->mutex map
    mappingMutex->lock();
    // Check if the user is still in the map
    if (fileMutexes.find(username) != fileMutexes.end()) {
        // Get the lock to access the user file
        fileMutexes[username]->lock();
        // Open the user file
        string fileName = "users/" + username + ".txt";
        ifstream mainFile(fileName.c_str());
        if (mainFile) {
            string fileString = "";
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
        }
        // Release the lock to access the user file
        fileMutexes[username]->unlock();
    }
    // Release the lock to access the user->mutex map
    mappingMutex->unlock();
}

///////////////////////////////////////////////////////////////////////////////
// Query  8: DELCHP (Delete Chirp)
///////////////////////////////////////////////////////////////////////////////

// Delete a user's chirp
void deleteChirp (const string& username, int chirpid, std::mutex* mappingMutex,
                  std::unordered_map<std::string, std::mutex*>& fileMutexes) {
    // Get the lock to access the user->mutex map
    mappingMutex->lock();
    // Check if the user is still in the map
    if (fileMutexes.find(username) != fileMutexes.end()) {
        // Get the lock to access the user file
        fileMutexes[username]->lock();
        // Open the user file
        string fileName = "users/" + username + ".txt";
        ifstream mainFile(fileName.c_str());
        // Go through the file and copy over everything except the chirp to delete
        string fileString = "";
        string temp;
        getline(mainFile, temp);
        fileString += temp + "\n";
        getline(mainFile, temp);
        fileString += temp + "\n";
        getline(mainFile, temp);
        fileString += temp + "\n";
        // Get number of friends, to go through friends list dynamically
        int noFriends = atoi(temp.c_str());
        for (int i = 0; i < noFriends; i++) {
            getline(mainFile, temp);
            fileString += temp + "\n";
        }
        getline(mainFile, temp);
        int noChirps = atoi(temp.c_str());
        fileString += to_string(noChirps - 1) + "\n";
        // Copy everything except the chirp to delete
        for (int i = 0; i < noChirps; i++) {
            getline(mainFile, temp);
            if (i != chirpid) fileString += temp + "\n";
        }
        mainFile.close();
        // Overwrite the file, such that the chirp is deleted
        ofstream mainFile2(fileName.c_str());
        mainFile2 << fileString;
        // Release the lock to access the user file
        fileMutexes[username]->unlock();
    }
    // Release the lock to access the user->mutex map
    mappingMutex->unlock();
}

// Driver function to delete a user's chirp
void deleteChirpParse (int newline, const string& query, const string& username, 
                       char buff[MAXLINE], int connfd, std::mutex* mappingMutex,
                       std::unordered_map<std::string, std::mutex*>& fileMutexes) {
    // Parse the query to get the chirpid
    int secondnewline = query.find('\n', newline+1);
    int valuelength = secondnewline - newline - 1;
    int chirpid = atoi(query.substr(newline+1, valuelength).c_str());
    // Delete the chirp based on the chirpid
    deleteChirp(username, chirpid, mappingMutex, fileMutexes);
}

///////////////////////////////////////////////////////////////////////////////
// Query  9: ADDFND (Add Friend)
///////////////////////////////////////////////////////////////////////////////

void addFriend (int newline, const string& query, const string& username, 
                char buff[MAXLINE], int connfd) {
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

///////////////////////////////////////////////////////////////////////////////
// Query 10: DELFND (Delete Friend)
///////////////////////////////////////////////////////////////////////////////

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

void deleteFriendParse (int newline, const string& query, const string& username, 
                        char buff[MAXLINE], int connfd) {
    string fileName = "users/" + username + ".txt";
    int secondnewline = query.find('\n', newline+1);
    int friendlength = secondnewline - newline - 1;
    string friendName = query.substr(newline+1, friendlength);
    deleteFriend(fileName, friendName);
    string temp = "YES";
    sendMessage(temp, buff, connfd);
}

///////////////////////////////////////////////////////////////////////////////
// Query 11: POPLAT (Populate Page)
///////////////////////////////////////////////////////////////////////////////

void checkValidFriends (const string& fileName, std::mutex* userManifestMutex) {
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
            if (!checkUser(friendsList[i], userManifestMutex)) {
                deleteFriend(fileName, friendsList[i]);
            }
        }
    }
}

void populatePage (const string& username, char buff[MAXLINE], int connfd, 
                   char readbuff[MAXLINE], std::mutex* userManifestMutex) {
    string fileName = "users/" + username + ".txt";
    // This will first make sure that the friend's list is valid
    checkValidFriends(fileName, userManifestMutex);
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

///////////////////////////////////////////////////////////////////////////////
// Query 12: MOVEUP (Move Friend Up)
///////////////////////////////////////////////////////////////////////////////

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

void moveUserUpParse (int newline, const string& query, const string& username, 
                      char buff[MAXLINE], int connfd) {
    string fileName = "users/" + username + ".txt";
    int secondnewline = query.find('\n', newline+1);
    int valuelength = secondnewline - newline - 1;
    int userid = atoi(query.substr(newline+1, valuelength).c_str());
    moveUserUp(fileName, userid);
    string temp = "YES";
    sendMessage(temp, buff, connfd);
}

///////////////////////////////////////////////////////////////////////////////
// Query 13: MOVEDN (Move Friend Down)
///////////////////////////////////////////////////////////////////////////////

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

void moveUserDownParse (int newline, const string& query, const string& username, 
                        char buff[MAXLINE], int connfd) {
    string fileName = "users/" + username + ".txt";
    int secondnewline = query.find('\n', newline+1);
    int valuelength = secondnewline - newline - 1;
    int userid = atoi(query.substr(newline+1, valuelength).c_str());
    moveUserDown(fileName, userid);
    string temp = "YES";
    sendMessage(temp, buff, connfd);
}
