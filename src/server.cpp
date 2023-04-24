#include "ad.hpp"
#include <fstream>
#include <vector>

using namespace std;

int main()
{
    string file_name = "ads.csv";

    vector<string> ads;
    string line;
    uint64_t i = 0;

    fstream file(file_name, ios::in);
    if (file.is_open()) {
        while (getline(file, line)) {
            ads.push_back(line);
            ++i;
        }
    } else {
        cout<<"Could not open the file\n";
    }

    uint64_t number_of_items = i;
    uint64_t size_per_item = 32; // string in bytes

    // db initialization
    auto db(make_unique<uint8_t[]>(number_of_items * size_per_item));
    for (uint64_t i = 0; i < number_of_items; ++i) {
        for (uint64_t j = 0; j < size_per_item; ++j) {
            if (j < ads[i].size()) {
                db.get()[(i * size_per_item) + j] = ads[i][j];
            } else {
                db.get()[(i * size_per_item) + j] = 0;
            }
        }
    }

    return 0;
}
