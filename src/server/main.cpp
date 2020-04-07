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

    app.add_option("-p,--port", port, "Port on which to send data on")->required(true);
    app.add_flag("-v,--verbose", verbose, "Print additional debug messages");

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
    ifstrm.open("../src/server/data.txt");
    if (ifstrm) {       /* file could be opened */
        string line;

        /* read data.txt line by line
         * data is alternately title and data*/

        while (getline(ifstrm, line)) {
            titles.push_back(line);
            getline(ifstrm, line);
            char *c = new char[message_len + 1];
            strcpy(c, line.c_str());        /* strcpy can be used since the data does not contain \0 */
            cleanup_char_arr(c, message_len + 1);       /* fill up char[] to full message len */
            messages.push_back(c);
        }
    } else {        /* file could not be opened */
        spdlog::error("could not find data.txt");
        exit(2);
    }
    ifstrm.close();

    spdlog::debug("loaded {} messages", messages.size());

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
        getline(strm, data);

        if (data == "req message cnt") {        /* message cnt requested */
            strm << messages.size() << endl;
            getline(strm, data);
        } else if (data == "req list") {        /* message titles requested */
            for (const string& title : titles) {
                strm << title << endl;
            }
            strm << (char)4 << endl;        /* send EOT (end of transmission) */
            exit(0);        /* exit program without setting the error code */
        }

        /* split message idx into integers */

        size_t start = 0;
        size_t end = 0;

        end = data.find(',', start);
        int first = stoi(data.substr(start, end - start));
        start = end + 1;
        end = data.find(',', start);
        int second = stoi(data.substr(start, end - start));

        /* start the xor operation of the messages */

        char* output{xor_string(messages.at(first), messages.at(second))};

        /* retrieve and xor the rest of the messages */

        while ((start = data.find_first_not_of(',', end)) != std::string::npos)
        {
            end = data.find(',', start);
            int message_idx = stoi(data.substr(start, end - start));
            output = xor_string(output, messages.at(message_idx), message_len + 1);
        }

        /* send data */

        spdlog::debug("sending {}", data);
        strm.write(output, message_len + 1);

        /* delete resources */

        delete output;
        strm.close();
        spdlog::debug("connection closed");
    } else {        /* stream could not be opened */
        spdlog::error("could not open stream");
    }
    return 0;
}
