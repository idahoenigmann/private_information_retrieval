#include <iostream>
#include <asio.hpp>

#include "bit_operations.h"

using namespace std;
using namespace asio::ip;

int main() {
    char a[280] = "Will be starting The White House news conference at 5:15 P.M. Eastern.";
    char b[280] = "My proposal to the politically correct Automobile Companies would lower the average price of a car to consumers by more than $3500, while at the same time making the cars substantially safer. Engines would run smoother. Positive impact on the environment! Foolish executives!";
    char c[280] = "My Administration is helping U.S. auto workers by replacing the failed Obama Emissions Rule. Impossible to satisfy its Green New Deal Standard; Lots of unnecessary and expensive penalties to car buyers!";

    tcp::iostream strm{"localhost", "1234"};
    if (strm) {
        cout << "connection created" << endl;
        string data;
        getline(strm, data);
        cout << "received data" << endl;
        char* output{xor_string(data.c_str(), a)};
        output = xor_string(output, c);

        cout << output << endl;

        strm.close();
        cout << "connection closed" << endl;
    } else {
        cout << "Could not connect to server!" << endl;
    }

    return 0;
}
