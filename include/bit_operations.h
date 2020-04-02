#ifndef PRIVATE_INFORMATION_RETRIEVAL_BIT_OPERATIONS_H
#define PRIVATE_INFORMATION_RETRIEVAL_BIT_OPERATIONS_H

char* xor_string(const char a[], const char b[], unsigned int len=280) {
    char* output = new char[len];

    for (int i{}; i < len - 1; i++) {
        char tmp = a[i] ^ b[i];
        output[i] = tmp;
    }
    output[len - 1] = '\0';
    return output;
}

#endif //PRIVATE_INFORMATION_RETRIEVAL_BIT_OPERATIONS_H
