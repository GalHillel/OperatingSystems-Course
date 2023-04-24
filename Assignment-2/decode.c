#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <codec> <message>\n", argv[0]);
        exit(1);
    }

    void *handle;
    void (*decode)(char *);
    char *error;

    if (strcmp(argv[1], "codecA") == 0)
    {
        handle = dlopen("./libcodecA.so", RTLD_LAZY);
        if (!handle)
        {
            fprintf(stderr, "%s\n", dlerror());
            exit(1);
        }

        decode = (void (*)(char *))dlsym(handle, "decode");
        if ((error = dlerror()) != NULL)
        {
            fprintf(stderr, "%s\n", error);
            exit(1);
        }
    }
    else if (strcmp(argv[1], "codecB") == 0)
    {
        handle = dlopen("./libcodecB.so", RTLD_LAZY);
        if (!handle)
        {
            fprintf(stderr, "%s\n", dlerror());
            exit(1);
        }

        decode = (void (*)(char *))dlsym(handle, "decode");
        if ((error = dlerror()) != NULL)
        {
            fprintf(stderr, "%s\n", error);
            exit(1);
        }
    }
    else
    {
        fprintf(stderr, "Invalid codec: %s\n", argv[1]);
        exit(1);
    }

    char *message = argv[2];
    decode(message);
    printf("%s\n", message);

    dlclose(handle);
    return 0;
}