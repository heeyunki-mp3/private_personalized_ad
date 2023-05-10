#ifndef SERIALIZATION_HPP_
#define SERIALIZATION_HPP_

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
using namespace sealpir;

template<typename POD>
void deserialize_vector(std::vector<POD> vec, char *buffer);

template<typename POD>
void serialize_vector(std::vector<POD> vec, char *buffer);

std::string serialize_galoiskeys(const GaloisKeys& sealobj);

GaloisKeys *deserialize_galoiskeys(string s, SEALContext *context);

template<typename POD>
unsigned long get_size_of_serialized_vector(std::vector<POD> const& v);

void hexDump(void *addr, int len);

void my_print_pir_params(const PirParams &pir_params);

#endif  // SERIALIZATION_HPP_