#include "ad.hpp"

namespace user {

Advertisement::Advertisement(const char *val) {
    snprintf(ad_, 33, "%s", val);
}

Advertisement::~Advertisement() {
    // Nothing to do here
}

// Does not print a new line
void Advertisement::printAd() {
    std::cout << ad_;
}

} // namespace user