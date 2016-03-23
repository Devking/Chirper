#ifndef QUERIES
#define QUERIES

#include <string>
#include "api_mapping.h"

bool checkUser         (const std::string& username);

bool checkFriend       (const std::string& fileName, const std::string& friendName);
void deleteFriend      (const std::string& fileName, const std::string& friendName);
void checkValidFriends (const std::string& fileName);
void deleteChirp       (const std::string& fileName, int chirpid);
void moveUserUp        (const std::string& fileName, int userid);
void moveUserDown      (const std::string& fileName, int userid);

void checkEmail        (const std::string& emailToFind, char buff[MAXLINE], int connfd);
void checkPassword     (int newline, const std::string& query, const std::string& username, char buff[MAXLINE], int connfd);
void deleteUser        (const std::string& username, char buff[MAXLINE], int connfd);
void createUser        (int newline, const std::string& query, const std::string& username, char buff[MAXLINE], int connfd);
void createChirp       (int newline, const std::string& query, const std::string& username, char buff[MAXLINE], int connfd);
void checkFriendParse  (int newline, const std::string& query, const std::string& username, char buff[MAXLINE], int connfd);
void deleteChirpParse  (int newline, const std::string& query, const std::string& username, char buff[MAXLINE], int connfd);
void addFriend         (int newline, const std::string& query, const std::string& username, char buff[MAXLINE], int connfd);
void deleteFriendParse (int newline, const std::string& query, const std::string& username, char buff[MAXLINE], int connfd);
void populatePage      (const std::string& username, char buff[MAXLINE], int connfd, char readbuff[MAXLINE]);
void moveUserUpParse   (int newline, const std::string& query, const std::string& username, char buff[MAXLINE], int connfd);
void moveUserDownParse (int newline, const std::string& query, const std::string& username, char buff[MAXLINE], int connfd);

#endif
