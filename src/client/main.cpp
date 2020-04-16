#include <random>

#include <asio.hpp>
#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>

#include "bit_operations.h"
#include "exit_codes.h"

using namespace std;
using namespace asio::ip;

unsigned int message_len{280};

int main(int argc, char* argv[]) {
    CLI::App app{"privately retrieve messages"};

    /* Specify command line options */

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

    /* set verbose level */

    if (verbose) {
        spdlog::set_level(spdlog::level::debug);
    } else {
        spdlog::set_level(spdlog::level::info);
    }

    /* setup vectors for later use */

    vector<const char*> responses;
    vector<int> req_idx{message_idx};
    vector<pair<string, string>> endpoints{make_pair(server1, to_string(port1)), make_pair(server2, to_string(port2))};

    /* setup random engine for later use */

    random_device seeder;
    mt19937 rndm_engine{seeder()};

    /* loop through both endpoints */

    for (int idx_server{0}; idx_server < 2; idx_server++) {

        /* create stream to endpoint */
        tcp::iostream strm{endpoints.at(idx_server).first, endpoints.at(idx_server).second};
        if (strm) {     /* stream could be created */
            spdlog::debug("connection to localhost:{} created", endpoints.at(idx_server).second);

            if (list) {     /* list cli option was specified */
                strm << "req list" << endl;
                string line{};
                if (!strm) {
                    spdlog::error("connection to {}:{} closed unexpectedly", endpoints.at(idx_server).first, endpoints.at(idx_server).second);
                    exit(EXIT_UNEXPECTED_CLOSE);
                }
                getline(strm, line);
                string end;
                end = (char)4;      /* 4 == EOT (end of transmission) */
                int idx{};

                /* output header */

                cout << setw(5) << "idx" << ": " << "title" << endl;

                while (line != end) {       /* while the end character has not been send */

                    /* output the received data in a formatted way */

                    cout << setw(5) << idx << ": " << line << endl;
                    idx++;
                    if (!strm) {
                        spdlog::error("connection to {}:{} closed unexpectedly", endpoints.at(idx_server).first, endpoints.at(idx_server).second);
                        exit(EXIT_UNEXPECTED_CLOSE);
                    }
                    getline(strm, line);
                }

                /* exit the program without setting an error code */

                exit(EXIT_SUCCESS);
            }

            /* create a copy of the idx vector (for shuffle) */

            vector<int> idx_{};

            if (idx_server == 0) {      /* current is the first server */

                /* get number of messages from server */

                strm << "req message cnt" << endl;
                string message_cnt_string;

                if (!strm) {
                    spdlog::error("connection to {}:{} closed unexpectedly", endpoints.at(idx_server).first, endpoints.at(idx_server).second);
                    exit(EXIT_UNEXPECTED_CLOSE);
                }
                getline(strm, message_cnt_string);

                int message_cnt{};

                try {
                    message_cnt = stoi(message_cnt_string);
                } catch (invalid_argument& e) {
                    spdlog::error("could not parse {}, expected number", message_cnt_string);
                    strm.close();
                    spdlog::debug("connection closed");
                    exit(EXIT_COULD_NOT_PARSE_RESPONSE);
                }

                /* get message len from server */

                strm << "req message len" << endl;
                string message_len_str;

                if (!strm) {
                    spdlog::error("connection to {}:{} closed unexpectedly", endpoints.at(idx_server).first, endpoints.at(idx_server).second);
                    exit(EXIT_UNEXPECTED_CLOSE);
                }

                getline(strm, message_len_str);

                try {
                    message_len = stoi(message_len_str);
                } catch (invalid_argument& e) {
                    spdlog::error("could not parse {}, expected number", message_cnt_string);
                    strm.close();
                    spdlog::debug("connection closed");
                    exit(EXIT_COULD_NOT_PARSE_RESPONSE);
                }

                /* setup number distributions */

                uniform_int_distribution<int> dist_idx(0, message_cnt - 1);
                uniform_int_distribution<int> dist_cnt(5, 10);

                int cnt{dist_cnt(rndm_engine)};     /* number of messages to request */

                vector<int> possible_req_idx(message_cnt);
                iota(begin(possible_req_idx), end(possible_req_idx), 0);
                shuffle(begin(possible_req_idx), end(possible_req_idx), rndm_engine);

                for (int j{}; j < cnt; j++) {
                    req_idx.push_back(possible_req_idx.at(j));
                }

            } else {    /* current is the second server */

                /* second server also request the desired message */

                idx_.push_back(req_idx.at(0));
            }

            /* add all messages besides the first one to the requested messages */

            for (unsigned int i{1}; i < req_idx.size(); i++) {
                idx_.push_back(req_idx.at(i));
            }

            /* randomize the oder of the requested messages */

            shuffle(begin(idx_), end(idx_), rndm_engine);

            /* transform the reuqest idx to a string */

            std::stringstream ss;

            for (unsigned int i{0}; i < idx_.size(); ++i) {
                ss << idx_[i];
                if (i != idx_.size() - 1) {
                    ss << ",";
                }
            }

            /* reqeust messages */

            strm << ss.str() << endl;

            /* receive data */

            string data;
            char* buffer = new char[message_len + 1];

            /* write messages into char buffer */

            if (!strm) {
                spdlog::error("connection to {}:{} closed unexpectedly", endpoints.at(idx_server).first, endpoints.at(idx_server).second);
                exit(EXIT_UNEXPECTED_CLOSE);
            }

            while (strm.read(buffer, sizeof(buffer))) {
                data.append(buffer, sizeof(buffer));
            }

            /* move buffer into string */

            data.append(buffer, strm.gcount());

            delete[] buffer;
            spdlog::debug("received data");

            /* copy messages viy memcpy to avoid \0 stopping the copy process */

            char* answer = new char[message_len + 1];
            memcpy(answer, data.c_str(), message_len + 1);
            responses.push_back(answer);

            /* close stream */

            strm.close();
            spdlog::debug("connection closed");
        } else {        /* endpoint is not reachable */
            spdlog::error("Could not connect to server {}:{}!", endpoints.at(idx_server).first, endpoints.at(idx_server).second);
            exit(EXIT_SERVER_UNREACHABLE);
        }
    }

    /* xor responses */

    char* output;
    output = xor_string(responses.at(0), responses.at(1), message_len + 1);

    /* output the desired message */

    cout.write(output, message_len);
    cout << endl;

    /* delete everything */

    delete output;

    for (const char* a : responses) {
        delete a;
    }

    exit(EXIT_SUCCESS);
}
