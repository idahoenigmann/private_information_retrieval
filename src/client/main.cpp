#include <iostream>
#include <asio.hpp>
#include <memory.h>

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
            char buffer[280];

            while (strm.read(buffer, sizeof(buffer))) {
                data.append(buffer, sizeof(buffer));
            }
            data.append(buffer, strm.gcount());
            cout << "received data" << endl;

            char* answer = new char[280];
            memcpy(answer, data.c_str(), 280);
            answers.push_back(answer);
            cout.write(data.c_str(), 280);
            strm.close();
            cout << "connection closed" << endl;
        } else {
            cout << "Could not connect to server!" << endl;
        }
    }

    char* output;
    output = xor_string(answers.at(0), answers.at(1));
    for (int i{2}; i < answers.size(); i++) {
        cout << "Entering " << i << endl;
        output = xor_string(output, answers.at(i));
    }

    cout << "output: " << flush;
    cout.write(output, 280);
    delete output;

    for (const char* a : answers) {
        delete a;
    }

    return 0;
}
