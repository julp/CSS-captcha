#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>

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

#ifdef _MSC_VER
extern char __progname[];
#else
extern char *__progname;
# endif /* _MSC_VER */

#ifndef EUSAGE
# define EUSAGE -2
#endif /* EUSAGE */

static UChar ignorable[] = { 0x005B, 0x005C, 0x0074, 0x005C, 0x006E, 0x005C, 0x0066, 0x005C, 0x0072, 0x005C, 0x0070, 0x007B, 0x005A, 0x007D, 0x005D, 0 }; /* [\t\n\f\r\p{Z}] */
static char alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyz";

static char optstr[] = "i:v:";

static struct option long_options[] = {
    { "ignorables", required_argument, NULL, 'i' },
    { "version",    required_argument, NULL, 'v' },
    { NULL,         no_argument,       NULL, 0   }
};

static void usage(void)
{
    fprintf(
        stderr,
        "usage: %s [-%s]\n",
        __progname,
        optstr
    );
    exit(EUSAGE);
}

int version_check(const char *s)
{
    int part;
    char *end;
    unsigned long v;

    part = 0;
    while (1) {
        v = strtoul(s, &end, 10);
        if (v > 9 || end == s || ++part >= U_MAX_VERSION_LENGTH) {
            return 0;
        }
        if ('\0' == *end) {
            break;
        }
        if (U_VERSION_DELIMITER != *end) {
            return 0;
        }
        s = end + 1;
    }

    return part > 0 && part <= U_MAX_VERSION_LENGTH;
}

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

int main(int argc, char **argv)
{
    size_t i;
    UChar32 c;
    USet *uset;
    UChar *ignorablep;
    UErrorCode status;
    UBreakIterator *ubrk;
    int32_t cp_len, res_len;
    char path[1024] = "X.txt";
    const UNormalizer2 *unorm;
    int o, ret, letter, vflag;
    UVersionInfo wanted_version;
    FILE *fps[STR_LEN(alphabet) + 1] = { 0 }; /* + 1 for ignorable.txt */
    UChar cp[U16_MAX_LENGTH + 1], res[STR_LEN(cp) * MAX_UTF16_NFKD_EXPANSION_FACTOR + 1];

    vflag = 0;
    ubrk = NULL;
    uset = NULL;
    unorm = NULL;
    ignorablep = NULL;
    ret = EXIT_FAILURE;
    status = U_ZERO_ERROR;

#ifdef BSD
    {
# include <sys/types.h>
# include <pwd.h>
# include <login_cap.h>

        login_cap_t *lc;
        const char *tmp;
        struct passwd *pwd;

        if (NULL != (pwd = getpwuid(getuid()))) {
            if (NULL != (lc = login_getuserclass(pwd))) {
                if (NULL != (tmp = login_getcapstr(lc, "charset", NULL, NULL))) {
                    ucnv_setDefaultName(tmp);
                }
                login_close(lc);
            } else {
                if (NULL != (lc = login_getpwclass(pwd))) {
                    if (NULL != (tmp = login_getcapstr(lc, "charset", NULL, NULL))) {
                        ucnv_setDefaultName(tmp);
                    }
                    login_close(lc);
                }
            }
        }
        if (NULL != (tmp = getenv("MM_CHARSET"))) {
            ucnv_setDefaultName(tmp);
        }
    }
#endif /* BSD */

    while (-1 != (o = getopt_long(argc, argv, optstr, long_options, NULL))) {
        switch (o) {
            case 'i':
                // not yet implemented
                break;
            case 'v':
            {
                UVersionInfo current_unicode_version;

                vflag = 1;
                u_getUnicodeVersion(current_unicode_version);
                if (!version_check(optarg)) {
                    fprintf(stderr, "Invalid version '%s'\n", optarg);
                    return EXIT_FAILURE;
                }
                u_versionFromString(wanted_version, optarg);
                if (memcmp(wanted_version, current_unicode_version, sizeof(wanted_version)) > 0) {
                    fprintf(stderr, "Requested unicode version (%s) is higher than existent and/or supported (%s)\n", optarg, U_UNICODE_VERSION);
                    return EXIT_FAILURE;
                }
                break;
            }
            default:
                usage();
        }
    }
    argc -= optind;
    argv += optind;

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
        if (vflag) {
            UVersionInfo char_version;

            u_charAge(c, char_version);
            if (memcmp(char_version, wanted_version, sizeof(wanted_version)) > 0) {
                continue;
            }
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
    {
#include <unicode/uclean.h>
        u_cleanup();
    }

    return ret;
}
