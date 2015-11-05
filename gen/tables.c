#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <limits.h>

#include <unicode/ucnv.h>
#include <unicode/uset.h>
#include <unicode/ubrk.h>
#include <unicode/unorm2.h>

#include "darray.h"

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
#endif /* _MSC_VER */

#ifndef EUSAGE
# define EUSAGE -2
#endif /* !EUSAGE */

#ifndef UINT8_MAX
# if defined(UCHAR_MAX)
#  define UINT8_MAX UCHAR_MAX
# elif defined(CHAR_MAX)
#  define UINT8_MAX CHAR_MAX
# endif
#endif /* !UINT8_MAX */

typedef struct {
    int cu_len;
    UChar32 value;
    UVersionInfo version;
} codepoint_t;

static char alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyz";
static UChar default_ignorables[] = { 0x005B, 0x005C, 0x0074, 0x005C, 0x006E, 0x005C, 0x0066, 0x005C, 0x0072, 0x005C, 0x0070, 0x007B, 0x005A, 0x007D, 0x005D, 0 }; /* [\p{Z}] */

static char optstr[] = "f:i:v:";

static struct option long_options[] = {
    { "format",     required_argument, NULL, 'f' },
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

enum {
#define UNICODE_FIRST(M, m, p) \
    UNICODE_FIRST = UNICODE_##M##_##m##_##p,
#define UNICODE_LAST(M, m, p) \
    UNICODE_LAST = UNICODE_##M##_##m##_##p,
#define UNICODE_VERSION(M, m, p) \
    UNICODE_##M##_##m##_##p,
#include "unicode_versions.h"
    _UNICODE_VERSIONS_COUNT
#undef UNICODE_FIRST
#undef UNICODE_LAST
#undef UNICODE_VERSION
};

static UVersionInfo unicode_versions[_UNICODE_VERSIONS_COUNT] = {
#define UNICODE_FIRST(M, m, p) /* NOP */
#define UNICODE_LAST(M, m, p) /* NOP */
#define UNICODE_VERSION(M, m, p) \
    { M, m, p, 0 },
#include "unicode_versions.h"
#undef UNICODE_FIRST
#undef UNICODE_LAST
#undef UNICODE_VERSION
};

#define IDENT_STRING "    "

#define NEW_LINE(fp) \
    fputc('\n', fp)

static void generic_generate_table(FILE *fp, DArray **da, const char *start_decl, const char *end_decl)
{
    int i, col_len;

    fputs(start_decl, fp);
    NEW_LINE(fp);
    fputs(IDENT_STRING, fp);
    col_len = STR_LEN(IDENT_STRING);
    for (i = 0; i < STR_LEN(alphabet) + 1 /* for ignorables */; i++) {
        size_t l, j;

        l = darray_length(da[i]);
        for (j = 0; j < l; j++) {
            codepoint_t cp;

            cp = darray_at_unsafe(da[i], j, codepoint_t);
            col_len += fprintf(fp, 2 == cp.cu_len ? "0x%06X," : "0x%04X,", cp.value);
            if (col_len > 160) {
                col_len = STR_LEN(IDENT_STRING);
                NEW_LINE(fp);
                fputs(IDENT_STRING, fp);
            } else {
                fputc(' ', fp);
            }
        }
    }
    NEW_LINE(fp);
    fputs(end_decl, fp);
    NEW_LINE(fp);
}

static void generic_generate_offsets(FILE *fp, size_t offsets[][_UNICODE_VERSIONS_COUNT + 1 /* for start offsets */], const char *one_line_comment, const char *fmt_start_decl, const char *end_decl, const char *array_beg, char array_end)
{
    int i;

    fprintf(fp, fmt_start_decl, _UNICODE_VERSIONS_COUNT + 1 /* for start offsets */);
    fputs(" = ", fp);
    fputs(array_beg, fp);
    NEW_LINE(fp);
    fputs(IDENT_STRING, fp);
    fputs(one_line_comment, fp);
    fprintf(fp, "%*c BASE", strlen(array_beg) - strlen(one_line_comment) + 2, ' ');
    for (i = 0; i < _UNICODE_VERSIONS_COUNT; i++) {
        fprintf(fp, " | %u.%u.%u", unicode_versions[i][0], unicode_versions[i][1], unicode_versions[i][2]);
    }
    NEW_LINE(fp);
    for (i = 0; i < STR_LEN(alphabet) + 1 /* for ignorables */; i++) {
        int j;

        if (0 != i) {
            fprintf(fp, " %c,", array_end);
            NEW_LINE(fp);
        }
        fputs(IDENT_STRING, fp);
        fputs(one_line_comment, fp);
        fputc(' ', fp);
        if ('\0' == alphabet[i]) {
            fputs("ignorables", fp);
        } else {
            fputc(alphabet[i], fp);
        }
        NEW_LINE(fp);
        fputs(IDENT_STRING, fp);
        fputs(array_beg, fp);
        fputc(' ', fp);
        for (j = 0; j < _UNICODE_VERSIONS_COUNT + 1 /* for start offsets */; j++) {
            if (0 != j) {
                fputs(", ", fp);
            }
            fprintf(fp, "%6zu", offsets[i][j]);
        }
    }
    fprintf(fp, " %c", array_end);
    NEW_LINE(fp);
    fputs(end_decl, fp);
    NEW_LINE(fp);
}

void generate_tables_for_ruby(FILE *fp, DArray **da, size_t offsets[][_UNICODE_VERSIONS_COUNT + 1 /* for start offsets */])
{
    generic_generate_table(fp, da, "TABLE = [", "]");
    generic_generate_offsets(fp, offsets, "#", "OFFSETS", "]", "[", ']');
}

void generate_tables_for_php(FILE *fp, DArray **da, size_t offsets[][_UNICODE_VERSIONS_COUNT + 1 /* for start offsets */])
{
    generic_generate_table(fp, da, "protected static $_table = array(", ");");
    generic_generate_offsets(fp, offsets, "//", "protected static $_offsets", ");", "array(", ')');
}

void generate_tables_for_c(FILE *fp, DArray **da, size_t offsets[][_UNICODE_VERSIONS_COUNT + 1 /* for start offsets */])
{
    int i;

    fputs("/* do not edit this file, it was generated by gen/tables */\n\n", fp);
    generic_generate_table(fp, da, "static uint32_t table[] = {", "};");
    generic_generate_offsets(fp, offsets, "//", "static size_t offsets[][%d]", "};", "{", '}');
}

static struct {
    const char *name;
    void (*callback)(FILE *, DArray **, size_t [][_UNICODE_VERSIONS_COUNT + 1 /* for start offsets */]);
} formats[] = {
    { "c", generate_tables_for_c },
    { "php", generate_tables_for_php },
    { "ruby", generate_tables_for_ruby },
};

int cmp_version_info(const void *a, const void *b)
{
    const UVersionInfo *v1, *v2;

    v1 = (const UVersionInfo *) a; /* key */
    v2 = (const UVersionInfo *) b;

    return memcmp(v1, v2, sizeof(*v1));
}

int sort_codepoints(const void *a, const void *b, void *arg)
{
    int diff;
    const codepoint_t *c1, *c2;

    c1 = (const codepoint_t *) a;
    c2 = (const codepoint_t *) b;

    if (0 == (diff = (memcmp(c1->version, c2->version, sizeof(*c1->version))))) {
        diff = c1->value - c2->value;
    }

    return diff;
}

int version_check(const char *s)
{
    int part;
    char *end;
    unsigned long v;

    part = 0;
    while (1) {
        v = strtoul(s, &end, 10);
        if (v > UINT8_MAX || end == s || ++part >= U_MAX_VERSION_LENGTH) {
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
    USet *uset;
    codepoint_t cp;
    UErrorCode status;
    UBreakIterator *ubrk;
    UChar *user_ignorables;
    const UNormalizer2 *unorm;
    UVersionInfo wanted_version;
    int o, ret, letter, vflag, output_format;
    DArray *da[STR_LEN(alphabet) + 1] = { 0 }; /* + 1 for ignorables.txt */
    size_t offsets[STR_LEN(alphabet) + 1 /* for ignorables */][_UNICODE_VERSIONS_COUNT + 1 /* for start offsets */] = { 0 };

    vflag = 0;
    ubrk = NULL;
    uset = NULL;
    unorm = NULL;
    ret = EXIT_FAILURE;
    status = U_ZERO_ERROR;
    output_format = 0; // default for "c"
    user_ignorables = default_ignorables;

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
            case 'f':
            {
                output_format = -1;

                for (i = 0; -1 == output_format && i < ARRAY_SIZE(formats); i++) {
                    if (0 == strcmp(formats[i].name, optarg)) {
                        output_format = i;
                    }
                }
                if (-1 == output_format) {
                    fprintf(stderr, "unknown format %s\n", optarg);
                    return EXIT_FAILURE;
                }
                break;
            }
            case 'i':
            {
                UConverter *ucnv;
                int32_t user_ignorables_size, user_ignorables_length;

                ucnv = ucnv_open(NULL, &status);
                if (U_FAILURE(status)) {
                    fprintf(stderr, "ucnv_open failed with %s\n", u_errorName(status));
                    return EXIT_FAILURE;
                }
                user_ignorables_size = ucnv_toUChars(ucnv, NULL, 0, optarg, -1, &status) + 1;
                if (status != U_BUFFER_OVERFLOW_ERROR) {
                    fprintf(stderr, "unexpected return value from ICU (%s)\n", u_errorName(status));
                    ucnv_close(ucnv);
                    return EXIT_FAILURE;
                }
                status = U_ZERO_ERROR;
                if (NULL == (user_ignorables = malloc(*user_ignorables * user_ignorables_size))) {
                    fprintf(stderr, "memory allocation failed\n");
                    ucnv_close(ucnv);
                    return EXIT_FAILURE;
                }
                user_ignorables_length = ucnv_toUChars(ucnv, user_ignorables, user_ignorables_size, optarg, -1, &status);
                if (U_FAILURE(status)) {
                    fprintf(stderr, "conversion into UTF-16 failed (ucnv_toUChars) with %s\n", u_errorName(status));
                    ucnv_close(ucnv);
                    return EXIT_FAILURE;
                }
                ucnv_close(ucnv);
                break;
            }
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

    uset = uset_openPattern(user_ignorables, -1, &status);
    if (U_FAILURE(status)) {
        fprintf(stderr, "uset_openPattern failed with %s\n", u_errorName(status));
        goto end;
    }
#if U_ICU_VERSION_MAJOR_NUM >= 49
//     unorm = unorm2_getNFDInstance(&status);
    unorm = unorm2_getNFKDInstance(&status);
#else
    unorm = unorm2_getInstance(NULL, "nfkc", UNORM2_DECOMPOSE, &status);
#endif /* ICU >= 49 */
    if (U_FAILURE(status)) {
        fprintf(stderr, "unorm2_getNFKDInstance failed with %s\n", u_errorName(status));
        goto end;
    }
    ubrk = ubrk_open(UBRK_CHARACTER, NULL, NULL, 0, &status);
    if (U_FAILURE(status)) {
        fprintf(stderr, "ubrk_open failed with %s\n", u_errorName(status));
        goto end;
    }
    for (i = 0; i < STR_LEN(alphabet) + 1 /* for ignorables */; i++) {
        da[i] = darray_new(NULL, sizeof(codepoint_t));
    }
    for (cp.value = 0x80; cp.value <= UCHAR_MAX_VALUE; cp.value++) {
        int32_t res_len;
        UChar cu[U16_MAX_LENGTH + 1], res[STR_LEN(cu) * MAX_UTF16_NFKD_EXPANSION_FACTOR + 1];

        cp.cu_len = 0;
        U16_APPEND_UNSAFE(cu, cp.cu_len, cp.value);
        cu[cp.cu_len] = 0;
        res_len = unorm2_normalize(unorm, cu, cp.cu_len, res, STR_SIZE(res), &status);
        if (U_FAILURE(status)) {
            fprintf(stderr, "unorm2_normalize failed with %s\n", u_errorName(status));
            goto end;
        }
        u_charAge(cp.value, cp.version);
        if (vflag) {
            if (memcmp(cp.version, wanted_version, sizeof(wanted_version)) > 0) {
                continue;
            }
        }
        if ((letter = accept_as(res[0])) >= 0) {
            UVersionInfo *match;

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
            if (NULL == (match = bsearch(&cp.version, unicode_versions, ARRAY_SIZE(unicode_versions), sizeof(unicode_versions[0]), cmp_version_info))) {
                fprintf(stderr, "unable to found Unicode version %u.%u.%u.%u\n", cp.version[0], cp.version[1], cp.version[2], cp.version[3]);
                goto end;
            } else {
                offsets[letter][match - unicode_versions + 1 /* for start offsets */]++;
            }
            darray_push(da[letter], cp);
        }
    }
    {
        int32_t i, s;
        UVersionInfo *match;

        for (i = 0, s = uset_size(uset); i < s; i++) {
            cp.value = uset_charAt(uset, i);
            if (cp.value < 0x20) {
                continue;
            }
            u_charAge(cp.value, cp.version);
            if (vflag) {
                if (memcmp(cp.version, wanted_version, sizeof(wanted_version)) > 0) {
                    continue;
                }
            }
            cp.cu_len = U16_LENGTH(cp.value);
            if (NULL == (match = bsearch(&cp.version, unicode_versions, ARRAY_SIZE(unicode_versions), sizeof(unicode_versions[0]), cmp_version_info))) {
                fprintf(stderr, "unable to found Unicode version %u.%u.%u.%u\n", cp.version[0], cp.version[1], cp.version[2], cp.version[3]);
                goto end;
            } else {
                offsets[ARRAY_SIZE(da) - 1][match - unicode_versions + 1 /* for start offsets */]++;
            }
            darray_push(da[ARRAY_SIZE(da) - 1], cp);
        }
    }
    for (i = 0; i < STR_LEN(alphabet) + 1 /* for ignorables */; i++) {
        darray_sort(da[i], sort_codepoints, NULL);
    }
    for (i = 0; i < STR_LEN(alphabet) + 1 /* for ignorables */; i++) {
        int j;

        if (0 != i) {
            offsets[i][0] = offsets[i - 1][_UNICODE_VERSIONS_COUNT - 1 + 1 /* for start offsets */];
        }
        for (j = 1; j < _UNICODE_VERSIONS_COUNT + 1 /* for start offsets */; j++) {
            offsets[i][j] += offsets[i][j - 1];
        }
    }
    formats[output_format].callback(stdout, da, offsets);
    ret = EXIT_SUCCESS;

end:
    if (user_ignorables != default_ignorables) {
        free(user_ignorables);
    }
    if (NULL != ubrk) {
        ubrk_close(ubrk);
    }
    if (NULL != uset) {
        uset_close(uset);
    }
    for (i = 0; i < STR_LEN(alphabet) + 1 /* for ignorables */; i++) {
        darray_destroy(da[i]);
    }
    {
#include <unicode/uclean.h>
        u_cleanup();
    }

    return ret;
}
