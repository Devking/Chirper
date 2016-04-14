#ifndef MAPPING_FILE
#define MAPPING_FILE

#include <unordered_map>
#include <string>

#define MAXLINE  4096    // max text line length
#define SA       struct sockaddr
#define LISTENQ  1024    // 2nd argument to listen() -- size of listening queue
#define PORT_NUM 9000

#define CHKEML 1
#define CHKUSR 2
#define CHKPWD 3
#define CHKFND 4
#define CRTUSR 5
#define DELUSR 6
#define CRTCHP 7
#define DELCHP 8
#define ADDFND 9
#define DELFND 10
#define POPLAT 11
#define MOVEUP 12
#define MOVEDN 13

void initAPIMapping (std::unordered_map<std::string, int>& actions);
void sendMessage (const std::string& returnString, char buff[MAXLINE], int connfd);

#endif
