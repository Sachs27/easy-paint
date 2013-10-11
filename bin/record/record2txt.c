#include <inttypes.h>
#include <stdio.h>

struct record {
    int32_t         x, y;
    uint8_t         r, g, b, a;
};

int main(int argc, char *argv[]) {
    FILE *fin, *fout;
    int nsegments, nrecords, i, j;

    if (argc != 3) {
        printf("Usage: record2txt in out\n");
        return 1;
    }

    fin = fopen(argv[1], "rb");
    fout = fopen(argv[2], "w+");

    fread(&nsegments, sizeof(nsegments), 1, fin);
    fprintf(fout, "s %d\n", nsegments);
    for (i = 0; i < nsegments; ++i) {
        struct record record;

        fread(&nrecords, sizeof(nrecords), 1, fin);
        fprintf(fout, "r %d\n", nrecords);

        for (j = 0; j < nrecords; ++j) {
            fread(&record, sizeof(record), 1, fin);
            fprintf(fout, "c %"PRIi32" %"PRIi32" "
                          "%"PRIu8" %"PRIu8" %"PRIu8" %"PRIu8"\n",
                          record.x, record.y,
                          record.r, record.g, record.b, record.a);
        }
    }

    return 0;
}
