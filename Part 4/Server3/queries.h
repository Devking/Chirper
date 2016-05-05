#ifndef QUERIES
#define QUERIES

#include <string>
#include <mutex>
#include <unordered_map>

#include "mappings.h"

// Query  1: CHKEML (Check Email)
void checkEmail        (const std::string& emailToFind, char buff[MAXLINE], int connfd, 
                        std::mutex* emailManifestMutex);

// Query  2: CHKUSR (Check User)
bool checkUser         (const std::string& username, std::mutex* userManifestMutex);

// Query  3: CHKPWD (Check Password)
void checkPassword     (int newline, const std::string& query, const std::string& username, 
                        char buff[MAXLINE], int connfd, std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

// Query  4: CHKFND (Check Friend)
bool checkFriend       (const std::string& fileName, const std::string& friendName, 
                        std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

void checkFriendParse  (int newline, const std::string& query, const std::string& username, 
                        char buff[MAXLINE], int connfd, std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

// Query  5: CRTUSR (Create User)
void createUser        (int newline, const std::string& query, const std::string& username, 
                        char buff[MAXLINE], int connfd, std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes,
                        std::mutex* emailManifestMutex, std::mutex* userManifestMutex);

// Query  6: DELUSR (Delete User)
void deleteUser        (const std::string& username, char buff[MAXLINE], int connfd, 
                        std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes,
                        std::mutex* emailManifestMutex, std::mutex* userManifestMutex);

// Query  7: CRTCHP (Create Chirp)
void createChirp       (int newline, const std::string& query, const std::string& username, 
                        char buff[MAXLINE], int connfd, std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

// Query  8: DELCHP (Delete Chirp)
void deleteChirp       (const std::string& fileName, int chirpid, std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

void deleteChirpParse  (int newline, const std::string& query, const std::string& username, 
                        char buff[MAXLINE], int connfd, std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

// Query  9: ADDFND (Add Friend)
void addFriend         (int newline, const std::string& query, const std::string& username, 
                        char buff[MAXLINE], int connfd, std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

// Query 10: DELFND (Delete Friend)
void deleteFriend      (const std::string& fileName, const std::string& friendName, 
                        std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

void deleteFriendParse (int newline, const std::string& query, const std::string& username, 
                        char buff[MAXLINE], int connfd, std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

// Query 11: POPLAT (Populate Page)
void checkValidFriends (const std::string& fileName, std::mutex* userManifestMutex, 
                        std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

void populatePage      (const std::string& username, char buff[MAXLINE], int connfd, 
                        char readbuff[MAXLINE], std::mutex* userManifestMutex, 
                        std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

// Query 12: MOVEUP (Move Friend Up)
void moveUserUp        (const std::string& fileName, int userid, std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

void moveUserUpParse   (int newline, const std::string& query, const std::string& username, 
                        char buff[MAXLINE], int connfd, std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

// Query 13: MOVEDN (Move Friend Down)
void moveUserDown      (const std::string& fileName, int userid, std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

void moveUserDownParse (int newline, const std::string& query, const std::string& username, 
                        char buff[MAXLINE], int connfd, std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

#endif
