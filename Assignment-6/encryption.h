#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <stddef.h>

void encrypt_memory(void *data, size_t length, int key)
{
    int i;
    unsigned int *ptr = (unsigned int *)data;
    size_t num_ints = length / sizeof(unsigned int);

    for (i = 0; i < num_ints; ++i)
        ptr[i] ^= key;
}

void decrypt_memory(void *data, size_t length, int key)
{
    encrypt_memory(data, length, key);
}

#endif
