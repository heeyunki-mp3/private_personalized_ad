#include "user.h"

namespace user {

UserProgram::UserProgram() {
    // Assume the groupings are known to any computer that uses the ad server
    //
    // In our example:
    //      0 -- Cooking ads
    //      1 -- Book ads
    //      2 -- Sports ads
    //      3 -- Movie ads
    for (unsigned int i = 0; i < 4; ++i)
        cnts_[i] = 0;
}

UserProgram::~UserProgram() {
    // Nothing to do here
}

unsigned int UserProgram::getGroupFromAdNumber(unsigned int no) {
    // Assume this mapping is known to any computer that uses the ad server
    //
    // In our example:
    //      0-99: Ad group 0
    //      100-199: Ad group 1
    //      200-299: Ad group 2
    //      300-399: Ad group 3
    return no / 100;
}

void UserProgram::run() {
    // Obtain initial ad set
    obtainInitialAdSet();

    // Runs forever
    unsigned int userSelection;
    while (true) {
        // Interface with user
        std::cout << "Here are the ads we have for you!" << std::endl;
        for (auto a : currAds_) {
            std::cout << "Ad " << a.first << ": ";
            a.second.printAd();
            std::cout << std::endl;
        }
        std::cout << "Click on an ad by entering its number: " << std::endl;
        std::cin >> userSelection;
        while (true) {
            if ((currAds_.count(userSelection)) != 0)
                break;
            std::cout << "That was not a valid ad number: " << std::endl;
            std::cin >> userSelection;
        }

        // Update cnt_
        updateCnts(userSelection);
        updateAdSet();
        printCnts();    // For testing
    }
}

uint64_t UserProgram::generateRandomInt64() {
    return 0;
}

void UserProgram::obtainInitialAdSet() {
    // TODO: Use SealPIR to update currAds_
    // Currently, we just add one ad for each category
    for (unsigned int i = 0; i < 400; i += 100) {
        Advertisement ad(generateRandomInt64());
        currAds_.insert({ i, ad });
    }
}

void UserProgram::updateAdSet() {
    // TODO: Use SealPIR to update currAds_
    // Currently, we don't make any updates
}

unsigned int UserProgram::getMostPopularAdGroup() {
    unsigned int mostPopularGrp;
    unsigned int mostPopularCnt;

    mostPopularGrp = mostPopularCnt = 0;    // Note slight bias toward group 0
    for (auto p : cnts_) {
        if (p.second > mostPopularCnt) {
            mostPopularCnt = p.second;
            mostPopularGrp = p.first;
        }
    }

    return mostPopularGrp;
}

void UserProgram::updateCnts(unsigned int userSelection) {
    ++cnts_[getGroupFromAdNumber(userSelection)];
}

void UserProgram::printCnts() {
    std::cout << "Printing counts: " << std::endl;
    for (auto a : cnts_) {
        std::cout << "Ad group " << a.first << ": " << a.second << std::endl;
    }
    // For visual separation on the terminal
    std::cout << std::endl << std::endl;
}

} // namespace user
