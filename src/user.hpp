#ifndef SRC_USER_HPP_
#define SRC_USER_HPP_

#include <chrono>
#include <iostream>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <unordered_map>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "ad.hpp"
#include "pir_client.hpp"
#include "custom_serialization.hpp"

#include <boost/log/trivial.hpp>

using namespace seal;
using namespace sealpir;

namespace user {

enum ModeType {
    local,
    server,
};

// Program running on user's local computer
//
// Displays ads to the user
// Collects the user's ad preferences
// Interfaces with the centralized ad server to obtain personalized ads
class UserProgram {

private:
    std::unordered_map<unsigned int, unsigned int> cnts_;       // K: Ad group, V: Number clicks
    std::unordered_map<unsigned int, Advertisement> currAds_;   // K: Ad number, V: Ad
    sealpir::PIRClient *pirclient_;
    int socketfd_;

public:
    /* General */
    UserProgram(seal::EncryptionParameters, sealpir::PirParams);
    ~UserProgram();
    void _run(ModeType modeType);                           // Not exposed
    void runLocal();                                        // Exposed
    void runWithServer(char *hostname, char *port);         // Exposed

    /* Utility functions */
    unsigned int getGroupFromAdNumber(unsigned int);
    unsigned int getMostPopularAdGroup();
    void updateCntsFromUserSelection(unsigned int);

    /* For interfacing with server */   
    void updateAdSetServer(seal::EncryptionParameters, unsigned int);

    /* For setup with server */
    void doSetup(char *hostname, char *port);
    void doSocketConnection(char *hostname, char *port);
    void doEncryptionSetup();
    void removeLeastPopularAd();
    void obtainInitialAdSetServer();  

    /* For local testing */
    void obtainInitialAdSetLocal();
    void updateAdSetLocal();
    void printCnts();
}; // class UserProgram

} // namespace user 

#endif // #ifndef SRC_USER_HPP_
