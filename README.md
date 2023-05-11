# Private Personalized Advertisement using Private Information Retrieval

## Introduction

In this repository, we present a demonstration of how private information retrieval (PIR) can be used to serve up personalized advertisements, without letting the centralized ad server collect data about the user's ad preferences.

For a brief introduction to the design of our system, refer to [this document](./DESIGN.md).

Our system uses source code copied directly from SealPIR, which is a PIR research library developed by Microsoft. View the source code [here](https://github.com/microsoft/SealPIR).

### Installing SEAL

SEAL is a homomorphic encryption library developed by Microsoft. It is the key dependency for SealPIR and thus our PIR system. The main source code is located [here](https://github.com/microsoft/SEAL). 

To install SEAL, follow the instructions [here](https://github.com/microsoft/SEAL/tree/4.0.0#getting-started). Note that our program uses SEAL version 4.1.1.

For MacOS users with Homebrew, run:

```bash
brew install seal
```

### Running our program

Build the set of ads using [this python script](./src/create_ads.py):

```bash
python ./src/create_ads.py
```

Build the executable using [this CMakeLists.txt](./CMakeLists.txt):

```bash
cmake .
cmake --build .
```

The executables will be located in a `bin` directory. For demonstration purposes, the client and server run locally and thus communicate using a port number. Note that the server must be created before the client. Run these executables using:

```bash
cd bin
./main_server <port>            # Creates a server instance
./main_user <hostname> <port>   # Creates a client instance
```

## changing the length of the database
If you are changing length of the database please update:
1. `create_ads.py` line 6 
    `for i in range(<# of line >)`

2. `user.hpp` line 26 - 29
    `#define TTL_NUMBER_ADS 50000`
    `#define MAX_DIGITS_AD_NUMBER 6`

3. `pir_param.nvec value`
    Get the nvec value of pir param from server side, and put the number into 
    `  std::vector<std::uint64_t> local_nvec = {160};`
    