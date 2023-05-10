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
    : cnts_({}) {
    // Assume the groupings are known to any computer that uses the ad server
    //
    // In our example, there are 10 groups
    //      0 -- Cooking ads
    //      1 -- Book ads
    //      2 -- Sports ads
    //      3 -- Movie ads
    //      ...
    //      9 -- Toy ads
    pirclient_ = NULL;
    for (int i = 0; i < 10; ++i)
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
    BOOST_LOG_TRIVIAL(info) << "Client: Entering infinite loop";
    while (true) {
        // Interface with user
        std::cout << endl << endl;  // For spacing
        std::cout << "Here are the ads we have for you!" << std::endl;
        for (auto a : currAds_) {
            std::cout << "Ad " << a.first << " (" << getGroupFromAdNumber(a.first)
                << "): ";
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
        else {
            unsigned int mostPopularGrp = getMostPopularAdGroup();
            unsigned int adRequested = rand() % 5000 + mostPopularGrp * 5000;
            // updateAdSetServer(adRequested);      // No longer need this here
        }
        printCnts();    // For testing
    }
}

void UserProgram::runLocal() {
    ModeType modeType = local;
    _run(modeType);
}

void UserProgram::runWithServer(char *hostname, char *port) {
    doSetup(hostname, port);        // TODO: Refactor (currently does everything)
    // ModeType modeType = server;
    // _run(modeType);
}

/* UTILITY FUNCTIONS */

unsigned int UserProgram::getGroupFromAdNumber(unsigned int no) {
    // Assume this mapping is known to any computer that uses the ad server
    //
    // In our example:
    //      0-4999: Ad group 0
    //      5000-9999: Ad group 1
    //      10000-14999: Ad group 2
    //      ...
    //      45000-49999: Ad group 9
    return no / 5000;
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

void UserProgram::removeLeastPopularAd() {
    BOOST_LOG_TRIVIAL(info) << "Client: Removing least popular ad";

    // Find least popular ad group
    unsigned int leastPopularGrp, leastPopularCnt;
    leastPopularGrp = 0;         // Note slight bias toward group 0
    leastPopularCnt = INT_MAX;
    for (auto p : cnts_) {
        if (p.second < leastPopularCnt) {
            leastPopularCnt = p.second;
            leastPopularGrp = p.first;
        }
    }

    // We only want to remove the second ad onwards
    bool foundOneAdAlr = false;
    bool erasedAnAd = false;
    unsigned int erasedAd;
    for (auto ad : currAds_) {
        if (getGroupFromAdNumber(ad.first) == leastPopularGrp) {
            if (foundOneAdAlr) {
                erasedAnAd = true;
                erasedAd = ad.first;
                break;
            } else {
                foundOneAdAlr = true;
            }
        }
    }

    if (erasedAnAd) {
        currAds_.erase(erasedAd);
        BOOST_LOG_TRIVIAL(info) << "Client: Erased ad number " << erasedAd;
    } else {
        BOOST_LOG_TRIVIAL(info) << "Client: Could not find a least popular ad to erase";
    }
}

void UserProgram::updateAdSetServer(seal::EncryptionParameters enc_params, unsigned int adRequested) {
    /* Request ad from server */

    PirQuery query = pirclient_->generate_query(adRequested);
    std::cout << std::endl;
    std::cout << std::endl;
    BOOST_LOG_TRIVIAL(info) << "Client: Query generated for ad number " << adRequested;
    uint64_t index = pirclient_->get_fv_index(adRequested);
    uint64_t offset = pirclient_->get_fv_offset(adRequested);
    char buffer[4096];
    bzero(buffer, 4096);
    stringstream client_stream;
    pirclient_->generate_serialized_query(index, client_stream);
    const std::string tmp = client_stream.str();
    const char *tmp_buffer;
    unsigned long tmp_len;
    tmp_buffer = tmp.data();
    tmp_len = tmp.size();
    // hexDump((char *)tmp_buffer,tmp_len);

    write(socketfd_, tmp_buffer, tmp_len);
    BOOST_LOG_TRIVIAL(info) << "Client: Sent query to server";

    bzero(buffer, 4096);
    unsigned long n;
    BOOST_LOG_TRIVIAL(info) << "Client: receiving reply size";
    int reply_size =0;
   
    n = read(socketfd_, &reply_size, sizeof(int));
    char *reply_buffer = (char *)malloc(reply_size);
    write(socketfd_, "ACK", 3);

    BOOST_LOG_TRIVIAL(info) << "Client: receiving actual reply ";
    n = read(socketfd_, reply_buffer, reply_size);
    std::string reply_str(reply_buffer, reply_size);
    
    if (n < 0) {
        BOOST_LOG_TRIVIAL(error) << "Client: Could not read from socket";
        exit(0);
    }
    BOOST_LOG_TRIVIAL(info) << "Client: deserializing actual reply ";

    // Deserialize reply
    std::stringstream istream;
    istream << reply_str;
    PirReply reply;
    
    // ryan: get enc_param for this line
    SEALContext context(enc_params);

    while (istream.rdbuf()->in_avail() > 0) {
        seal::Ciphertext c;
        c.load(context, istream);
        reply.push_back(std::move(c));
    }

    // context: the seal context initialized with parameters matching  SealPIRseal
    // e.g. seal::SEALContext(seal::EncryptionParameters{seal::scheme_type::bfv}, true)
    vector<uint8_t> elems = pirclient_->decode_reply(reply, offset);
    std::string adString = "";
    for (auto a : elems) {
        adString += (char) a;
    }
    BOOST_LOG_TRIVIAL(info) << "Client: Received ad: " << adString;
    BOOST_LOG_TRIVIAL(info) << "Client: Ad number: " << adRequested;
    Advertisement adtmp(adString);
    currAds_.insert({ adRequested, adtmp });

    removeLeastPopularAd();
}

/* FOR SETUP WITH SERVER */

// TODO: Currently does the running
void UserProgram::doSetup(char *hostname, char *port) {
    // Set up socket connection
    doSocketConnection(hostname, port);
    doEncryptionSetup();            // TODO: Currently does the running
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

// TODO: Refactor (this currently does all the running too)
void UserProgram::doEncryptionSetup() {
    int n;
    int64_t num_items = 1 << 10;
	uint64_t item_size = 288;
	uint32_t degree = 4096;

	uint64_t number_of_items = num_items;
	uint64_t size_per_item = item_size; // in bytes
	uint32_t N = degree;

	EncryptionParameters enc_params(scheme_type::bfv);
	PirParams pir_params;

	void *enc_param_buffer = malloc(sizeof(EncryptionParameters));
	void *pir_param_buffer = malloc(sizeof(PirParams));
	void *pir_vector_buffer;
	unsigned long pir_nvec_len = 0;
    char buffer[2048];

	// Connect

	n = write(socketfd_, "connect", 7);
	if (n < 0)
		BOOST_LOG_TRIVIAL(error) << "Client: Error writing to socket";
	BOOST_LOG_TRIVIAL(info) << "Client: Wrote connet to the server";
	bzero(buffer, 2048);
	// n = read(sockfd, buffer, 2047);

	std::string paramString = "";
	FILE *socketFile = fdopen(socketfd_, "r");
	BOOST_LOG_TRIVIAL(info) << "Client: Opened read socket file";

	n = read(socketfd_, buffer, 2047);

	while (n == 2047) {
		paramString.append(buffer);
		// std::cout << "buffer: " << paramString << endl;
		hexDump(buffer, 2047);
		n = read(socketfd_, buffer, 2047);
	}
	paramString.append(buffer);

	// std::cout << "Socket: recieved param" << endl;
	// std::cout << "Main: Initializing pir client"
	// 	  << "Enc size: " << sizeof(EncryptionParameters) << " pir size: " << sizeof(PirParams) << endl;

	// copying the received information to local buffers
	unsigned long enc_size;
	::memcpy(&enc_size, buffer, sizeof(unsigned long));

	::memcpy(enc_param_buffer, buffer + sizeof(unsigned long), enc_size);
	::memcpy(pir_param_buffer, buffer + sizeof(unsigned long) + enc_size, 
        sizeof(PirParams));
	::memcpy(&pir_nvec_len, buffer + sizeof(unsigned long) + enc_size + 
        sizeof(PirParams), sizeof(unsigned long));
	pir_vector_buffer = malloc(pir_nvec_len);
	// printf("pir allocated at: %p with %ld size\n", pir_vector_buffer, pir_nvec_len);
	::memcpy(pir_vector_buffer, buffer + sizeof(unsigned long) + enc_size  +
        sizeof(PirParams) + sizeof(unsigned long), pir_nvec_len);
	// std::cout << "Main: copied params" << endl;
	// std::cout << "vector buffer:" << endl;

	// turning the buffered bits into objects
	// deserialzing pir object
	PirParams pir_param_object;
	memcpy(&pir_param_object, pir_param_buffer, sizeof(PirParams));
	// std::cout << "Main: copied pir param" << endl;

	// deserializing pir.nvec
	std::vector<std::uint64_t> nvec_hold;

	deserialize_vector(nvec_hold, (char *) pir_vector_buffer);
	// std::cout << "Main: deserialized vector" << endl;
	pir_param_object.nvec = nvec_hold;
	// my_print_pir_params(pir_param_object);

	EncryptionParameters enc_param_object;
	std::stringstream enc_stream;
    enc_stream << enc_param_buffer << endl;
    enc_param_object.load(reinterpret_cast<const seal_byte *>(enc_param_buffer), enc_size);

    // Initialize PIRClient
	PIRClient client(enc_param_object, pir_param_object);
    pirclient_ = &client;
    BOOST_LOG_TRIVIAL(info) << "Client: Initialized pirclient_";

	// std::cout << "Main: created galois key" << endl;
	GaloisKeys galois_keys = client.generate_galois_keys();
	bzero(buffer, 2048);

	// std::cout << "Socket: sending galois key" << endl;
	std::string galois_str = serialize_galoiskeys(galois_keys);
	unsigned long gal_len = galois_str.size();

	// std::cout << "Socket: sending galois key len" << endl;

	::memcpy(buffer, &gal_len, sizeof(unsigned long));
	
	n = write(socketfd_, buffer, sizeof(unsigned long));
	BOOST_LOG_TRIVIAL(info) << "Client: Sent galois key length";

	n = read(socketfd_, buffer, 2047);
	if (buffer[0] == 'A' && buffer[1] == 'C' && buffer[2] == 'K') {
		// std::cout << "Socket: received ACK for Galois key length" << endl;
	}
	// std::cout << "Socket: sending galois key of length "<< gal_len << endl;

	n = write(socketfd_, galois_str.data(), gal_len);
	BOOST_LOG_TRIVIAL(info) << "Client: Done sending galois key";

	n = read(socketfd_, buffer, 2047);
	if (buffer[0] == 'A' && buffer[1] == 'C' && buffer[2] == 'K') {
		BOOST_LOG_TRIVIAL(info) << "Client: Setup done";
	} else {
        BOOST_LOG_TRIVIAL(error) << "Client: Setup failed";
        exit(0);
    }

    // TODO: THIS SHOULD BE IN THE RUN FUNCTION
    // THIS SHOULD NOT BE CALLED FROM HERE
    // WE JUST PUT IT HERE BECAUSE OF THE MALLOC ERROR

    obtainInitialAdSetServer();
    unsigned int userSelection;
    BOOST_LOG_TRIVIAL(info) << "Client: Entering infinite loop";
    std::cout << std::endl << std::endl;
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

        // Update adset
        unsigned int mostPopularGrp = getMostPopularAdGroup();
        srand(time(NULL));
        unsigned int adRequested = rand() % 5000 + mostPopularGrp * 5000;
        auto time_pre_r = high_resolution_clock::now();
        updateAdSetServer(enc_param_object, adRequested);
        auto time_post_r = high_resolution_clock::now();
        BOOST_LOG_TRIVIAL(info) << "Client: Total time for SealPIR query: " <<
            duration_cast<microseconds>(time_post_r-time_pre_r).count() / 1000 << " ms";
        cout << std::endl << std::endl;
        printCnts();    // For testing
    }
}

void UserProgram::obtainInitialAdSetServer() {
    BOOST_LOG_TRIVIAL(info) << "Client: Obtaining initial ad set from server";

    int nBytes, startByte;
    char buf[512], tmpad[33], tmpno[6];
    
    // Tell server we want to obtain the initial ad set
    nBytes = write(socketfd_, "OBTAIN", 6);
	if (nBytes < 0) {
		BOOST_LOG_TRIVIAL(error) << "Client: Could not write \"obtain\" to socket";
        exit(0);
    }
    BOOST_LOG_TRIVIAL(info) << "Client: Sent \"obtain\" to server";

    // Read initial ad set from server
    nBytes = read(socketfd_, buf, 512);
    if (nBytes < 370) {         // 32 bytes for ad, 5 bytes for ad number
        BOOST_LOG_TRIVIAL(error) << "Client: Could not read initial ad set from server";
        exit(0);
    }
    BOOST_LOG_TRIVIAL(info) << "Client: Read initial ad set from server";

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
        tmpno[5] = '\0';
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
