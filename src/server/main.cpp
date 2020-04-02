#include <iostream>
#include <cstring>
#include <asio.hpp>

#include "bit_operations.h"

using namespace std;
using namespace asio::ip;

int main() {
    char a[280] = "Will be starting The White House news conference at 5:15 P.M. Eastern.";
    char b[280] = "My proposal to the politically correct Automobile Companies would lower the average price of a car to consumers by more than $3500, while at the same time making the cars substantially safer. Engines would run smoother. Positive impact on the environment! Foolish executives!";
    char c[280] = "My Administration is helping U.S. auto workers by replacing the failed Obama Emissions Rule. Impossible to satisfy its Green New Deal Standard; Lots of unnecessary and expensive penalties to car buyers!";

    char* output{xor_string(a, b)};
    output = xor_string(output, c);

    asio::io_context ctx;
    tcp::endpoint ep{tcp::v4(), 1234};
    cout << "connection openend" << endl;
    tcp::acceptor acceptor{ctx, ep};
    cout << "listening" << endl;
    acceptor.listen();

    tcp::socket sock{ctx};
    acceptor.accept(sock);
    tcp::iostream strm{std::move(sock)};

    if (strm) {
        cout << "sending" << endl;
        strm << output << endl;
        strm.close();
    } else {
        cout << "could not open stream" << endl;
    }
    cout << "connection closed" << endl;
    delete output;

    return 0;
}
