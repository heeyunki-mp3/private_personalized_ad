#include "user.hpp"

int main(int argc, char **argv) {
    if (argc < 3) {
		fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
		exit(0);
	}

    // Initialize and run user program
    user::UserProgram program;
    program.runWithServer(argv[1], argv[2]);
    return 0;

    // To test locally:
    // program.runLocal();
}
