#include <inttypes.h>
#include <string.h>
#include <stdio.h>

struct record_pixel {
    int32_t         x, y;
    uint8_t         r, g, b, a;
};

int main(int argc, char *argv[]) {
    FILE *fin, *fout;
    int version, nsegments, nrecords, i, j;
    char pathname[1024];
    char *ptr;

    if (argc != 2) {
        printf("Usage: record2txt record\n");
        return 1;
    }

    fin = fopen(argv[1], "rb");
    strncpy(pathname, argv[1], 1024);
    ptr = strrchr(pathname, '.');
    strcpy(ptr, ".txt");
    fout = fopen(pathname, "w+");

    fread(&version, sizeof(version), 1, fin);
    fwrite(&version, sizeof(version), 1, fout);
    fread(&nsegments, sizeof(nsegments), 1, fin);
    fprintf(fout, "s %d\n", nsegments);
    for (i = 0; i < nsegments; ++i) {
        struct record_pixel pixel;

        fread(&nrecords, sizeof(nrecords), 1, fin);
        fprintf(fout, "r %d\n", nrecords);

        for (j = 0; j < nrecords; ++j) {
            fread(&pixel, sizeof(pixel), 1, fin);
            fprintf(fout, "c %"PRIi32" %"PRIi32" "
                          "%"PRIu8" %"PRIu8" %"PRIu8" %"PRIu8"\n",
                          pixel.x, pixel.y,
                          pixel.r, pixel.g, pixel.b, pixel.a);
        }
    }

    return 0;
}
