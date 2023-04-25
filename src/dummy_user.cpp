#include "pir.hpp"
#include "pir_client.hpp"
#include "pir_server.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <random>
#include <seal/seal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std::chrono;
using namespace std;
using namespace seal;
void hexDump(void *addr, int len)
{
	int i;
	unsigned char buff[17];
	unsigned char *pc = (unsigned char *)addr;
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

void my_print_pir_params(const PirParams &pir_params)
{
    cout<< "in my print"<< endl;

    std::__1::vector<uint64_t> paramVect = pir_params.nvec;
    cout << "I have vector" <<endl;
    for(auto paramV : paramVect){
        cout << "it vec"<< paramV <<endl;
    }
	std::uint32_t prod = accumulate(pir_params.nvec.begin(), pir_params.nvec.end(), 1,
	    multiplies<uint64_t>());


	cout << "PIR Parameters" << endl;
	cout << "number of elements: " << pir_params.ele_num << endl;
	cout << "element size: " << pir_params.ele_size << endl;
	cout << "elements per BFV plaintext: " << pir_params.elements_per_plaintext
	     << endl;
	cout << "dimensions for d-dimensional hyperrectangle: " << pir_params.d
	     << endl;
	cout << "number of BFV plaintexts (before padding): "
	     << pir_params.num_of_plaintexts << endl;
	cout << "Number of BFV plaintexts after padding (to fill d-dimensional "
		"hyperrectangle): "
	     << prod << endl;
	cout << "expansion ratio: " << pir_params.expansion_ratio << endl;
	cout << "Using symmetric encryption: " << pir_params.enable_symmetric << endl;
	cout << "Using recursive mod switching: " << pir_params.enable_mswitching
	     << endl;
	cout << "slot count: " << pir_params.slot_count << endl;
	cout << "==============================" << endl;
}

void error(const char *msg)
{
	perror(msg);
	exit(0);
}
int query_test(uint64_t num_items, uint64_t item_size, uint32_t degree,
    uint32_t lt, uint32_t dim);

int main(int argc, char *argv[])
{

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

	// Generates all parameters

	//------ Socket -------
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[256];
	if (argc < 3) {
		fprintf(stderr, "usage %s hostname port\n", argv[0]);
		exit(0);
	}
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	server = gethostbyname(argv[1]);
	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
	    (char *)&serv_addr.sin_addr.s_addr,
	    server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR connecting");

	// cONNECT

	n = write(sockfd, "connect", 7);
	if (n < 0)
		error("ERROR writing to socket");
	cout << "Socket: wrote connet to the server" << endl;
	bzero(buffer, 256);
	// n = read(sockfd, buffer, 255);

	std::string paramString = "";
	FILE *socketFile = fdopen(sockfd, "r");
	std::cout << "Socket: opened read socket file" << endl;
	/*while (fgets(buffer, 255, socketFile)) {
		paramString.append(buffer);
	std::cout << "buffer: "<< paramString<< endl;
	hexDump(buffer, 255);
	}*/

	n = read(sockfd, buffer, 255);

	while (n == 255) {
		paramString.append(buffer);
		std::cout << "buffer: " << paramString << endl;
		hexDump(buffer, 255);
		n = read(sockfd, buffer, 255);
	}
	paramString.append(buffer);
	std::cout << "buffer: " << paramString << endl;
	hexDump(buffer, 255);

	std::cout << "Socket: recieved param" << endl;

	std::cout << "Main: Initializing pir client"
		  << "Enc size: " << sizeof(EncryptionParameters) << " pir size: " << sizeof(PirParams) << endl;

	::memcpy(enc_param_buffer, buffer, sizeof(EncryptionParameters));
	::memcpy(pir_param_buffer, buffer + sizeof(EncryptionParameters), sizeof(PirParams));

	std::cout << "Main: copied params" << endl;
	PirParams *pir_param_object;
	memcpy(pir_param_object, pir_param_buffer, sizeof(PirParams));
	hexDump(pir_param_object, sizeof(PirParams));

	my_print_pir_params(*pir_param_object);
	fprintf(stderr, "done printing\n");
	PIRClient client(*(EncryptionParameters *)enc_param_buffer, *(PirParams *)pir_param_buffer);

	std::cout << "Main: created galois key" << endl;
	GaloisKeys galois_keys = client.generate_galois_keys();
	bzero(buffer, 256);

	std::cout << "Socket: sending galois key" << endl;
	::memcpy(buffer, &galois_keys, sizeof(GaloisKeys));
	n = write(sockfd, buffer, sizeof(GaloisKeys));

	n = read(sockfd, buffer, 255);
	if (buffer[0] == 'A' && buffer[0] == 'C' && buffer[0] == 'K') {
		std::cout << "Main: done setting up!! yay" << endl;
	}

	close(sockfd);
	return 0;
	//----- socket ends -----
	/*



		// Set galois key for client with id 0
		cout << "Main: Setting Galois keys...";
		server.set_galois_key(0, galois_keys);

		// Measure database setup
		server.set_database(move(db), number_of_items, size_per_item);
		server.preprocess_database();
		cout << "Main: database pre processed " << endl;

		// Choose an index of an element in the DB
		uint64_t ele_index = rd() % number_of_items; // element in DB at random position
		uint64_t index = client.get_fv_index(ele_index); // index of FV plaintext
		uint64_t offset = client.get_fv_offset(ele_index); // offset in FV plaintext
		cout << "Main: element index = " << ele_index << " from [0, "
		     << number_of_items - 1 << "]" << endl;
		cout << "Main: FV index = " << index << ", FV offset = " << offset << endl;

		// Measure query generation
		PirQuery query = client.generate_query(index);
		cout << "Main: query generated" << endl;

		// To marshall query to send over the network, you can use
		// serialize/deserialize: std::string query_ser = serialize_query(query);
		// PirQuery query2 = deserialize_query(d, 1, query_ser, CIPHER_SIZE);

		// Measure query processing (including expansion)
		PirReply reply = server.generate_reply(query, 0);

		// Measure response extraction
		vector<uint8_t> elems = client.decode_reply(reply, offset);

		assert(elems.size() == size_per_item);

		bool failed = false;
		// Check that we retrieved the correct element
		for (uint32_t i = 0; i < size_per_item; i++) {
			if (elems[i] != db_copy.get()[(ele_index * size_per_item) + i]) {
				cout << "Main: elems " << (int)elems[i] << ", db "
				     << (int)db_copy.get()[(ele_index * size_per_item) + i] << endl;
				cout << "Main: PIR result wrong at " << i << endl;
				failed = true;
			}
		}
		if (failed) {
			return -1;
		}

		return 0;
	    */
}