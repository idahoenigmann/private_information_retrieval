#include <iostream>
#include <cstring>
#include <asio.hpp>

using namespace std;

char* xor_string(char a[], char b[], unsigned int len=280) {
    char* output = new char[len];

    for (int i{}; i < len - 1; i++) {
        char tmp = a[i] ^ b[i];
        output[i] = tmp;
    }
    output[len - 1] = '\0';
    return output;
}

int main() {
    char a[280] = "Will be starting The White House news conference at 5:15 P.M. Eastern.";
    char b[280] = "My proposal to the politically correct Automobile Companies would lower the average price of a car to consumers by more than $3500, while at the same time making the cars substantially safer. Engines would run smoother. Positive impact on the environment! Foolish executives!";
    char c[280] = "My Administration is helping U.S. auto workers by replacing the failed Obama Emissions Rule. Impossible to satisfy its Green New Deal Standard; Lots of unnecessary and expensive penalties to car buyers!";

    char* output{xor_string(a, b)};
    output = xor_string(output, c);

    cout << output << endl;

    output = xor_string(output, a);
    output = xor_string(output, c);

    cout << output << endl;
    delete output;

    return 0;
}
