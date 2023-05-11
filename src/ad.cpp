#include "ad.hpp"

namespace user {

Advertisement::Advertisement(const char *val) {
    char ad___[33];
    snprintf(ad___, 33, "%s", val);
    std::string tmp(ad___);
    ad_ = tmp;
}

Advertisement::Advertisement(std::string s) {
    ad_ = s;
}

Advertisement::~Advertisement() {
    // Nothing to do here
}

// Does not print a new line
void Advertisement::printAd() {
    std::cout << ad_;
}

std::string Advertisement::getAdString() {
    return ad_;
}
} // namespace user