#include <iostream>
#include <asio.hpp>

#include <spdlog/spdlog.h>

#include "bit_operations.h"

using namespace std;
using namespace asio::ip;

unsigned int message_len{280};

int main() {
    vector<const char*> answers;

    // first element gets retrieved
    vector<int> idx{5, 11, 8};
    vector<string> ports{"1234", "1235"};

    bool first_server{true};

    for (const string& port : ports) {
        tcp::iostream strm{"localhost", port};
        if (strm) {
            spdlog::info("connection created");

            std::stringstream ss;
            size_t i{0};
            if (first_server) {
                i++;
                first_server = false;
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
            spdlog::info("received data");

            char* answer = new char[message_len + 1];
            memcpy(answer, data.c_str(), message_len + 1);
            answers.push_back(answer);
            strm.close();
            spdlog::info("connection closed");
        } else {
            spdlog::error("Could not connect to server!");
            exit(1);
        }
    }

    char* output;
    output = xor_string(answers.at(0), answers.at(1));
    for (int i{2}; i < answers.size(); i++) {
        output = xor_string(output, answers.at(i));
    }

    cout.write(output, message_len);
    cout << endl;
    delete output;

    for (const char* a : answers) {
        delete a;
    }

    return 0;
}
