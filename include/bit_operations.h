#ifndef PRIVATE_INFORMATION_RETRIEVAL_BIT_OPERATIONS_H
#define PRIVATE_INFORMATION_RETRIEVAL_BIT_OPERATIONS_H

#include <iostream>


/* Sets all chars after (and including) the \0 to spaces
 * to enable all xor operations to be performed smoothly;
 * (and sets the \0 back into the last char)
 * */
void cleanup_char_arr(char a[], unsigned int len=281) {
    std::fill(a + strlen(a), a + len - 1, ' ');
    a[len - 1] = '\0';
}

/* Perform xor on every char and return a new output char[]
 * */
char* xor_string(const char a[], const char b[], unsigned int len=281) {
    char* output = new char[len];

    for (int i{}; i < len - 1; i++) {
        char tmp = a[i] ^ b[i];
        output[i] = tmp;
    }
    output[len - 1] = '\0';
    return output;
}

#endif //PRIVATE_INFORMATION_RETRIEVAL_BIT_OPERATIONS_H
