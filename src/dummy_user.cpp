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
	void *pir_vector_buffer;
	unsigned long pir_nvec_len = 0;

	// Generates all parameters
	//------ Socket -------
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[2048];
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
	cout << "Socket: wrote connect to the server" << endl;
	bzero(buffer, 2048);
	// n = read(sockfd, buffer, 2047);

	std::string paramString = "";
	FILE *socketFile = fdopen(sockfd, "r");
	std::cout << "Socket: opened read socket file" << endl;

	n = read(sockfd, buffer, 2047);

	while (n == 2047) {
		paramString.append(buffer);
		std::cout << "buffer: " << paramString << endl;
		hexDump(buffer, 2047);
		n = read(sockfd, buffer, 2047);
	}
	paramString.append(buffer);

	std::cout << "Socket: recieved param" << endl;

	std::cout << "Main: Initializing pir client"
		  << "Enc size: " << sizeof(EncryptionParameters) << " pir size: " << sizeof(PirParams) << endl;

	// copying the received information to local buffers
	unsigned long enc_size;
	::memcpy(&enc_size, buffer, sizeof(unsigned long));

	::memcpy(enc_param_buffer, buffer+sizeof(unsigned long), enc_size);
	::memcpy(pir_param_buffer, buffer +sizeof(unsigned long)+enc_size, sizeof(PirParams));
	::memcpy(&pir_nvec_len, buffer +sizeof(unsigned long)+enc_size + sizeof(PirParams), sizeof(unsigned long));
	pir_vector_buffer = malloc(pir_nvec_len);
	printf("pir allocated at: %p with %ld size\n", pir_vector_buffer, pir_nvec_len);
	::memcpy(pir_vector_buffer, buffer +sizeof(unsigned long)+enc_size + sizeof(PirParams) + sizeof(unsigned long), pir_nvec_len);

	std::cout << "Main: copied params" << endl;
	std::cout << "vector buffer:" << endl;

	// turning the buffered bits into objects
	// deserialzing pir object
	PirParams pir_param_object;
	memcpy(&pir_param_object, pir_param_buffer, sizeof(PirParams));
	std::cout << "Main: copied pir param" << endl;

	// deserializing pir.nvec
	std::vector<std::uint64_t> nvec_hold;

	deserialize_vector(nvec_hold, (char *)pir_vector_buffer);
	std::cout << "Main: deserialized vector" << endl;
	pir_param_object.nvec = nvec_hold;

	// my_print_pir_params(pir_param_object);

	EncryptionParameters enc_param_object;
	std::stringstream enc_stream;
    enc_stream << enc_param_buffer << endl;

    enc_param_object.load(reinterpret_cast<const seal_byte *>(enc_param_buffer), enc_size);

	PIRClient client(enc_param_object, pir_param_object);

	std::cout << "Main: created galois key" << endl;
	GaloisKeys galois_keys = client.generate_galois_keys();
	bzero(buffer, 2048);

	std::cout << "Socket: sending galois key" << endl;
	std::string galois_str = serialize_galoiskeys(galois_keys);
	unsigned long gal_len = galois_str.size();

	std::cout << "Socket: sending galois key len" << endl;

	::memcpy(buffer, &gal_len, sizeof(unsigned long));
	
	n = write(sockfd, buffer, sizeof(unsigned long));
	std::cout << "Socket: sent galois key len" << endl;

	n = read(sockfd, buffer, 2047);
	if (buffer[0] == 'A' && buffer[1] == 'C' && buffer[2] == 'K') {
		std::cout << "Socket: received ACK for Galois key length" << endl;
	}
	std::cout << "Socket: sending galois key of length "<< gal_len << endl;

	n = write(sockfd, galois_str.data(), gal_len);

	std::cout <<"Socekt: done sending galois key"<<endl;

	n = read(sockfd, buffer, 2047);
	if (buffer[0] == 'A' && buffer[1] == 'C' && buffer[2] == 'K') {
		std::cout << "Main: done setting up!! yay" << endl;
	}
	// RYAN: ready to send  query
	close(sockfd);
	std::cout << "Main: closed socket"<<endl;
	free(enc_param_buffer);
	free(pir_param_buffer);
	free(pir_vector_buffer);
	std::cout << "Main: freed malloc'd"<<endl;

	return 0;
	//----- socket ends -----
}