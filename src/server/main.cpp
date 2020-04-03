#include <iostream>
#include <cstring>
#include <asio.hpp>
#include <vector>
#include "CLI/CLI.hpp"
#include <spdlog/spdlog.h>

#include "bit_operations.h"

using namespace std;
using namespace asio::ip;

unsigned int message_len{280};

int main(int argc, char* argv[]) {
    CLI::App app{"Private Information Retrieval Server"};

    unsigned short port;
    bool verbose{false};

    app.add_option("-p,--port", port, "Port on which to send data on")->required(true);
    app.add_flag("-v,--verbose", verbose, "Print additional debug messages");

    CLI11_PARSE(app, argc, argv);

    if (verbose) {
        spdlog::set_level(spdlog::level::debug);
    } else {
        spdlog::set_level(spdlog::level::info);
    }

    vector<char*> v;
    vector<string> titles;

    ifstream ifstrm;
    ifstrm.open("../src/server/data.txt");
    if (ifstrm) {
        string line;
        while (getline(ifstrm, line)) {
            titles.push_back(line);
            getline(ifstrm, line);
            char *c = new char[message_len + 1];
            strcpy(c, line.c_str());
            cleanup_char_arr(c, message_len + 1);
            v.push_back(c);
        }
    } else {
        spdlog::error("could not find data.txt");
        exit(2);
    }
    ifstrm.close();

    spdlog::debug("loaded {} messages", v.size());

    asio::io_context ctx;
    tcp::endpoint ep{tcp::v4(), port};
    spdlog::debug("connection opened");
    tcp::acceptor acceptor{ctx, ep};
    spdlog::debug("listening");
    acceptor.listen();

    tcp::socket sock{ctx};
    acceptor.accept(sock);
    tcp::iostream strm{std::move(sock)};

    if (strm) {
        string data;
        getline(strm, data);

        if (data == "req message cnt") {
            strm << v.size() << endl;
            getline(strm, data);
        } else if (data == "req list") {
            for (string title : titles) {
                strm << title << endl;
            }
            strm << (char)4 << endl;
            exit(0);
        }

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
            output = xor_string(output, v.at(message_idx), message_len + 1);
        }
        spdlog::debug("sending {}", data);
        strm.write(output, message_len + 1);
        delete output;
        strm.close();
    } else {
        spdlog::error("could not open stream");
    }
    spdlog::debug("connection closed");

    return 0;
}
