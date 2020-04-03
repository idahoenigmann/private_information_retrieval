#include <iostream>
#include <asio.hpp>
#include "CLI/CLI.hpp"
#include <spdlog/spdlog.h>
#include <random>
#include <algorithm>

#include "bit_operations.h"

using namespace std;
using namespace asio::ip;

unsigned int message_len{280};

int main(int argc, char* argv[]) {
    CLI::App app{"privately retrieve messages"};

    unsigned short message_idx;
    unsigned int port1{1234};
    unsigned int port2{1235};
    bool verbose{false};

    app.add_option("-i,--index", message_idx, "Index of the message to retrieve")->required(true);
    app.add_option("-p,--port1", port1, "First port of the server from which to retrieve messages");
    app.add_option("-q,--port2", port2, "Second port of the server from which to retrieve messages");
    app.add_flag("-v,--verbose", verbose, "Print additional debug messages");

    CLI11_PARSE(app, argc, argv);

    if (verbose) {
        spdlog::set_level(spdlog::level::debug);
    } else {
        spdlog::set_level(spdlog::level::info);
    }

    vector<const char*> answers;

    vector<int> idx{message_idx};

    vector<string> ports{to_string(port1), to_string(port2)};

    bool first_server{true};

    for (const string& port : ports) {
        tcp::iostream strm{"localhost", port};
        if (strm) {
            spdlog::debug("connection to localhost:{} created", port);

            std::stringstream ss;
            size_t i{0};
            if (first_server) {
                i++;
                first_server = false;

                strm << "req message cnt" << endl;
                string res;
                getline(strm, res);

                random_device seeder;
                mt19937 engine(seeder());
                uniform_int_distribution<int> dist_idx(0, stoi(res) - 1);
                uniform_int_distribution<int> dist_cnt(5, 10);

                int cnt{dist_cnt(engine)};
                for (int i{}; i < cnt; i++) {
                    int num{message_idx};
                    while (find(idx.begin(), idx.end(), num) != idx.end()) {
                        num = dist_idx(engine);
                    }
                    idx.push_back(num);
                }
            }
            for (; i < idx.size(); ++i) {
                ss << idx[i];
                if (i != idx.size() - 1) {
                    ss << ",";
                }
            }
            strm << ss.str() << endl;
            string data;
            char buffer[message_len + 1];

            while (strm.read(buffer, sizeof(buffer))) {
                data.append(buffer, sizeof(buffer));
            }
            data.append(buffer, strm.gcount());
            spdlog::debug("received data");

            char* answer = new char[message_len + 1];
            memcpy(answer, data.c_str(), message_len + 1);
            answers.push_back(answer);
            strm.close();
            spdlog::debug("connection closed");
        } else {
            spdlog::error("Could not connect to server!");
            exit(1);
        }
    }

    char* output;
    output = xor_string(answers.at(0), answers.at(1), message_len + 1);
    for (int i{2}; i < answers.size(); i++) {
        output = xor_string(output, answers.at(i), message_len + 1);
    }

    cout.write(output, message_len);
    cout << endl;
    delete output;

    for (const char* a : answers) {
        delete a;
    }

    return 0;
}
