#include <asio.hpp>
#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>

#include "bit_operations.h"

using namespace std;
using namespace asio::ip;

unsigned int message_len{280};

int main(int argc, char* argv[]) {
    CLI::App app{"Private Information Retrieval Server"};

    /* Specify command line options */

    unsigned short port;
    bool verbose{false};
    bool loop{false};
    string data_file{"data.txt"};

    app.add_option("-p,--port", port, "Port on which to send data on")->required(true);
    app.add_flag("-v,--verbose", verbose, "Print additional debug messages");
    app.add_flag("-l,--loop", loop, "Loop back to start after having send data");
    app.add_option("-f,--file", data_file, "File in which the messages are stored");
    app.add_option("-s,--size", message_len, "Maximum length of messages");

    CLI11_PARSE(app, argc, argv);

    /* set verbose level */

    if (verbose) {
        spdlog::set_level(spdlog::level::debug);
    } else {
        spdlog::set_level(spdlog::level::info);
    }

    /* setup vectors for later use */

    vector<char*> messages;
    vector<string> titles;

    /* read data file */

    ifstream ifstrm;
    ifstrm.open(data_file);
    if (ifstrm) {       /* file could be opened */
        string line;

        /* read data_file line by line
         * data is alternately title and data*/

        while (getline(ifstrm, line)) {
            titles.push_back(line);
            if (!getline(ifstrm, line)) {
                spdlog::error("file {} does not have the reqired format (the number of lines must be even", data_file);
                exit(2);
            }
            char *c = new char[message_len + 1];
            strcpy(c, line.c_str());        /* strcpy can be used since the data does not contain \0 */
            cleanup_char_arr(c, message_len + 1);       /* fill up char[] to full message len */
            messages.push_back(c);
        }
    } else {        /* file could not be opened */
        spdlog::error("could not find {}", data_file);
        exit(2);
    }
    ifstrm.close();

    spdlog::debug("loaded {} messages", messages.size());

    do {        /* loop through code at least once or endlessly if loop flat was set */

        /* setup connection */

        asio::io_context ctx;
        tcp::endpoint ep{tcp::v4(), port};
        spdlog::debug("connection opened");
        tcp::acceptor acceptor{ctx, ep};
        spdlog::debug("listening");
        acceptor.listen();

        tcp::socket sock{ctx};
        acceptor.accept(sock);
        tcp::iostream strm{std::move(sock)};

        if (strm) {     /* stream could be opened */

            /* receive data on what to send */

            string data;

            if (!strm) {
                spdlog::error("connection to closed unexpectedly");
                exit(5);
            }

            getline(strm, data);

            if (data == "req list") {        /* message titles requested */
                for (const string& title : titles) {
                    strm << title << endl;
                }
                strm << (char)4 << endl;        /* send EOT (end of transmission) */

                strm.close();
                spdlog::debug("connection closed");
                continue;
            }

            if (data == "req message cnt") {        /* message cnt requested */
                strm << messages.size() << endl;

                if (!strm) {
                    spdlog::error("connection to closed unexpectedly");
                    exit(5);
                }

                getline(strm, data);
            }

            if (data == "req message len") {        /* message len requested */
                strm << message_len << endl;

                if (!strm) {
                    spdlog::error("connection to closed unexpectedly");
                    exit(5);
                }

                getline(strm, data);
            }

            /* split message idx into integers */

            char* output{};

            try {

                size_t start = 0;
                size_t end = 0;

                end = data.find(',', start);
                int first = stoi(data.substr(start, end - start));
                start = end + 1;
                end = data.find(',', start);
                int second = stoi(data.substr(start, end - start));

                /* start the xor operation of the messages */

                output = xor_string(messages.at(first), messages.at(second));

                /* retrieve and xor the rest of the messages */

                while ((start = data.find_first_not_of(',', end)) != std::string::npos) {
                    end = data.find(',', start);
                    int message_idx = stoi(data.substr(start, end - start));
                    output = xor_string(output, messages.at(message_idx), message_len + 1);
                }

            } catch (invalid_argument& e) {
                spdlog::error("response {} could not be parsed!", data);
                strm.close();
                spdlog::debug("connection closed");
                continue;
            }

            /* send data */

            if (!strm) {
                spdlog::error("connection to closed unexpectedly");
                exit(5);
            }

            spdlog::debug("sending {}", data);
            strm.write(output, message_len + 1);

            /* delete resources */

            delete output;
            strm.close();
            spdlog::debug("connection closed");

        } else {        /* stream could not be opened */
            spdlog::error("could not open stream");
        }

    } while (loop);     /* loop flag was set */

    return 0;
}
