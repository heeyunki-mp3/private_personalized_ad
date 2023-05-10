#ifndef SRC_AD_HPP_
#define SRC_AD_HPP_

#include <iostream>

namespace user {

// Advertisement
class Advertisement {

private:
    std::string ad_;

public:
    Advertisement(const char *val);
    Advertisement(std::string);
    ~Advertisement();
    void printAd();
}; // class Advertisement

} // namespace user

#endif // #ifndef SRC_AD_HPP_
