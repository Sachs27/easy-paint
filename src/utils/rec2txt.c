#include <stdio.h>
#include <stdlib.h>

#include "../record.h"
#include "../brush.h"

enum record_point_type {
    RECORD_BEGIN_PLOT,
    RECORD_DRAWLINE,
    RECORD_END_PLOT,
    RECORD_SET_BRUSH,
};

#define RECORD_SIG "EASYPAINTRECORD"
#define RECORD_SIG_LEN 16


int main(int argc, char *argv[])
{
    FILE *f;
    char sig[RECORD_SIG_LEN];
    uint32_t version;
    int w, h;
    uint8_t type;
    struct brush brush;
    float pos[4];

    if (argc != 2) {
        printf("Usage: rec2txt <filename>");
        exit(-1);
    }

    f = fopen(argv[1], "r");

    fread(sig, 1, RECORD_SIG_LEN, f);
    fprintf(stdout, "%s\n", sig);

    fread(&version, 1, sizeof(version), f);
    fprintf(stdout, "%u\n", version);

    fread(&w, 1, sizeof(w), f);
    fprintf(stdout, "%d\n", w);

    fread(&h, 1, sizeof(h), f);
    fprintf(stdout, "%d\n", h);

    while (fread(&type, 1, sizeof(type), f)) {
        switch (type) {
        case RECORD_BEGIN_PLOT:
            fprintf(stdout, "B\n");
            break;
        case RECORD_DRAWLINE:
            fread(pos, 1, sizeof(pos), f);
            fprintf(stdout, "L %.1f %.1f %.1f %.1f\n", pos[0], pos[1], pos[2],
                    pos[3]);
            break;
        case RECORD_END_PLOT:
            fprintf(stdout, "E\n");
            break;
        case RECORD_SET_BRUSH:
            fread(&brush, 1, sizeof(brush), f);
            fprintf(stdout, "S %d %.1f %.1f %.1f %.1f %.1f\n",
                    brush.blend_mode, brush.color[0], brush.color[1],
                    brush.color[2], brush.color[3], brush.radius);
            break;
        }
    }

    fclose(f);
}
