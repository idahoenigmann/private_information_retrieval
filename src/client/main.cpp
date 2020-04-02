#include <iostream>
#include <asio.hpp>

#include "bit_operations.h"

using namespace std;
using namespace asio::ip;

int main() {
    vector<const char*> answers;

    // first element gets retrieved
    vector<int> idx;
    idx.push_back(0);
    idx.push_back(1);
    idx.push_back(2);

    vector<string> ports;
    ports.push_back("1234");
    ports.push_back("1235");

    bool first_server{true};

    for (string port : ports) {
        tcp::iostream strm{"localhost", port};
        if (strm) {
            cout << "connection created" << endl;

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
            cout << port << ": " << ss.str() << endl;
            strm << ss.str() << endl;
            string data;
            //strm.read(data, 280);
            getline(strm, data);
            cout << "received data" << endl;
            answers.push_back(data.c_str());
            strm.close();
            cout << "connection closed" << endl;
        } else {
            cout << "Could not connect to server!" << endl;
        }
    }

    char* output;
    output = xor_string(answers.at(0), answers.at(1));
    for (int i{2}; i < answers.size(); i++) {
        output = xor_string(output, answers.at(i));
    }

    cout << "output: " << output << endl;
    delete output;

    return 0;
}
