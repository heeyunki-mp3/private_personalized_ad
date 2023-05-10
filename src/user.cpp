#include "user.hpp"

namespace user {

/* GENERAL */

template<typename POD>
void deserialize_vector(std::vector<POD> vec, char *buffer){
    static_assert(std::is_trivial<POD>::value && std::is_standard_layout<POD>::value,
        "Can only deserialize POD types with this function");
    unsigned long size;
    unsigned long ele_size;
    int offset = 0;

    memcpy(&size, buffer, sizeof(unsigned long));
    offset += sizeof(unsigned long);
    memcpy(&ele_size, buffer+offset, sizeof(unsigned long));
    offset += sizeof(unsigned long);

    for (int i=0; i<size; i++) {
        POD element_hold;
        memcpy(&element_hold, buffer + offset, ele_size);
        offset += ele_size;
        vec.push_back(element_hold);
    }
}

UserProgram::UserProgram(seal::EncryptionParameters encryptionParams,
                         sealpir::PirParams pirParams) 
    : pirclient_(encryptionParams, pirParams), 
      cnts_({}) {
    // Assume the groupings are known to any computer that uses the ad server
    //
    // In our example:
    //      0 -- Cooking ads
    //      1 -- Book ads
    //      2 -- Sports ads
    //      3 -- Movie ads
    //      ...
    //      49 -- Toy ads
    for (int i = 0; i < 50; ++i)
        cnts_[i] = 0;
}

UserProgram::~UserProgram() {
    // Nothing to do here
}

void UserProgram::_run(ModeType modeType) {
    // Obtain initial ad set
    // This is already done in doSetup() if running with server
    if (modeType == local)
        obtainInitialAdSetLocal();

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

void UserProgram::updateAdSetServer() {

}

/* FOR SETUP WITH SERVER */

void UserProgram::doSetup(char *hostname, char *port) {
    // Set up socket connection
    doSocketConnection(hostname, port);
    doEncryptionSetup();
    obtainInitialAdSetServer();
}

// References source code at https://www.linuxhowtos.org/C_C++/socket.htm
void UserProgram::doSocketConnection(char *hostname, char *port) {
    int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

    portno = atoi(port);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
		BOOST_LOG_TRIVIAL(error) << "Client: Could not open socket";
        exit(0);
    }
	server = gethostbyname(hostname);
	if (server == NULL) {
		BOOST_LOG_TRIVIAL(error) << "Client: No such host";
		exit(0);
	}
    bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *) server->h_addr,
	      (char *) &serv_addr.sin_addr.s_addr,
	      server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		BOOST_LOG_TRIVIAL(error) << "Client: Could not connect to server socket";
        exit(0);
    }

    // Update socketfd 
    socketfd_ = sockfd;
    BOOST_LOG_TRIVIAL(info) << "Client: Initial connection successful!";
}

void UserProgram::doEncryptionSetup() {
    int nBytes;
	seal::EncryptionParameters enc_params(seal::scheme_type::bfv);
	sealpir::PirParams pir_params;
    sealpir::PirParams pir_param_object;
    seal::GaloisKeys galois_keys;
    char buf[2048];
    unsigned long enc_size;
    void *pir_vector_buffer;
	unsigned long pir_nvec_len = 0;
    std::string paramString = "";
    void *enc_param_buffer = malloc(sizeof(seal::EncryptionParameters));
	void *pir_param_buffer = malloc(sizeof(sealpir::PirParams));
    if (!enc_param_buffer || !pir_param_buffer) {
        if (enc_param_buffer) free(enc_param_buffer);
        if (pir_param_buffer) free(pir_param_buffer);
        if (pir_vector_buffer) free(pir_vector_buffer);
        close(socketfd_);
        BOOST_LOG_TRIVIAL(error) << "Client: Encryption setup failed";
        exit(0);
    }

    // Tell server we want to do the encryption setup
    nBytes = write(socketfd_, "connect", 7);
	if (nBytes < 0) {
		BOOST_LOG_TRIVIAL(error) << "Client: Could not write \"connect\" to socket";
        if (enc_param_buffer) free(enc_param_buffer);
        if (pir_param_buffer) free(pir_param_buffer);
        if (pir_vector_buffer) free(pir_vector_buffer);
        close(socketfd_);
        BOOST_LOG_TRIVIAL(error) << "Client: Encryption setup failed";
        exit(0);
    }

    // Read parameters from server
    // Encryption parameters first, PIR parameters after
    while((nBytes = read(socketfd_, buf, 255)) == 255) {
        paramString.append(buf);
    }
    paramString.append(buf);
    BOOST_LOG_TRIVIAL(info) << "Client: Obtained paramString from server";

    // // Deserialize parameters
    // // TODO: WHY BUF?
    // std::memcpy(enc_param_buffer, buf, sizeof(seal::EncryptionParameters));
	// std::memcpy(pir_param_buffer, buf + sizeof(seal::EncryptionParameters), sizeof(sealpir::PirParams));

    // // Initialize PIRClient instance
    // pirclient_ = sealpir::PIRClient(*((seal::EncryptionParameters *) enc_param_buffer), *((sealpir::PirParams *) pir_param_buffer));
    // BOOST_LOG_TRIVIAL(info) << "Client: Initialized PIRClient with actual params";
   
    // Copy params
    bzero(buf, 2048);
	std::memcpy(&enc_size, buf, sizeof(unsigned long));
	std::memcpy(enc_param_buffer, buf + sizeof(unsigned long), enc_size);
	std::memcpy(pir_param_buffer, buf + sizeof(unsigned long) + enc_size, sizeof(sealpir::PirParams));
	std::memcpy(&pir_nvec_len, buf + sizeof(unsigned long) + enc_size + sizeof(sealpir::PirParams), sizeof(unsigned long));
	pir_vector_buffer = malloc(pir_nvec_len);
    if (!pir_vector_buffer) {
        if (enc_param_buffer) free(enc_param_buffer);
        if (pir_param_buffer) free(pir_param_buffer);
        if (pir_vector_buffer) free(pir_vector_buffer);
        close(socketfd_);
        BOOST_LOG_TRIVIAL(error) << "Client: Encryption setup failed";
        exit(0);
    }
	std::memcpy(pir_vector_buffer, buf + sizeof(unsigned long)+ enc_size + sizeof(sealpir::PirParams) + sizeof(unsigned long), pir_nvec_len);
    BOOST_LOG_TRIVIAL(info) << "Client: Copied params";
	
	std::memcpy(&pir_param_object, pir_param_buffer, sizeof(sealpir::PirParams));
	std::vector<std::uint64_t> nvec_hold;
	deserialize_vector(nvec_hold, (char *) pir_vector_buffer);
	BOOST_LOG_TRIVIAL(info) << "Client: Deserialized vector";
	pir_param_object.nvec = nvec_hold;

	EncryptionParameters enc_param_object;
	std::stringstream enc_stream;
    enc_stream << enc_param_buffer << endl;
    BOOST_LOG_TRIVIAL(info) << "Client: Placed into enc_stream";
    enc_param_object.load(reinterpret_cast<const seal_byte *>(enc_param_buffer), enc_size);
	
    BOOST_LOG_TRIVIAL(info) << "Client: Initializing client object";
    sealpir::PIRClient client(enc_param_object, pir_param_object);
    BOOST_LOG_TRIVIAL(info) << "Client: DONE!";





    // Generate galois keys
    galois_keys = pirclient_.generate_galois_keys();

    // Send galois keys to server
    bzero(buf, 256);
    std::memcpy(buf, &galois_keys, sizeof(seal::GaloisKeys));
    nBytes = write(socketfd_, buf, sizeof(seal::GaloisKeys));
    if (nBytes != sizeof(seal::GaloisKeys)) {
        BOOST_LOG_TRIVIAL(error) << "Client: Could not send galois keys to server";
        goto error_cleanup;
    }

    // Check if server acknowledged
    nBytes = read(socketfd_, buf, 255);
	if (buf[0] != 'A' || buf[0] != 'C' || buf[0] != 'K') {
		BOOST_LOG_TRIVIAL(error) << "Client: Server did not ACK encryption setup";
        goto error_cleanup;
	}

    BOOST_LOG_TRIVIAL(info) << "Client: Encryption setup complete";
    return;

error_cleanup:
    if (enc_param_buffer) free(enc_param_buffer);
    if (pir_param_buffer) free(pir_param_buffer);
    if (pir_vector_buffer) free(pir_vector_buffer);
    close(socketfd_);
    BOOST_LOG_TRIVIAL(error) << "Client: Encryption setup failed";
    exit(0);
}

void UserProgram::obtainInitialAdSetServer() {
    int nBytes, startByte;
    char buf[512], tmpad[33], tmpno[6];
    
    // Tell server we want to obtain the initial ad set
    nBytes = write(socketfd_, "obtain", 6);
	if (nBytes < 0) {
		BOOST_LOG_TRIVIAL(error) << "Client: Could not write \"obtain\" to socket";
        exit(0);
    }

    // Read initial ad set from server
    nBytes = read(socketfd_, buf, 512);
    if (nBytes < 370) {         // 32 bytes for ad, 5 bytes for ad number
        BOOST_LOG_TRIVIAL(error) << "Client: Could not read initial ad set from server";
        exit(0);
    }

    // Set up initial ad set
    vector<Advertisement> ads;  // Ordered
    vector<unsigned int> adNos; // Ordered

    // Parse advertisemeent
    for (int i = 0; i < 10; ++i) {
        startByte = i * 32;
        bzero(tmpad, 33);
        for (int j = 0; j < 32; ++j)
            tmpad[j] = buf[startByte + j];
        Advertisement ad(tmpad);
        ads.push_back(ad);
    }

    // Parse ad numbers
    for (int i = 0; i < 10; ++i) {
        startByte = 320 + i * 5;
        bzero(tmpno, 6);
        for (int j = 0; j < 6; ++j)
            tmpno[j] = buf[startByte + j];
        unsigned int adNo = (unsigned int) atoi(tmpno);
        adNos.push_back(adNo);
    }

    // Fill currAds_
    for (int i = 0; i < 10; ++i)
        currAds_.insert({ adNos[i], ads[i] });

    BOOST_LOG_TRIVIAL(info) << "Client: Initial ad set obtained";
}

/* FOR LOCAL TESTING */

void UserProgram::obtainInitialAdSetLocal() {
    // Add one ad for each category
    for (unsigned int i = 0; i < 400; i += 100) {
        const char *adString = "abcdefghijklmnopqrstuvwxyzabcdef";  // 32 chars
        Advertisement ad(adString);
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
