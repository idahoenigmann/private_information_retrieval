#include <iostream>
#include <asio.hpp>
#include <CLI11.hpp>
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
    app.add_option("-i,--index", message_idx, "Index of the message to retrieve")->required(true);

    CLI11_PARSE(app, argc, argv);

    vector<const char*> answers;

    // first element gets retrieved
    vector<int> idx{message_idx};

    for (int i{}; i < 3; i++) {
        int num{message_idx};
        while (find(idx.begin(), idx.end(), num) != idx.end()) {
            random_device seeder;
            mt19937 engine(seeder());
            uniform_int_distribution<int> dist(0, 11);
            num = dist(engine);
        }
        idx.push_back(num);
    }

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
