#ifndef QUERIES
#define QUERIES

#include <string>
#include <mutex>
#include <unordered_map>
#include "mappings.h"
using namespace std;

// Extern global mutexes, for separate compilation
extern mutex mappingMutex;
extern mutex userManifestMutex;
extern mutex emailManifestMutex;

// Query  1: CHKEML (Check Email)
string checkEmail (const string& emailToFind);

// Query  2: CHKUSR (Check User)
bool checkUser (const string& username);

string checkUserParse (const string& username);

// Query  3: CHKPWD (Check Password)
string checkPassword (int newline, const string& query, const string& username,
                      unordered_map<string, mutex*>& fileMutexes);

// Query  4: CHKFND (Check Friend)
bool checkFriend (const string& fileName, const string& friendName,
                  unordered_map<string, mutex*>& fileMutexes);

string checkFriendParse (int newline, const string& query, const string& username,
                        unordered_map<string, mutex*>& fileMutexes);

// Query  5: CRTUSR (Create User)
string createUser (int newline, const string& query, const string& username,
                   unordered_map<string, mutex*>& fileMutexes);

// Query  6: DELUSR (Delete User)
string deleteUser (const string& username, unordered_map<string, mutex*>& fileMutexes);

// Query  7: CRTCHP (Create Chirp)
string createChirp (int newline, const string& query, const string& username,
                    unordered_map<string, mutex*>& fileMutexes);

// Query  8: DELCHP (Delete Chirp)
void deleteChirp (const string& fileName, int chirpid,
                  unordered_map<string, mutex*>& fileMutexes);

string deleteChirpParse (int newline, const string& query, const string& username,
                         unordered_map<string, mutex*>& fileMutexes);

// Query  9: ADDFND (Add Friend)
string addFriend (int newline, const string& query, const string& username,
                  unordered_map<string, mutex*>& fileMutexes);

// Query 10: DELFND (Delete Friend)
void deleteFriend (const string& fileName, const string& friendName,
                   unordered_map<string, mutex*>& fileMutexes);

string deleteFriendParse (int newline, const string& query, const string& username,
                          unordered_map<string, mutex*>& fileMutexes);

// Query 11: POPLAT (Populate Page)
void checkValidFriends (const string& fileName, unordered_map<string, mutex*>& fileMutexes);

string populatePage (const string& username, unordered_map<string, mutex*>& fileMutexes);

// Query 12: MOVEUP (Move Friend Up)
void moveUserUp (const string& fileName, int userid, unordered_map<string, mutex*>& fileMutexes);

string moveUserUpParse (int newline, const string& query, const string& username,
                        unordered_map<string, mutex*>& fileMutexes);

// Query 13: MOVEDN (Move Friend Down)
void moveUserDown (const string& fileName, int userid, unordered_map<string, mutex*>& fileMutexes);

string moveUserDownParse (int newline, const string& query, const string& username,
                          unordered_map<string, mutex*>& fileMutexes);

#endif
