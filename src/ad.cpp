#include "ad.h"

namespace user {

Advertisement::Advertisement(uint64_t val) {
    ad_ = val;
}

Advertisement::~Advertisement() {
    // Nothing to do here
}

// Does not print a new line
void Advertisement::printAd() {
    std::cout << ad_;
}

} // namespace user