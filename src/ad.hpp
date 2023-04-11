#ifndef SRC_AD_HPP_
#define SRC_AD_HPP_

#include <iostream>

namespace user {

// Advertisement
class Advertisement {

private:
    uint64_t ad_;

public:
    Advertisement(uint64_t val);
    ~Advertisement();
    void printAd();
}; // class Advertisement

} // namespace user

#endif // #ifndef SRC_AD_HPP_
