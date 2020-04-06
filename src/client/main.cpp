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

    unsigned short message_idx{0};
    unsigned int port1{1234};
    string server1{"localhost"};
    unsigned int port2{1235};
    string server2{"localhost"};
    bool verbose{false};
    bool list{false};

    auto list_option = app.add_flag("-l,--list", list, "List all message titles");
    auto index_option = app.add_option("-i,--index", message_idx, "Index of the message to retrieve");
    app.add_option("-p,--port1", port1, "First port of the server from which to retrieve messages");
    app.add_option("-s,--server1", server1, "First IP address of the server from which to retrieve messages");
    auto port2_option = app.add_option("-q,--port2", port2, "Second port of the server from which to retrieve messages");
    auto server2_option = app.add_option("-t,--server2", server2, "Second IP address of the server from which to retrieve messages");
    app.add_flag("-v,--verbose", verbose, "Print additional debug messages");

    list_option->excludes(index_option);
    list_option->excludes(port2_option);
    list_option->excludes(server2_option);

    CLI11_PARSE(app, argc, argv);

    if (verbose) {
        spdlog::set_level(spdlog::level::debug);
    } else {
        spdlog::set_level(spdlog::level::info);
    }

    vector<const char*> answers;

    vector<int> idx{message_idx};

    vector<string> ports{to_string(port1), to_string(port2)};
    vector<string> servers{server1, server2};

    bool first_server{true};
    int idx_servers{0};
    for (const string& port : ports) {
        tcp::iostream strm{servers.at(idx_servers), port};
        if (strm) {
            spdlog::debug("connection to localhost:{} created", port);

            if (list) {
                strm << "req list" << endl;
                string line{};
                getline(strm, line);
                string end;
                end = (char)4;
                int idx{};

                cout << setw(5) << "idx" << ": " << "title" << endl;

                while (line != end) {
                    cout << setw(5) << idx << ": " << line << endl;
                    idx++;
                    getline(strm, line);
                }
                exit(0);
            }

            std::stringstream ss;
            size_t i{0};

            vector<int> idx_{};

            if (first_server) {
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
            } else {
                idx_.push_back(idx.at(0));
            }

            for (int i{1}; i < idx.size(); i++) {
                idx_.push_back(idx.at(i));
            }

            auto rng = default_random_engine {};
            shuffle(begin(idx_), end(idx_), rng);

            for (int i{0}; i < idx_.size(); ++i) {
                ss << idx_[i];
                if (i != idx_.size() - 1) {
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
        idx_servers++;
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
