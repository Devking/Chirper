#ifndef QUERIES
#define QUERIES

#include <string>
#include <mutex>

#include "mappings.h"

// Query  1: CHKEML (Check Email)
void checkEmail        (const std::string& emailToFind, char buff[MAXLINE], int connfd, 
                        std::mutex* emailManifestMutex);

// Query  2: CHKUSR (Check User)
bool checkUser         (const std::string& username);

// Query  3: CHKPWD (Check Password)
void checkPassword     (int newline, const std::string& query, const std::string& username, 
                        char buff[MAXLINE], int connfd);

// Query  4: CHKFND (Check Friend)
bool checkFriend       (const std::string& fileName, const std::string& friendName);
void checkFriendParse  (int newline, const std::string& query, const std::string& username, 
                        char buff[MAXLINE], int connfd);

// Query  5: CRTUSR (Create User)
void createUser        (int newline, const std::string& query, const std::string& username, 
                        char buff[MAXLINE], int connfd);

// Query  6: DELUSR (Delete User)
void deleteUser        (const std::string& username, char buff[MAXLINE], int connfd);

// Query  7: CRTCHP (Create Chirp)
void createChirp       (int newline, const std::string& query, const std::string& username, 
                        char buff[MAXLINE], int connfd);

// Query  8: DELCHP (Delete Chirp)
void deleteChirp       (const std::string& fileName, int chirpid);
void deleteChirpParse  (int newline, const std::string& query, const std::string& username, 
                        char buff[MAXLINE], int connfd);

// Query  9: ADDFND (Add Friend)
void addFriend         (int newline, const std::string& query, const std::string& username, 
                        char buff[MAXLINE], int connfd);

// Query 10: DELFND (Delete Friend)
void deleteFriend      (const std::string& fileName, const std::string& friendName);
void deleteFriendParse (int newline, const std::string& query, const std::string& username, 
                        char buff[MAXLINE], int connfd);

// Query 11: POPLAT (Populate Page)
void checkValidFriends (const std::string& fileName);
void populatePage      (const std::string& username, char buff[MAXLINE], int connfd, 
                        char readbuff[MAXLINE]);

// Query 12: MOVEUP (Move Friend Up)
void moveUserUp        (const std::string& fileName, int userid);
void moveUserUpParse   (int newline, const std::string& query, const std::string& username, 
                        char buff[MAXLINE], int connfd);

// Query 13: MOVEDN (Move Friend Down)
void moveUserDown      (const std::string& fileName, int userid);
void moveUserDownParse (int newline, const std::string& query, const std::string& username, 
                        char buff[MAXLINE], int connfd);

#endif
