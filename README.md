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

Build the executable using [this CMakeLists.txt](./CMakeLists.txt):

```bash
cmake .
cmake --build .
```

The `main` executable will be located in a `bin` directory. Run this executable
using:

```bash
cd bin
./main
```
