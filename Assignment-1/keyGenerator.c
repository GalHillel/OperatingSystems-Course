#include "ecnript.h"
#include <stdio.h>

int main() {

    int c = 0;

    while (c < 49 || c > 53) {
        printf("\n 1 - Michal Gor The Musician: ");
        printf("\n 2 - Hilel Zak The Tolmid Hoham: ");
        printf("\n 3 - Merav Verd The Engeneer: ");
        printf("\n 4 - Tehila Kai The Creator: ");
        printf("\n 5 - Gal Hillel The Computer Scientist: ");

        printf("\n Select an agent :");
        c = getchar();
    }

    char *name;
    long id;

    if (c == 49) {
        name = "Michal Gor";
        id = 22446688;
    }

    if (c == 51) {
        name = "Merav Verd";
        id = 11223344;
    }

    if (c == 50) {
        name = "Hilel Zak";
        id = 11335577;
    }

    if (c == 52) {
        name = "Tehila Kai";
        id = 321654987;
    }

    if (c == 53) {
        name = "Gal Hillel";
        id = 211696521;
    }

    int type = getType(name);

    if (type != 1) {
        *((char *) -1) = 'x';
    }
    printf("starting encryption\n");
    char *encrypted = getEncriptedMessage(name, type, id);
    printf("\nencryption done:\n");
    printf("%s\n", encrypted);
    printf("****------------------------------------------****\n");
    printf("| Put the above in read.me, ZIP with your ID, and upload to moodle |\n");
    printf("****------------------------------------------****\n");
    return 0;
}
