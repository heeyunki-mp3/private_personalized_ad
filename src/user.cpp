#include "user.hpp"

namespace user {

/* GENERAL */

UserProgram::UserProgram(seal::EncryptionParameters encryptionParams,
                         sealpir::PirParams pirParams) 
    : pirclient_(encryptionParams, pirParams), 
      cnts_({{0, 0}, {1, 0}, {2, 0}, {3, 0}}) {
    // Assume the groupings are known to any computer that uses the ad server
    //
    // In our example:
    //      0 -- Cooking ads
    //      1 -- Book ads
    //      2 -- Sports ads
    //      3 -- Movie ads
}

UserProgram::~UserProgram() {
    // Nothing to do here
}

void UserProgram::_run(ModeType modeType) {
    // Obtain initial ad set
    if (modeType == local)
        obtainInitialAdSetLocal();
    else
        obtainInitialAdSetServer();

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
        updateCntsFromUserSelection(userSelection);
        if (modeType == local)
            updateAdSetLocal();
        else
            updateAdSetServer();
        printCnts();    // For testing
    }
}

void UserProgram::runLocal() {
    ModeType modeType = local;
    _run(modeType);
}

void UserProgram::runWithServer(char *hostname, char *port) {
    doSetup(hostname, port);
    ModeType modeType = server;
    _run(modeType);
}

/* UTILITY FUNCTIONS */

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

uint64_t UserProgram::generateRandomInt64() {
    return 0;
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

void UserProgram::updateCntsFromUserSelection(unsigned int userSelection) {
    ++cnts_[getGroupFromAdNumber(userSelection)];
}

/* FOR INTERFACING WITH SERVER */

void UserProgram::obtainInitialAdSetServer() {

}   

void UserProgram::updateAdSetServer() {

}

void UserProgram::doSetup(char *hostname, char *port) {
   // Set up socket connection
   doSocketConnection(hostname, port);
}

// References source code at https://www.linuxhowtos.org/C_C++/socket.htm
void UserProgram::doSocketConnection(char *hostname, char *port) {
    int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

    portno = atoi(port);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
		fprintf(stderr, "ERROR: Client: Could not open socket\n");
        exit(0);
    }
	server = gethostbyname(hostname);
	if (server == NULL) {
		fprintf(stderr, "ERROR: Client: No such host\n");
		exit(0);
	}
    bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *) server->h_addr,
	      (char *) &serv_addr.sin_addr.s_addr,
	      server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		fprintf(stderr, "ERROR: Client: Could not connect to server socket\n");
        exit(0);
    }

    // Update socketfd 
    socketfd_ = sockfd;
    std::cout << "Connection successful!\n";
}

/* FOR LOCAL TESTING */

void UserProgram::obtainInitialAdSetLocal() {
    // Add one ad for each category
    for (unsigned int i = 0; i < 400; i += 100) {
        Advertisement ad(generateRandomInt64());
        currAds_.insert({ i, ad });
    }
}

void UserProgram::updateAdSetLocal() {
    // We don't make any updates when testing locally
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
