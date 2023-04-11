#ifndef USER_AD_H_
#define USER_AD_H_

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

#endif // #ifndef USER_AD_H_
