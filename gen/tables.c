#include <stdlib.h>
#include <stdio.h>

#include <unicode/uset.h>
#include <unicode/ubrk.h>
#include <unicode/unorm2.h>

#define U_0 0x0030 /* 0 */
#define U_9 0x0039 /* 9 */
#define U_A 0x0041 /* A */
#define U_Z 0x005A /* Z */
#define U_a 0x0061 /* a */
#define U_z 0x007A /* z */

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#define STR_LEN(str)      (ARRAY_SIZE(str) - 1)
#define STR_SIZE(str)     (ARRAY_SIZE(str))

#define MAX_UTF16_NFD_EXPANSION_FACTOR  4
#define MAX_UTF16_NFKD_EXPANSION_FACTOR 18

static UChar ignorable[] = { 0x005B, 0x005C, 0x0074, 0x005C, 0x006E, 0x005C, 0x0066, 0x005C, 0x0072, 0x005C, 0x0070, 0x007B, 0x005A, 0x007D, 0x005D, 0 }; /* [\t\n\f\r\p{Z}] */
static char alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyz";

int accept_as(UChar c)
{
    if (c >= U_0 && c <= U_9) { /* '0' .. '9' */
        return c - U_0;
    }
    if (c >= U_A && c <= U_Z) { /* 'A' .. 'Z' */
        return 10 + (c - U_A);
    }
    if (c >= U_a && c <= U_z) { /* 'a' .. 'z' */
        return 10 + (c - U_a);
    }

    return -1;
}

int main(void)
{
    size_t i;
    UChar32 c;
    USet *uset;
    int ret, letter;
    UErrorCode status;
    UBreakIterator *ubrk;
    int32_t cp_len, res_len;
    char path[1024] = "X.txt";
    const UNormalizer2 *unorm;
    FILE *fps[STR_LEN(alphabet) + 1] = { 0 }; /* + 1 pour ignorable.txt */
    UChar cp[U16_MAX_LENGTH + 1], res[STR_LEN(cp) * MAX_UTF16_NFKD_EXPANSION_FACTOR + 1];

    ubrk = NULL;
    uset = NULL;
    unorm = NULL;
    ret = EXIT_FAILURE;
    status = U_ZERO_ERROR;
//     unorm = unorm2_getNFDInstance(&status);
    unorm = unorm2_getNFKDInstance(&status);
    if (U_FAILURE(status)) {
        fprintf(stderr, "unorm2_getNFDInstance failed with %s\n", u_errorName(status));
        goto end;
    }
    ubrk = ubrk_open(UBRK_CHARACTER, NULL, NULL, 0, &status);
    if (U_FAILURE(status)) {
        fprintf(stderr, "ubrk_open failed with %s\n", u_errorName(status));
        goto end;
    }
    for (i = 0; i < STR_LEN(alphabet); i++) {
        path[0] = alphabet[i];
        if (NULL == (fps[i] = fopen(path, "w"))) {
            fprintf(stderr, "fopen failed\n");
            goto end;
        }
    }
    if (NULL == (fps[i] = fopen("ignorable.txt", "w"))) {
        fprintf(stderr, "fopen failed\n");
        goto end;
    }
    for (c = 0x80; c <= UCHAR_MAX_VALUE/*0xFFFF*/; c++) {
        cp_len = 0;
        U16_APPEND_UNSAFE(cp, cp_len, c);
        cp[cp_len] = 0;
        res_len = unorm2_normalize(unorm, cp, cp_len, res, STR_SIZE(res), &status);
        if (U_FAILURE(status)) {
            fprintf(stderr, "unorm2_normalize failed with %s\n", u_errorName(status));
            goto end;
        }
        if ((letter = accept_as(res[0])) >= 0) {
            ubrk_setText(ubrk, res, res_len, &status);
            if (U_FAILURE(status)) {
                fprintf(stderr, "ubrk_setText failed with %s\n", u_errorName(status));
                goto end;
            }
            ubrk_first(ubrk);
            ubrk_next(ubrk);
            if (UBRK_DONE != ubrk_next(ubrk)) {
                continue;
            }
            if (2 == cp_len) {
                fprintf(fps[letter], "%06X\n", c);
            } else {
                fprintf(fps[letter], "%04X\n", c);
            }
        }
    }
    uset = uset_openPattern(ignorable, -1, &status);
    if (U_FAILURE(status)) {
        fprintf(stderr, "uset_openPattern failed with %s\n", u_errorName(status));
        goto end;
    }
    {
        int32_t i, s;

        for (i = 0, s = uset_size(uset); i < s; i++) {
            c = uset_charAt(uset, i);
            fprintf(fps[ARRAY_SIZE(fps) - 1], "%06X\n", c);
        }
    }
    ret = EXIT_SUCCESS;

end:
    if (NULL != ubrk) {
        ubrk_close(ubrk);
    }
    if (NULL != uset) {
        uset_close(uset);
    }
    for (i = 0; i < ARRAY_SIZE(fps); i++) {
        if (NULL != fps[i]) {
            fclose(fps[i]);
        }
    }
    return ret;
}
