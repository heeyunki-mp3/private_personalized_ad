#ifndef USER_USER_HPP_
#define USER_USER_HPP_

#include <iostream>
#include <unordered_map>
#include "ad.hpp"

namespace user {

// Program running on user's local computer
//
// Displays ads to the user
// Collects the user's ad preferences
// Interfaces with the centralized ad server to obtain personalized ads
class UserProgram {

private:
    std::unordered_map<unsigned int, unsigned int> cnts_;       // K: Ad group, V: No. clicks
    std::unordered_map<unsigned int, Advertisement> currAds_;   // K: Ad number, V: Ad    

    // TODO: Some data member for interfacing with the server via SealPIR

public:
    UserProgram();
    ~UserProgram();
    unsigned int getGroupFromAdNumber(unsigned int no);
    void run();
    uint64_t generateRandomInt64();
    void obtainInitialAdSet();     // Interfaces with server
    void updateAdSet();         // Interfaces with server
    unsigned int getMostPopularAdGroup();
    void updateCnts(unsigned int userSelection);
    void printCnts();           // Used for testing

}; // class UserProgram

} // namespace user 

#endif // #ifndef USER_USER_HPP_
