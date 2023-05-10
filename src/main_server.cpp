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
#include "serialization.h"
using namespace seal;
using namespace std::chrono;
using namespace std;

void error(const char *msg)
{
	perror(msg);
	exit(1);
}
void hexDump(void *addr, int len) 
{
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf("  %s\n", buff);

            // Output the offset.
            printf("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf(" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
            buff[i % 16] = '.';
        } else {
            buff[i % 16] = pc[i];
        }

        buff[(i % 16) + 1] = '\0';
    }
    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf("   ");
        i++;
    }
    // And print the final ASCII bit.
    printf("  %s\n", buff);
}
/**
 * serialization:
 * /------sizeof(unsigned long) * 2 -------/
 *   [size of vector]  [size of element]  [element 1][element 2]...
 * 
*/
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
template<typename POD>
void serialize_vector(std::vector<POD> vec, char *buffer){
    static_assert(std::is_trivial<POD>::value && std::is_standard_layout<POD>::value,
        "Can only serialize POD types with this function");
    unsigned long size = vec.size();
    unsigned long ele_size = sizeof(vec[0]);
    int offset = 0;
    memcpy(buffer, &size, sizeof(unsigned long));
    offset += sizeof(unsigned long);
    memcpy(buffer+offset, &ele_size, sizeof(unsigned long));
    offset += sizeof(unsigned long);
    for (auto element : vec) {
        memcpy(buffer+offset, &element, ele_size);
        offset += ele_size;
    }
}

string serialize_galoiskeys(Serializable<GaloisKeys> g) {
  std::ostringstream output;
  g.save(output);
  return output.str();
}

GaloisKeys *deserialize_galoiskeys(string s, SEALContext *context, GaloisKeys *g) {
  std::istringstream input(s);
  g->load(*context, input);
  return g;
}

template<typename POD>
unsigned long get_size_of_serialized_vector(std::vector<POD> const& v){
    return sizeof(v[0]) * v.size() + sizeof(unsigned long) * 2;
}

int main(int argc, char *argv[])
{
    int64_t num_items = 1<<10;
    uint64_t item_size=288;
    uint32_t degree=4096;

	uint64_t number_of_items = num_items;
	uint64_t size_per_item = item_size; // in bytes
	uint32_t N = degree;

	// Recommended values: (logt, d) = (12, 2) or (8, 1).
    uint32_t lt = 20;
    uint32_t dim = 1;

	uint32_t logt = lt;
	uint32_t d = dim;

	EncryptionParameters enc_params(scheme_type::bfv);
	sealpir::PirParams pir_params;

	// Generates all parameters

	std::cout << "Main: Generating SEAL parameters" << endl;
	sealpir::gen_encryption_params(N, logt, enc_params);

	std::cout << "Main: Verifying SEAL parameters" << endl;
	sealpir::verify_encryption_params(enc_params);

	std::cout << "Main: SEAL parameters are good" << endl;

	std::cout << "Main: Generating PIR parameters" << endl;
	gen_pir_params(number_of_items, size_per_item, d, enc_params, pir_params);
	print_pir_params(pir_params);

	std::cout << "Main: Initializing the database (this may take some time) ..." << endl;
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
	std::cout << "Main: Initializing pir server" << endl;
	sealpir::PIRServer server(enc_params, pir_params);


    void *query = malloc(sizeof(PirQuery));

//############### copied to while loop from here #################//
	
	// Measure database setup

	server.set_database(move(db), number_of_items, size_per_item);
	server.preprocess_database();
	std::cout << "Main: database pre processed " << endl;


	//------------------------server socket-------------------------
	std::cout << "Main: Initializing the server socket" << endl;

	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char buffer[2048];
    char buffer_sending[2048];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}
    std::cout << "Socket: port recognized" << endl;


	// create a socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
    std::cout << "Socket: opened new socket" << endl;

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

	if ((::bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
		error("ERROR on binding");
    std::cout << "Socket: binding successful"<< endl;

	while (true) {
        std::cout << "Socket: listening to incoming connection"<< endl;
		listen(sockfd, 5);
		// The accept() call actually accepts an incoming connection
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
		if (newsockfd < 0)
			error("ERROR on accept");

		printf("server: got connection from %s port %d\n",
		    inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

		// This send() function sends the 13 bytes of the string to the new socket
		bzero(buffer, 2047);
		n = read(newsockfd, buffer, 2047);
		if (n < 0)
			error("ERROR reading from socket");
		printf("Client input: %s\n", buffer);

		// buffer is the communication from the user
        if (strcmp(buffer, "connect") == 0){
            std::cout << "Registering" << endl;
        }
        else {
            std::cout <<"Wrong request" <<endl;
            continue;
        }
		//TODO: figure out if this serialization works
        // sending encrypt param and PIR param in char array
        //serialzing vector pir_param.nvec

        std::cout << "pir param nvec: ";
        for(auto i: pir_params.nvec)
            std::cout << i << ", ";
        std::cout << endl;

        unsigned long vec_size = get_size_of_serialized_vector(pir_params.nvec);

        void *buffer_vector = malloc(vec_size);
        serialize_vector(pir_params.nvec, (char *)buffer_vector);

        /**
         * communication format:
         * enc_size (unsigned long)
         * enc_param (serialized)
         * pir_param (serialized)
         * pir_param.nvec length (unsigned long)
         * pir_param.nvec (serialized vector)
         * /r/n/0
         * 
        */

        std:cout << "Main: serializing enc stream"<<endl;
        std::stringstream enc_stream;
        unsigned long enc_size = enc_params.save(enc_stream);

        const std::string tmp_enc = enc_stream.str();
        const char* enc_buffer = tmp_enc.c_str();
        std::cout << "Main: serialized enc stream: size = " << enc_size<<endl;

        bzero(buffer_sending, 2048);
        ::memcpy(buffer_sending, &enc_size, sizeof(unsigned long));
        ::memcpy(buffer_sending+sizeof(unsigned long), enc_buffer, enc_size);
        ::memcpy(buffer_sending+sizeof(unsigned long)+enc_size, &pir_params, sizeof(pir_params));
        ::memcpy(buffer_sending+sizeof(unsigned long)+enc_size+sizeof(pir_params), &(vec_size), sizeof(unsigned long)); // tells the size of vector
        ::memcpy(buffer_sending+sizeof(unsigned long)+enc_size+sizeof(pir_params)+sizeof(unsigned long), buffer_vector, vec_size);
        ::memcpy(buffer_sending+sizeof(unsigned long)+enc_size+sizeof(pir_params)+sizeof(unsigned long)+vec_size, "\r\n\0",2);
        std::cout<< "Main: enc param:"<<endl;

        std::cout << "Main: copied the parameters\n"<< "    size of enc param: "<<enc_size<< "\n  size of pir param: "<<sizeof(pir_params)<< "\n  size of nvector:" << vec_size << endl;
        std::cout << "Socket: sending the parameters"<< endl;
		send(newsockfd, buffer_sending, enc_size+sizeof(pir_params)+vec_size+2, 0);
        std::cout << "Socket: params sent"<< endl;

        //reads Galois key from client
        std::cout << "Socket: reading Galois key len from client"<< endl;

        bzero(buffer, 2048);
		n = read(newsockfd, buffer, 2048);
		if (n < 0)
			error("ERROR reading from socket"); 
        std::cout << "Socket: read length of gal key"<< endl;
        unsigned long gal_len = 0;

        ::memcpy(&gal_len, buffer, sizeof(unsigned long));
        std::cout << "Main: creating buffer"<< endl;
        char *galois_keys_buffer = (char *)malloc(gal_len);
        send(newsockfd, "ACK", 4, 0);


        std::cout << "Socket: reading gal key"<< endl;
        int total_read =0;
        while(total_read < gal_len){
            n = read(newsockfd, galois_keys_buffer+total_read, gal_len);
            total_read += n;
            std::cout << "Socket: just read gal key of length: "<< n << " total: " << total_read<< endl;

        }
        std::cout << "Socket: done reading gal key of length: "<< total_read<< endl;

        SEALContext context(enc_params);
        std::cout << "Main: made context"<< endl;

        std::string gal_str(galois_keys_buffer, gal_len);
        std::cout << "Main: created string"<< endl;
        GaloisKeys *galois_key = new GaloisKeys();
        std::cout << "Main: created empty gal key"<< endl;

        deserialize_galoiskeys(gal_str, &context, galois_key);

        std::cout << "Main: copied Galois key from client"<< endl;

        // Set galois key for client with id 0
        std::cout << "Main: Setting Galois keys...";
        server.set_galois_key(0, *galois_key); 
        std::cout << "Main: Done initializing the user!!!!! YAY"<< endl;

        // ack the client that it is done initializing
        send(newsockfd, "ACK", 4, 0);


        // RYAN: Now, it is ready to accept the query request
        // Below is the code I might want to use for querying
        /*

        n = read(newsockfd, buffer, 2047);
		if (n < 0)
			error("ERROR reading from socket"); 
        
        std::stringstream query_stream;
        query_stream << buffer;
        PirQuery query_object = server.deserialize_query(query_stream);
        PirReply reply = server.generate_reply(query_object, 0);
        
        std::stringstream server_stream;
        int reply_size=server.serialize_reply(reply, server_stream);

        const std::string tmp = server_stream.str();
        const char* main_stream_buffer = tmp.c_str();

		send(newsockfd, main_stream_buffer, reply_size ,0);
        std::cout << "Main: Done sending the ad!"<< endl;
        */

		close(newsockfd);
	}
	close(sockfd);

    ::free(query);
	return 0;
}