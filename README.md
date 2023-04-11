# Private Personalized Advertisement using Private Information Retrieval

## Introduction

In this repository, we present a demonstration of how private information retrieval
can be used to serve up personalized advertisements, without letting the
centralized ad server collect data about the user's ad preferences.

For a brief introduction to the design of the system, refer to [this document](./DESIGN.md).

### Installing SEAL

SEAL is a homomorphic encryption library developed by Microsoft. It is the key
dependency for our PIR system. Follow the instructions [here](https://github.com/microsoft/SEAL/tree/4.0.0#getting-started).

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
