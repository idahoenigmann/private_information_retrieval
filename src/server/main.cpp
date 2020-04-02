#include <iostream>
#include <cstring>
#include <asio.hpp>
#include <vector>
#include <CLI11.hpp>

#include "bit_operations.h"

using namespace std;
using namespace asio::ip;

int main(int argc, char* argv[]) {
    CLI::App app{"Private Information Retrieval Server"};

    unsigned short port;
    app.add_option("-p,--port", port, "Port on which to send data on")->required(true);

    CLI11_PARSE(app, argc, argv);

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

    asio::io_context ctx;
    tcp::endpoint ep{tcp::v4(), port};
    cout << "connection openend" << endl;
    tcp::acceptor acceptor{ctx, ep};
    cout << "listening" << endl;
    acceptor.listen();

    tcp::socket sock{ctx};
    acceptor.accept(sock);
    tcp::iostream strm{std::move(sock)};

    if (strm) {
        string data;
        getline(strm, data);

        size_t start = 0;
        size_t end = 0;

        end = data.find(',', start);
        int first = stoi(data.substr(start, end - start));
        start = end + 1;
        end = data.find(',', start);
        int second = stoi(data.substr(start, end - start));

        char* output{xor_string(v.at(first), v.at(second))};

        while ((start = data.find_first_not_of(',', end)) != std::string::npos)
        {
            end = data.find(',', start);
            int message_idx = stoi(data.substr(start, end - start));
            output = xor_string(output, v.at(message_idx));
        }
        cout << "sending" << endl;
        cout << output << endl;
        strm << output << endl;
        delete output;
        strm.close();
    } else {
        cout << "could not open stream" << endl;
    }
    cout << "connection closed" << endl;

    return 0;
}
