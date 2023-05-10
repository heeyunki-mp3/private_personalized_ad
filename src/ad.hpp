#ifndef SRC_AD_HPP_
#define SRC_AD_HPP_

#include <iostream>

namespace user {

// Advertisement
class Advertisement {

private:
    char ad_[33];

public:
    Advertisement(const char *val);
    ~Advertisement();
    void printAd();
}; // class Advertisement

} // namespace user

#endif // #ifndef SRC_AD_HPP_
