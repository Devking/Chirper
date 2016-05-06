#ifndef QUERIES
#define QUERIES

#include <string>
#include <mutex>
#include <unordered_map>

#include "mappings.h"

// Query  1: CHKEML (Check Email)
std::string checkEmail        (const std::string& emailToFind, char buff[MAXLINE], int connfd,
                        std::mutex* emailManifestMutex);

// Query  2: CHKUSR (Check User)
bool checkUser         (const std::string& username, std::mutex* userManifestMutex);

// Query  3: CHKPWD (Check Password)
std::string checkPassword     (int newline, const std::string& query, const std::string& username,
                        char buff[MAXLINE], int connfd, std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

// Query  4: CHKFND (Check Friend)
bool checkFriend       (const std::string& fileName, const std::string& friendName,
                        std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

std::string checkFriendParse  (int newline, const std::string& query, const std::string& username,
                        char buff[MAXLINE], int connfd, std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

// Query  5: CRTUSR (Create User)
std::string createUser        (int newline, const std::string& query, const std::string& username,
                        char buff[MAXLINE], int connfd, std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes,
                        std::mutex* emailManifestMutex, std::mutex* userManifestMutex);

// Query  6: DELUSR (Delete User)
std::string deleteUser        (const std::string& username, char buff[MAXLINE], int connfd,
                        std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes,
                        std::mutex* emailManifestMutex, std::mutex* userManifestMutex);

// Query  7: CRTCHP (Create Chirp)
std::string createChirp       (int newline, const std::string& query, const std::string& username,
                        char buff[MAXLINE], int connfd, std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

// Query  8: DELCHP (Delete Chirp)
void deleteChirp       (const std::string& fileName, int chirpid, std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

std::string deleteChirpParse  (int newline, const std::string& query, const std::string& username,
                        char buff[MAXLINE], int connfd, std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

// Query  9: ADDFND (Add Friend)
std::string addFriend         (int newline, const std::string& query, const std::string& username,
                        char buff[MAXLINE], int connfd, std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

// Query 10: DELFND (Delete Friend)
void deleteFriend      (const std::string& fileName, const std::string& friendName,
                        std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

std::string deleteFriendParse (int newline, const std::string& query, const std::string& username,
                        char buff[MAXLINE], int connfd, std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

// Query 11: POPLAT (Populate Page)
void checkValidFriends (const std::string& fileName, std::mutex* userManifestMutex,
                        std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

std::string populatePage      (const std::string& username, char buff[MAXLINE], int connfd,
                        char readbuff[MAXLINE], std::mutex* userManifestMutex,
                        std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

// Query 12: MOVEUP (Move Friend Up)
void moveUserUp        (const std::string& fileName, int userid, std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

std::string moveUserUpParse   (int newline, const std::string& query, const std::string& username,
                        char buff[MAXLINE], int connfd, std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

// Query 13: MOVEDN (Move Friend Down)
void moveUserDown      (const std::string& fileName, int userid, std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

std::string moveUserDownParse (int newline, const std::string& query, const std::string& username,
                        char buff[MAXLINE], int connfd, std::mutex* mappingMutex,
                        std::unordered_map<std::string, std::mutex*>& fileMutexes);

#endif
