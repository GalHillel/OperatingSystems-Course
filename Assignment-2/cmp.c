#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int cmp(const char *file1, const char *file2, int verbose, int ignore);

int main(int argc, char *argv[])
{
    int verbose = 0, ignore = 0;
    char *file1, *file2;

    if (argc < 3)
    {
        printf("Usage: cmp <file1> <file2> [-v] [-i]\n");
        exit(1);
    }

    file1 = argv[1];
    file2 = argv[2];

    if (argc > 3)
    {
        for (int i = 3; i < argc; i++)
        {
            if (strcmp(argv[i], "-v") == 0)
            {
                verbose = 1;
            }
            else if (strcmp(argv[i], "-i") == 0)
            {
                ignore = 1;
            }
            else
            {
                printf("Unknown option: %s\n", argv[i]);
                exit(1);
            }
        }
    }

    return cmp(file1, file2, verbose, ignore);
}

int cmp(const char *file1, const char *file2, int verbose, int ignore)
{
    FILE *fp1, *fp2;
    char ch1, ch2;
    int line_num = 1, char_num = 0;
    int equal = 1;

    fp1 = fopen(file1, "r");
    fp2 = fopen(file2, "r");

    if (fp1 == NULL || fp2 == NULL)
    {
        printf("Error: Cannot open file\n");
        exit(1);
    }

    while ((ch1 = fgetc(fp1)) != EOF && (ch2 = fgetc(fp2)) != EOF)
    {
        if (ignore)
        {
            ch1 = toupper(ch1);
            ch2 = toupper(ch2);
        }

        if (ch1 != ch2)
        {
            equal = 0;
            if (verbose)
            {
                printf("Files differ at line %d, character %d:\n", line_num, char_num);
                printf("%s: %c\n", file1, ch1);
                printf("%s: %c\n", file2, ch2);
            }
            break;
        }

        if (ch1 == '\n')
        {
            line_num++;
            char_num = 0;
        }
        else
        {
            char_num++;
        }
    }

    if (equal && fgetc(fp1) != fgetc(fp2))
    {
        equal = 0;
    }

    fclose(fp1);
    fclose(fp2);

    if (equal)
    {
        if (verbose)
        {
            printf("equal\n");
        }
        return 0;
    }
    else
    {
        if (verbose)
        {
            printf("distinct\n");
        }
        return 1;
    }
}
