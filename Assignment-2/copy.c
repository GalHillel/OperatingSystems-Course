#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int success = 0;
    char *source_file = argv[1];
    char *target_file = argv[2];
    char *flag = argv[3];

    if (argc < 3)
    {
        printf("Usage: copy <source_file> <target_file> [-v] [-f]\n");
        return 1;
    }

    FILE *src = fopen(source_file, "r");
    if (!src)
    {
        printf("Failed to open source file\n");
        return 1;
    }

    FILE *tgt = fopen(target_file, "r");
    if (tgt)
    {
        fclose(tgt);
        if (strcmp(flag, "-f") != 0)
        {
            printf("Target file exists\n");
            return 1;
        }
    }

    tgt = fopen(target_file, "w");
    if (!tgt)
    {
        printf("Failed to create target file\n");
        return 1;
    }

    int c;
    while ((c = fgetc(src)) != EOF)
    {
        if (fputc(c, tgt) == EOF)
        {
            printf("Failed to write to target file\n");
            success = 1;
            break;
        }
    }

    fclose(src);
    fclose(tgt);

    if (success == 0)
    {
        printf("Success\n");
        return 0;
    }
    else
    {
        printf("General failure\n");
        return 1;
    }
}
