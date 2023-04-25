#include "user.hpp"

int main(int argc, char **argv) {
    if (argc < 3) {
		fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
		exit(0);
	}

    // Generate dummy parameters to initialize the user program.
    // These dummy parameters should be overriden when the user program
    // does setup with the server.
    // Referenced from https://github.com/microsoft/SealPIR/blob/master/test/query_test.cpp
    uint64_t number_of_items = 1 << 10;
    uint64_t size_per_item = 288;
    uint32_t N = 4096;
    uint32_t logt = 20;
    uint32_t d = 1;
    seal::EncryptionParameters enc_params(seal::scheme_type::bfv);
    sealpir::PirParams pir_params;
    sealpir::gen_encryption_params(N, logt, enc_params);
    sealpir::verify_encryption_params(enc_params);
    gen_pir_params(number_of_items, size_per_item, d, enc_params, pir_params);
    std::cout << "Dummy parameters generated\n";

    // Initialize and run user program
    user::UserProgram program(enc_params, pir_params);
    // program.runLocal();
    program.runWithServer(argv[1], argv[2]);
    return 0;
}
