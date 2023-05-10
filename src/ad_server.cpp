#include "pir.hpp"
#include "pir_client.hpp"
#include "pir_server.hpp"
#include <arpa/inet.h>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <netinet/in.h>
#include <random>
#include <seal/seal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std::chrono;
using namespace std;
using namespace seal;

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

int query()
{
}

string SEALSerialize(const GaloisKeys& sealobj) {
    std::stringstream stream;
    sealobj.save(stream);

    return stream.str();
}
GaloisKeys SEALDeserialize(const SEALContext& sealctx, const string& in) {
    GaloisKeys out;
    std::stringstream stream;
    stream << in;
    out.load(sealctx, stream);
    return out;
}

int main(int argc, char *argv[])
{
	uint64_t number_of_items = num_items;
	uint64_t size_per_item = item_size; // in bytes
	uint32_t N = degree;

	// Recommended values: (logt, d) = (12, 2) or (8, 1).
	uint32_t logt = lt;
	uint32_t d = dim;

	EncryptionParameters enc_params(scheme_type::bfv);
	PirParams pir_params;

	// Generates all parameters

	cout << "Main: Generating SEAL parameters" << endl;
	gen_encryption_params(N, logt, enc_params);

	cout << "Main: Verifying SEAL parameters" << endl;
	verify_encryption_params(enc_params);

	cout << "Main: SEAL parameters are good" << endl;

	cout << "Main: Generating PIR parameters" << endl;
	gen_pir_params(number_of_items, size_per_item, d, enc_params, pir_params);
	print_pir_params(pir_params);

	cout << "Main: Initializing the database (this may take some time) ..." << endl;
	// Create test database
	auto db(make_unique<uint8_t[]>(number_of_items * size_per_item));

	// Copy of the database. We use this at the end to make sure we retrieved
	// the correct element.
	auto db_copy(make_unique<uint8_t[]>(number_of_items * size_per_item));
	random_device rd;
	for (uint64_t i = 0; i < number_of_items; i++) {
		for (uint64_t j = 0; j < size_per_item; j++) {
			uint8_t val = rd() % 256;
			db.get()[(i * size_per_item) + j] = val;
			db_copy.get()[(i * size_per_item) + j] = val;
		}
	}

	// Initialize PIR Server
	cout << "Main: Initializing pir server" << endl;
	PIRServer server(enc_params, pir_params);

    GaloisKeys *galois_keys = malloc(sizeof(GaloisKeys));
    PirQuery *query = malloc(sizeof(PirQuery));

//############### copied to while loop from here #################//
	
	// Measure database setup

	server.set_database(move(db), number_of_items, size_per_item);
	server.preprocess_database();
	cout << "Main: database pre processed " << endl;


	//------------------------server socket-------------------------
	cout << "Main: Initializing the server socket" << endl;

	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char buffer[256];
    char buffer_sending[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}

	// create a socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");

	// clear address structure
	bzero((char *)&serv_addr, sizeof(serv_addr));

	portno = atoi(argv[1]);

	/* setup the host_addr structure for use in bind call */
	// server byte order
	serv_addr.sin_family = AF_INET;

	// automatically be filled with current host's IP address
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	// convert short integer value for port must be converted into network byte order
	serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR on binding");

	while (true) {
		listen(sockfd, 5);
		// The accept() call actually accepts an incoming connection
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
		if (newsockfd < 0)
			error("ERROR on accept");

		printf("server: got connection from %s port %d\n",
		    inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

		// This send() function sends the 13 bytes of the string to the new socket
		bzero(buffer, 256);
		n = read(newsockfd, buffer, 255);
		if (n < 0)
			error("ERROR reading from socket");
		printf("Client input: %s\n", buffer);
        
		// TODO: buffer is the communication from the user
        if (strcmp(buffer, "connect") == 0){
            cout << "Registering" << endl;
        }
        else {
            cout <<"Wrong request" <<endl;
            continue;
        }
		//TODO: figure out if this serialization works
        // sending encrypt param and PIR param in char array
        bzero(buffer_sending, 256);
        memcpy(buffer_sending, &enc_param, sizeof(enc_param));
        memcpy(buffer_sending+sizeof(enc_param), &pir_params, sizeof(pir_params));
        memcpy(buffer_sending+sizeof(enc_param)+sizeof(pir_param), "\n\0",2);
		send(newsockfd, buffer_sending, sizeof(enc_param)+sizeof(pir_param)+3, 0);

        //reads Galois key from client
        bzero(buffer, 256);
		n = read(newsockfd, buffer, 255);
		if (n < 0)
			error("ERROR reading from socket"); 

        memcpy(galois_keys, buffer, sizeof(GaloisKeys));
        
        // Set galois key for client with id 0
        cout << "Main: Setting Galois keys...";
        server.set_galois_key(0, *galois_keys); 
        cout << "Main: Done initializing the user!!!!! YAY"<< endl;


        n = read(newsockfd, buffer, 255);
		if (n < 0)
			error("ERROR reading from socket"); 
        
        PirReply reply = server.generate_reply(server.deserialize_query(buffer), 0);
        
        stringstream server_stream;
        int reply_size=server.serialize_reply(reply, server_stream);

		send(newsockfd, server_stream, reply_size ,0);
        cout << "Main: Done sending the ad!"<< endl;

		close(newsockfd);
	}
	close(sockfd);
    free(galois_keys);
    free(query);

	return 0;
}