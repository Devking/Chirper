#include "api_mapping.h"

#include <unordered_map>
#include <string>
#include <string.h> // strlen
#include <unistd.h> // write

// A mapping for convenience for possible queries defined by the API
void initAPIMapping (std::unordered_map<std::string, int>& actions) {
    actions["CHKEML"] = CHKEML;
    actions["CHKUSR"] = CHKUSR;
    actions["CHKPWD"] = CHKPWD;
    actions["CHKFND"] = CHKFND;
    actions["CRTUSR"] = CRTUSR;
    actions["DELUSR"] = DELUSR;
    actions["CRTCHP"] = CRTCHP;
    actions["DELCHP"] = DELCHP;
    actions["ADDFND"] = ADDFND;
    actions["DELFND"] = DELFND;
    actions["POPLAT"] = POPLAT;
    actions["MOVEUP"] = MOVEUP;
    actions["MOVEDN"] = MOVEDN;
}

void sendMessage (const std::string& returnString, char buff[MAXLINE], int connfd) {
    const char* thing = returnString.c_str();
    sprintf(buff, "%s", thing);
    int len = strlen(buff);
    if (len != write(connfd, buff, strlen(buff))) {
        perror("write to connection failed");
    }
}
