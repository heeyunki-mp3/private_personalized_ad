#include "user.hpp"

int main(int argc, char **argv) {
    if (argc < 3) {
		fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
		exit(0);
	}

    // Generate dummy parameters to initialize the user program
    // These dummy parameters should be overriden when the user program
    // does setup with the server.
    seal::EncryptionParameters enc_params(seal::scheme_type::bfv);
    sealpir::PirParams pir_params;
    uint64_t number_of_items = 1 << 16;
    uint64_t size_per_item = 1024;
    uint32_t N = 4096;
    uint32_t logt = 20;
    uint32_t d = 2;
    bool use_symmetric = true;
    bool use_batching = true;
    bool use_recursive_mod_switching = true;
    gen_pir_params(number_of_items, size_per_item, d, enc_params, pir_params,
                 use_symmetric, use_batching, use_recursive_mod_switching);

    // Initialize and run user program
    user::UserProgram program(enc_params, pir_params);
    // program.runLocal();
    program.runServer(argv[1], argv[2]);
    return 0;
}
