#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <unicode/uchar.h>
#include "utils.h"

static const UVersionInfo versions[] = {
#define UNICODE_VERSION(M, m, p) \
    { M, m, p, 0 },
#include "known_unicode_versions.h"
#undef UNICODE_VERSION
};

int main(int argc, char **argv)
{
    size_t i;
    FILE *fp;
    UVersionInfo implemented;

    if (2 != argc) {
        fprintf(stderr, "argument expected");
        return EXIT_FAILURE;
    }
    if (NULL == (fp = fopen(argv[1], "w"))) {
        fprintf(stderr, "failed to open %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    u_getUnicodeVersion(implemented);
    for (i = 0; i < ARRAY_SIZE(versions); i++) {
        if (memcmp(versions[i], implemented, sizeof(versions[i])) > 0) {
            break;
        } else {
            fprintf(fp, "UNICODE_VERSION(%" PRIu8 ", %" PRIu8 ", %" PRIu8 ")\n", versions[i][0], versions[i][1], versions[i][2]);
        }
    }
    fprintf(fp, "\nUNICODE_FIRST(%" PRIu8 ", %" PRIu8 ", %" PRIu8 ")\n", versions[0][0], versions[0][1], versions[0][2]);
    fprintf(fp, "UNICODE_LAST(%" PRIu8 ", %" PRIu8 ", %" PRIu8 ")\n", versions[i - 1][0], versions[i - 1][1], versions[i - 1][2]);
    fclose(fp);

    return EXIT_SUCCESS;
}
