#include <iostream>
#include <asio.hpp>

#include "bit_operations.h"

using namespace std;
using namespace asio::ip;

int main() {
    char a[280] = "Will be starting The White House news conference at 5:15 P.M. Eastern.";
    char b[280] = "My proposal to the politically correct Automobile Companies would lower the average price of a car to consumers by more than $3500, while at the same time making the cars substantially safer. Engines would run smoother. Positive impact on the environment! Foolish executives!";
    char c[280] = "My Administration is helping U.S. auto workers by replacing the failed Obama Emissions Rule. Impossible to satisfy its Green New Deal Standard; Lots of unnecessary and expensive penalties to car buyers!";

    cleanup_char_arr(a);
    cleanup_char_arr(b);
    cleanup_char_arr(c);

    vector<char*> v;
    v.push_back(a);
    v.push_back(b);
    v.push_back(c);

    // first element gets retrieved
    vector<int> idx;
    idx.push_back(2);
    idx.push_back(0);
    idx.push_back(1);


    tcp::iostream strm{"localhost", "1234"};
    if (strm) {
        cout << "connection created" << endl;

        std::stringstream ss;
        for(size_t i = 0; i < idx.size(); ++i)
        {
            if(i != 0) {
                ss << ",";
            }
            ss << idx[i];
        }
        strm << ss.str() << endl;
        string data;
        getline(strm, data);
        cout << "received data" << endl;
        char* output;
        bool first = true;
        for (int i{1}; i < idx.size(); i++) {
            if (first) {
                output = xor_string(data.c_str(), v.at(idx.at(i)));
                first = false;
            } else {
                output = xor_string(output, v.at(idx.at(i)));
            }
        }
        cout << output << endl;
        delete output;

        strm.close();
        cout << "connection closed" << endl;
    } else {
        cout << "Could not connect to server!" << endl;
    }

    return 0;
}
