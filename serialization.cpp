


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

std::string serialize_galoiskeys(const GaloisKeys& sealobj) {
    std::stringstream stream;
    sealobj.save(stream);
    return stream.str();
}
GaloisKeys *deserialize_galoiskeys(string s, SEALContext *context) {
  GaloisKeys *g = new GaloisKeys();
  std::istringstream input(s);
  g->load(*context, input);
  return g;
}

template<typename POD>
unsigned long get_size_of_serialized_vector(std::vector<POD> const& v){
    return sizeof(v[0]) * v.size() + sizeof(unsigned long) * 2;
}


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
	cout << "in my print" << endl;

	std::__1::vector<uint64_t> paramVect = pir_params.nvec;
	cout << "I have vector" << endl;
	for (auto paramV : paramVect) {
		cout << "it vec" << paramV << endl;
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