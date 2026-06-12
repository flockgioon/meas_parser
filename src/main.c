#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage %s <file.bin>\n", argv[0]);
        return 1;
    }

    return 0;
}
