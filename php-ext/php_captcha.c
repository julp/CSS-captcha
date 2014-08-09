#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

/* Uncommnent the following line to enable confusable characters ('0', '1', 'i', 'l' and 'o') */
/*#define CAPTCHA_WITH_CONFUSABLE 1*/

/* Uncomment the following lines to target a maximum version of Unicode */
/*#define CAPTCHA_UNICODE_MAJOR 3
#define CAPTCHA_UNICODE_MINOR 0
#define CAPTCHA_UNICODE_PATCH 0*/

#ifndef CAPTCHA_UNICODE_MAJOR
# define CAPTCHA_UNICODE_MAJOR 99
#endif /* !CAPTCHA_UNICODE_MAJOR */
#ifndef CAPTCHA_UNICODE_MINOR
# define CAPTCHA_UNICODE_MINOR 0
#endif /* !CAPTCHA_UNICODE_MINOR */
#ifndef CAPTCHA_UNICODE_PATCH
# define CAPTCHA_UNICODE_PATCH 0
#endif /* !CAPTCHA_UNICODE_PATCH */

#define CAPTCHA_UNICODE_VERSION (CAPTCHA_UNICODE_MAJOR * 1000 + CAPTCHA_UNICODE_MINOR * 100 + CAPTCHA_UNICODE_PATCH)

#include <stdint.h>
#include "php.h"
#include "php_ini.h"
#include "php_captcha.h"
#include "ext/standard/php_rand.h"
#include "ext/session/php_session.h"
#include "ext/standard/php_smart_str.h"

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#define STR_LEN(str)      (ARRAY_SIZE(str) - 1)
#define STR_SIZE(str)     (ARRAY_SIZE(str))

#define CAPTCHA_CLASS_NAME "CSSCaptcha"
#define CAPTCHA_INI_PREFIX "captcha"

#define CAPTCHA_RENDER_CSS  (1 << 0)
#define CAPTCHA_RENDER_HTML (1 << 1)

#define CAPTCHA_PREFIX(x) \
    CAPTCHA_##x

#define XCAPTCHA_ATTR_PREFIX \
    ATTR_

#define CAPTCHA_ATTR_PREFIX(x) \
    CAPTCHA_PREFIX(XCAPTCHA_ATTR_PREFIX ## x)
//     CAPTCHA_ ## XCAPTCHA_ATTR_PREFIX ## x

#define XCAPTCHA_COLOR_PREFIX \
    COLOR_

#define CAPTCHA_COLOR_PREFIX(x) \
    CAPTCHA_PREFIX(XCAPTCHA_COLOR_PREFIX ## x)

#define CAPTCHA_ATTR(member) \
    co->member

enum {
#define BOOL_CAPTCHA_ATTRIBUTE(member, name, defaultvalue) \
    CAPTCHA_ATTR_PREFIX(name),
#define LONG_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb) \
    CAPTCHA_ATTR_PREFIX(name),
#define STRING_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb) \
    CAPTCHA_ATTR_PREFIX(name),
#include "captcha_attributes.h"
#undef BOOL_CAPTCHA_ATTRIBUTE
#undef LONG_CAPTCHA_ATTRIBUTE
#undef STRING_CAPTCHA_ATTRIBUTE
    CAPTCHA_ATTR_PREFIX(COUNT)
};

static int check_color_attribute(zval * TSRMLS_DC);
static int check_challenge_length_attribute(zval * TSRMLS_DC);
static int check_zero_or_positive_attribute(zval * TSRMLS_DC);
static int check_fake_characters_length_attribute(zval * TSRMLS_DC);

struct captcha_attribute_t {
    size_t offset;
    int (*cb)(zval * TSRMLS_DC);
} static attributes[] = {
#define BOOL_CAPTCHA_ATTRIBUTE(member, name, defaultvalue) \
    { offsetof(Captcha_object, member), cb },
#define LONG_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb) \
    { offsetof(Captcha_object, member), cb },
#define STRING_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb) \
    { offsetof(Captcha_object, member), cb },
#include "captcha_attributes.h"
#undef BOOL_CAPTCHA_ATTRIBUTE
#undef LONG_CAPTCHA_ATTRIBUTE
#undef STRING_CAPTCHA_ATTRIBUTE
};

enum {
#define CAPTCHA_COLOR(hminx, hmax, smin, smax, lmin, lmax, name) \
    CAPTCHA_COLOR_PREFIX(name),
#include "captcha_colors.h"
#undef CAPTCHA_COLOR
    CAPTCHA_COLOR_PREFIX(COUNT)
};

struct captcha_colordef_t {
    // [0;360[
    uint16_t hmin;
    uint16_t hmax;
    // [0;100]
    uint8_t smin;
    uint8_t smax;
    // [0;100]
    uint8_t lmin;
    uint8_t lmax;
} static colordefs[] = {
#define CAPTCHA_COLOR(hminx, hmax, smin, smax, lmin, lmax, name) \
    { hminx, hmax, smin, smax, lmin, lmax },
#include "captcha_colors.h"
#undef CAPTCHA_COLOR
};

zend_class_entry *Captcha_ce_ptr = NULL;
zend_object_handlers Captcha_handlers;

#ifdef CAPTCHA_WITH_CONFUSABLE
static const char alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyz";
#else
static const char alphabet[] = "23456789abcdefghjkmnpqrstuvwxyz";
#endif /* CAPTCHA_WITH_CONFUSABLE */

static const char *table_0[] = {
    "2070", "2080", "24EA", "FF10",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D7CE", "01D7D8", "01D7E2", "01D7EC", "01D7F6",
#endif /* UNICODE >= 3.1.0 */
    NULL
};

static const char *table_1[] = {
    "00B9", "2081", "2460", "FF11",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D7CF", "01D7D9", "01D7E3", "01D7ED", "01D7F7",
#endif /* UNICODE >= 3.1.0 */
    NULL
};

static const char *table_2[] = {
    "00B2", "2082", "2461", "FF12",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D7D0", "01D7DA", "01D7E4", "01D7EE", "01D7F8",
#endif /* UNICODE >= 3.1.0 */
    NULL
};

static const char *table_3[] = {
    "00B3", "2083", "2462", "FF13",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D7D1", "01D7DB", "01D7E5", "01D7EF", "01D7F9",
#endif /* UNICODE >= 3.1.0 */
    NULL
};

static const char *table_4[] = {
    "2074", "2084", "2463", "FF14",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D7D2", "01D7DC", "01D7E6", "01D7F0", "01D7FA",
#endif /* UNICODE >= 3.1.0 */
    NULL
};

static const char *table_5[] = {
    "2075", "2085", "2464", "FF15",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D7D3", "01D7DD", "01D7E7", "01D7F1", "01D7FB",
#endif /* UNICODE >= 3.1.0 */
    NULL
};

static const char *table_6[] = {
    "2076", "2086", "2465", "FF16",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D7D4", "01D7DE", "01D7E8", "01D7F2", "01D7FC",
#endif /* UNICODE >= 3.1.0 */
    NULL
};

static const char *table_7[] = {
    "2077", "2087", "2466", "FF17",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D7D5", "01D7DF", "01D7E9", "01D7F3", "01D7FD",
#endif /* UNICODE >= 3.1.0 */
    NULL
};

static const char *table_8[] = {
    "2078", "2088", "2467", "FF18",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D7D6", "01D7E0", "01D7EA", "01D7F4", "01D7FE",
#endif /* UNICODE >= 3.1.0 */
    NULL
};

static const char *table_9[] = {
    "2079", "2089", "2468", "FF19",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D7D7", "01D7E1", "01D7EB", "01D7F5", "01D7FF",
#endif /* UNICODE >= 3.1.0 */
    NULL
};

static const char *table_a[] = {
    "00AA", "00C0", "00C1", "00C2", "00C3", "00C4", "00C5",
    "00E0", "00E1", "00E2", "00E3", "00E4", "00E5", "0100",
    "0101", "0102", "0103", "0104", "0105", "01CD", "01CE",
    "01DE", "01DF", "01E0", "01E1", "01FA", "01FB", "0200",
    "0201", "0202", "0203", "1E00", "1E01", "1EA0", "1EA1",
    "1EA2", "1EA3", "1EA4", "1EA5", "1EA6", "1EA7", "1EA8",
    "1EA9", "1EAA", "1EAB", "1EAC", "1EAD", "1EAE", "1EAF",
    "1EB0", "1EB1", "1EB2", "1EB3", "1EB4", "1EB5", "1EB6",
    "1EB7", "212B", "24B6", "24D0", "FF21", "FF41",
#if CAPTCHA_UNICODE_VERSION >= 3000
    "0226", "0227",
#endif /* UNICODE >= 3.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D400", "01D41A", "01D434", "01D44E", "01D468", "01D482",
    "01D49C", "01D4B6", "01D4D0", "01D4EA", "01D504", "01D51E",
    "01D538", "01D552", "01D56C", "01D586", "01D5A0", "01D5BA",
    "01D5D4", "01D5EE", "01D608", "01D622", "01D63C", "01D656",
    "01D670", "01D68A",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 4000
    "1D2C", "1D43",
#endif /* UNICODE >= 4.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 4100
    "2090",
#endif /* UNICODE >= 4.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 6000
    "01F130",
#endif /* UNICODE >= 6.0.0 */
    NULL
};

static const char *table_b[] = {
    "1E02", "1E03", "1E04", "1E05", "1E06", "1E07", "212C",
    "24B7", "24D1", "FF22", "FF42",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D401", "01D41B", "01D435", "01D44F", "01D469", "01D483",
    "01D4B7", "01D4D1", "01D4EB", "01D505", "01D51F", "01D539",
    "01D553", "01D56D", "01D587", "01D5A1", "01D5BB", "01D5D5",
    "01D5EF", "01D609", "01D623", "01D63D", "01D657", "01D671",
    "01D68B",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 4000
    "1D2E", "1D47",
#endif /* UNICODE >= 4.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 5200
    "01F131",
#endif /* UNICODE >= 5.2.0 */
    NULL
};

static const char *table_c[] = {
    "00C7", "00E7", "0106", "0107", "0108", "0109", "010A",
    "010B", "010C", "010D", "1E08", "1E09", "2102", "212D",
    "216D", "217D", "24B8", "24D2", "FF23", "FF43",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D402", "01D41C", "01D436", "01D450", "01D46A", "01D484",
    "01D49E", "01D4B8", "01D4D2", "01D4EC", "01D520", "01D554",
    "01D56E", "01D588", "01D5A2", "01D5BC", "01D5D6", "01D5F0",
    "01D60A", "01D624", "01D63E", "01D658", "01D672", "01D68C",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 4100
    "1D9C",
#endif /* UNICODE >= 4.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 5200
    "01F12B",
#endif /* UNICODE >= 5.2.0 */
#if CAPTCHA_UNICODE_VERSION >= 6000
    "01F132",
#endif /* UNICODE >= 6.0.0 */
    NULL
};

static const char *table_d[] = {
    "010E", "010F", "1E0A", "1E0B", "1E0C", "1E0D", "1E0E",
    "1E0F", "1E10", "1E11", "1E12", "1E13", "216E", "217E",
    "24B9", "24D3", "FF24", "FF44",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D403", "01D41D", "01D437", "01D451", "01D46B", "01D485",
    "01D49F", "01D4B9", "01D4D3", "01D4ED", "01D507", "01D521",
    "01D53B", "01D555", "01D56F", "01D589", "01D5A3", "01D5BD",
    "01D5D7", "01D5F1", "01D60B", "01D625", "01D63F", "01D659",
    "01D673", "01D68D",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 3200
    "2145", "2146",
#endif /* UNICODE >= 3.2.0 */
#if CAPTCHA_UNICODE_VERSION >= 4000
    "1D30", "1D48",
#endif /* UNICODE >= 4.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 6000
    "01F133",
#endif /* UNICODE >= 6.0.0 */
    NULL
};

static const char *table_e[] = {
    "00C8", "00C9", "00CA", "00CB", "00E8", "00E9", "00EA",
    "00EB", "0112", "0113", "0114", "0115", "0116", "0117",
    "0118", "0119", "011A", "011B", "0204", "0205", "0206",
    "0207", "1E14", "1E15", "1E16", "1E17", "1E18", "1E19",
    "1E1A", "1E1B", "1E1C", "1E1D", "1EB8", "1EB9", "1EBA",
    "1EBB", "1EBC", "1EBD", "1EBE", "1EBF", "1EC0", "1EC1",
    "1EC2", "1EC3", "1EC4", "1EC5", "1EC6", "1EC7", "212F",
    "2130", "24BA", "24D4", "FF25", "FF45",
#if CAPTCHA_UNICODE_VERSION >= 3000
    "0228", "0229",
#endif /* UNICODE >= 3.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D404", "01D41E", "01D438", "01D452", "01D46C", "01D486",
    "01D4D4", "01D4EE", "01D508", "01D522", "01D53C", "01D556",
    "01D570", "01D58A", "01D5A4", "01D5BE", "01D5D8", "01D5F2",
    "01D60C", "01D626", "01D640", "01D65A", "01D674", "01D68E",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 3200
    "2147",
#endif /* UNICODE >= 3.2.0 */
#if CAPTCHA_UNICODE_VERSION >= 4000
    "1D31", "1D49",
#endif /* UNICODE >= 4.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 4100
    "2091",
#endif /* UNICODE >= 4.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 6000
    "01F134",
#endif /* UNICODE >= 6.0.0 */
    NULL
};

static const char *table_f[] = {
    "1E1E", "1E1F", "2131", "24BB", "24D5", "FF26", "FF46",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D405", "01D41F", "01D439", "01D453", "01D46D", "01D487",
    "01D4BB", "01D4D5", "01D4EF", "01D509", "01D523", "01D53D",
    "01D557", "01D571", "01D58B", "01D5A5", "01D5BF", "01D5D9",
    "01D5F3", "01D60D", "01D627", "01D641", "01D65B", "01D675",
    "01D68F",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 4100
    "1DA0",
#endif /* UNICODE >= 4.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 6000
    "01F135",
#endif /* UNICODE >= 6.0.0 */
    NULL
};

static const char *table_g[] = {
    "011C", "011D", "011E", "011F", "0120", "0121", "0122",
    "0123", "01E6", "01E7", "01F4", "01F5", "1E20", "1E21",
    "210A", "24BC", "24D6", "FF27", "FF47",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D406", "01D420", "01D43A", "01D454", "01D46E", "01D488",
    "01D4A2", "01D4D6", "01D4F0", "01D50A", "01D524", "01D53E",
    "01D558", "01D572", "01D58C", "01D5A6", "01D5C0", "01D5DA",
    "01D5F4", "01D60E", "01D628", "01D642", "01D65C", "01D676",
    "01D690",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 4000
    "1D33", "1D4D",
#endif /* UNICODE >= 4.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 6000
    "01F136",
#endif /* UNICODE >= 6.0.0 */
    NULL
};

static const char *table_h[] = {
    "0124", "0125", "02B0", "1E22", "1E23", "1E24", "1E25",
    "1E26", "1E27", "1E28", "1E29", "1E2A", "1E2B", "1E96",
    "210B", "210C", "210D", "210E", "24BD", "24D7", "FF28",
    "FF48",
#if CAPTCHA_UNICODE_VERSION >= 3000
    "021E", "021F",
#endif /* UNICODE >= 3.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D407", "01D421", "01D43B", "01D46F", "01D489", "01D4BD",
    "01D4D7", "01D4F1", "01D525", "01D559", "01D573", "01D58D",
    "01D5A7", "01D5C1", "01D5DB", "01D5F5", "01D60F", "01D629",
    "01D643", "01D65D", "01D677", "01D691",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 4000
    "1D34",
#endif /* UNICODE >= 4.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 6000
    "2095", "01F137",
#endif /* UNICODE >= 6.0.0 */
    NULL
};

static const char *table_i[] = {
    "00CC", "00CD", "00CE", "00CF", "00EC", "00ED", "00EE",
    "00EF", "0128", "0129", "012A", "012B", "012C", "012D",
    "012E", "012F", "0130", "01CF", "01D0", "0208", "0209",
    "020A", "020B", "1E2C", "1E2D", "1E2E", "1E2F", "1EC8",
    "1EC9", "1ECA", "1ECB", "2110", "2111", "2160", "2170",
    "24BE", "24D8", "FF29", "FF49",
#if CAPTCHA_UNICODE_VERSION >= 3000
    "2139",
#endif /* UNICODE >= 3.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D408", "01D422", "01D43C", "01D456", "01D470", "01D48A",
    "01D4BE", "01D4D8", "01D4F2", "01D526", "01D540", "01D55A",
    "01D574", "01D58E", "01D5A8", "01D5C2", "01D5DC", "01D5F6",
    "01D610", "01D62A", "01D644", "01D65E", "01D678", "01D692",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 3200
    "2071", "2148",
#endif /* UNICODE >= 3.2.0 */
#if CAPTCHA_UNICODE_VERSION >= 4000
    "1D35", "1D62",
#endif /* UNICODE >= 4.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 6000
    "01F138",
#endif /* UNICODE >= 6.0.0 */
    NULL
};

static const char *table_j[] = {
    "0134", "0135", "01F0", "02B2", "24BF", "24D9", "FF2A",
    "FF4A",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D409", "01D423", "01D43D", "01D457", "01D471", "01D48B",
    "01D4A5", "01D4BF", "01D4D9", "01D4F3", "01D50D", "01D527",
    "01D541", "01D55B", "01D575", "01D58F", "01D5A9", "01D5C3",
    "01D5DD", "01D5F7", "01D611", "01D62B", "01D645", "01D65F",
    "01D679", "01D693",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 3200
    "2149",
#endif /* UNICODE >= 3.2.0 */
#if CAPTCHA_UNICODE_VERSION >= 4000
    "1D36",
#endif /* UNICODE >= 4.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 5100
    "2C7C",
#endif /* UNICODE >= 5.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 6000
    "01F139",
#endif /* UNICODE >= 6.0.0 */
    NULL
};

static const char *table_k[] = {
    "0136", "0137", "01E8", "01E9", "1E30", "1E31", "1E32",
    "1E33", "1E34", "1E35", "212A", "24C0", "24DA", "FF2B",
    "FF4B",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D40A", "01D424", "01D43E", "01D458", "01D472", "01D48C",
    "01D4A6", "01D4C0", "01D4DA", "01D4F4", "01D50E", "01D528",
    "01D542", "01D55C", "01D576", "01D590", "01D5AA", "01D5C4",
    "01D5DE", "01D5F8", "01D612", "01D62C", "01D646", "01D660",
    "01D67A", "01D694",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 4000
    "1D37", "1D4F",
#endif /* UNICODE >= 4.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 6000
    "2096", "01F13A",
#endif /* UNICODE >= 6.0.0 */
    NULL
};

static const char *table_l[] = {
    "0139", "013A", "013B", "013C", "013D", "013E", "02E1",
    "1E36", "1E37", "1E38", "1E39", "1E3A", "1E3B", "1E3C",
    "1E3D", "2112", "2113", "216C", "217C", "24C1", "24DB",
    "FF2C", "FF4C",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D40B", "01D425", "01D43F", "01D459", "01D473", "01D48D",
    "01D4DB", "01D4F5", "01D50F", "01D529", "01D543", "01D55D",
    "01D577", "01D591", "01D5AB", "01D5C5", "01D5DF", "01D5F9",
    "01D613", "01D62D", "01D647", "01D661", "01D67B", "01D695",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 4000
    "1D38", "01D4C1",
#endif /* UNICODE >= 4.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 6000
    "2097", "01F13B",
#endif /* UNICODE >= 6.0.0 */
    NULL
};

static const char *table_m[] = {
    "1E3E", "1E3F", "1E40", "1E41", "1E42", "1E43", "2133",
    "216F", "217F", "24C2", "24DC", "FF2D", "FF4D",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D40C", "01D426", "01D440", "01D45A", "01D474", "01D48E",
    "01D4C2", "01D4DC", "01D4F6", "01D510", "01D52A", "01D544",
    "01D55E", "01D578", "01D592", "01D5AC", "01D5C6", "01D5E0",
    "01D5FA", "01D614", "01D62E", "01D648", "01D662", "01D67C",
    "01D696",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 4000
    "1D39", "1D50",
#endif /* UNICODE >= 4.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 6000
    "2098", "01F13C",
#endif /* UNICODE >= 6.0.0 */
    NULL
};

static const char *table_n[] = {
    "00D1", "00F1", "0143", "0144", "0145", "0146", "0147",
    "0148", "1E44", "1E45", "1E46", "1E47", "1E48", "1E49",
    "1E4A", "1E4B", "207F", "2115", "24C3", "24DD", "FF2E",
    "FF4E",
#if CAPTCHA_UNICODE_VERSION >= 3000
    "01F8", "01F9",
#endif /* UNICODE >= 3.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D40D", "01D427", "01D441", "01D45B", "01D475", "01D48F",
    "01D4A9", "01D4C3", "01D4DD", "01D4F7", "01D511", "01D52B",
    "01D55F", "01D579", "01D593", "01D5AD", "01D5C7", "01D5E1",
    "01D5FB", "01D615", "01D62F", "01D649", "01D663", "01D67D",
    "01D697",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 4000
    "1D3A",
#endif /* UNICODE >= 4.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 5200
    "01F13D",
#endif /* UNICODE >= 5.2.0 */
#if CAPTCHA_UNICODE_VERSION >= 6000
    "2099",
#endif /* UNICODE >= 6.0.0 */
    NULL
};

static const char *table_o[] = {
    "00BA", "00D2", "00D3", "00D4", "00D5", "00D6", "00F2",
    "00F3", "00F4", "00F5", "00F6", "014C", "014D", "014E",
    "014F", "0150", "0151", "01A0", "01A1", "01D1", "01D2",
    "01EA", "01EB", "01EC", "01ED", "020C", "020D", "020E",
    "020F", "1E4C", "1E4D", "1E4E", "1E4F", "1E50", "1E51",
    "1E52", "1E53", "1ECC", "1ECD", "1ECE", "1ECF", "1ED0",
    "1ED1", "1ED2", "1ED3", "1ED4", "1ED5", "1ED6", "1ED7",
    "1ED8", "1ED9", "1EDA", "1EDB", "1EDC", "1EDD", "1EDE",
    "1EDF", "1EE0", "1EE1", "1EE2", "1EE3", "2134", "24C4",
    "24DE", "FF2F", "FF4F",
#if CAPTCHA_UNICODE_VERSION >= 3000
    "022A", "022B", "022C", "022D", "022E", "022F", "0230",
    "0231",
#endif /* UNICODE >= 3.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D40E", "01D428", "01D442", "01D45C", "01D476", "01D490",
    "01D4AA", "01D4DE", "01D4F8", "01D512", "01D52C", "01D546",
    "01D560", "01D57A", "01D594", "01D5AE", "01D5C8", "01D5E2",
    "01D5FC", "01D616", "01D630", "01D64A", "01D664", "01D67E",
    "01D698",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 4000
    "1D3C", "1D52",
#endif /* UNICODE >= 4.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 4100
    "2092",
#endif /* UNICODE >= 4.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 6000
    "01F13E",
#endif /* UNICODE >= 6.0.0 */
    NULL
};

static const char *table_p[] = {
    "1E54", "1E55", "1E56", "1E57", "2119", "24C5", "24DF",
    "FF30", "FF50",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D40F", "01D429", "01D443", "01D45D", "01D477", "01D491",
    "01D4AB", "01D4C5", "01D4DF", "01D4F9", "01D513", "01D52D",
    "01D561", "01D57B", "01D595", "01D5AF", "01D5C9", "01D5E3",
    "01D5FD", "01D617", "01D631", "01D64B", "01D665", "01D67F",
    "01D699",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 4000
    "1D3E", "1D56",
#endif /* UNICODE >= 4.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 5200
    "01F13F",
#endif /* UNICODE >= 5.2.0 */
#if CAPTCHA_UNICODE_VERSION >= 6000
    "209A",
#endif /* UNICODE >= 6.0.0 */
    NULL
};

static const char *table_q[] = {
    "211A", "24C6", "24E0", "FF31", "FF51",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D410", "01D42A", "01D444", "01D45E", "01D478", "01D492",
    "01D4AC", "01D4C6", "01D4E0", "01D4FA", "01D514", "01D52E",
    "01D562", "01D57C", "01D596", "01D5B0", "01D5CA", "01D5E4",
    "01D5FE", "01D618", "01D632", "01D64C", "01D666", "01D680",
    "01D69A",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 6000
    "01F140",
#endif /* UNICODE >= 6.0.0 */
    NULL
};

static const char *table_r[] = {
    "0154", "0155", "0156", "0157", "0158", "0159", "0210",
    "0211", "0212", "0213", "02B3", "1E58", "1E59", "1E5A",
    "1E5B", "1E5C", "1E5D", "1E5E", "1E5F", "211B", "211C",
    "211D", "24C7", "24E1", "FF32", "FF52",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D411", "01D42B", "01D445", "01D45F", "01D479", "01D493",
    "01D4C7", "01D4E1", "01D4FB", "01D52F", "01D563", "01D57D",
    "01D597", "01D5B1", "01D5CB", "01D5E5", "01D5FF", "01D619",
    "01D633", "01D64D", "01D667", "01D681", "01D69B",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 4000
    "1D3F", "1D63",
#endif /* UNICODE >= 4.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 5200
    "01F12C",
#endif /* UNICODE >= 5.2.0 */
#if CAPTCHA_UNICODE_VERSION >= 6000
    "01F141",
#endif /* UNICODE >= 6.0.0 */
    NULL
};

static const char *table_s[] = {
    "015A", "015B", "015C", "015D", "015E", "015F", "0160",
    "0161", "017F", "02E2", "1E60", "1E61", "1E62", "1E63",
    "1E64", "1E65", "1E66", "1E67", "1E68", "1E69", "24C8",
    "24E2", "FF33", "FF53",
#if CAPTCHA_UNICODE_VERSION >= 2000
    "1E9B",
#endif /* UNICODE >= 2.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 3000
    "0218", "0219",
#endif /* UNICODE >= 3.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D412", "01D42C", "01D446", "01D460", "01D47A", "01D494",
    "01D4AE", "01D4C8", "01D4E2", "01D4FC", "01D516", "01D530",
    "01D54A", "01D564", "01D57E", "01D598", "01D5B2", "01D5CC",
    "01D5E6", "01D600", "01D61A", "01D634", "01D64E", "01D668",
    "01D682", "01D69C",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 5200
    "01F142",
#endif /* UNICODE >= 5.2.0 */
#if CAPTCHA_UNICODE_VERSION >= 6000
    "209B",
#endif /* UNICODE >= 6.0.0 */
    NULL
};

static const char *table_t[] = {
    "0162", "0163", "0164", "0165", "1E6A", "1E6B", "1E6C",
    "1E6D", "1E6E", "1E6F", "1E70", "1E71", "1E97", "24C9",
    "24E3", "FF34", "FF54",
#if CAPTCHA_UNICODE_VERSION >= 3000
    "021A", "021B",
#endif /* UNICODE >= 3.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D413", "01D42D", "01D447", "01D461", "01D47B", "01D495",
    "01D4AF", "01D4C9", "01D4E3", "01D4FD", "01D517", "01D531",
    "01D54B", "01D565", "01D57F", "01D599", "01D5B3", "01D5CD",
    "01D5E7", "01D601", "01D61B", "01D635", "01D64F", "01D669",
    "01D683", "01D69D",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 4000
    "1D40", "1D57",
#endif /* UNICODE >= 4.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 6000
    "209C", "01F143",
#endif /* UNICODE >= 6.0.0 */
    NULL
};

static const char *table_u[] = {
    "00D9", "00DA", "00DB", "00DC", "00F9", "00FA", "00FB",
    "00FC", "0168", "0169", "016A", "016B", "016C", "016D",
    "016E", "016F", "0170", "0171", "0172", "0173", "01AF",
    "01B0", "01D3", "01D4", "01D5", "01D6", "01D7", "01D8",
    "01D9", "01DA", "01DB", "01DC", "0214", "0215", "0216",
    "0217", "1E72", "1E73", "1E74", "1E75", "1E76", "1E77",
    "1E78", "1E79", "1E7A", "1E7B", "1EE4", "1EE5", "1EE6",
    "1EE7", "1EE8", "1EE9", "1EEA", "1EEB", "1EEC", "1EED",
    "1EEE", "1EEF", "1EF0", "1EF1", "24CA", "24E4", "FF35",
    "FF55",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D414", "01D42E", "01D448", "01D462", "01D47C", "01D496",
    "01D4B0", "01D4CA", "01D4E4", "01D4FE", "01D518", "01D532",
    "01D54C", "01D566", "01D580", "01D59A", "01D5B4", "01D5CE",
    "01D5E8", "01D602", "01D61C", "01D636", "01D650", "01D66A",
    "01D684", "01D69E",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 4000
    "1D41", "1D58", "1D64",
#endif /* UNICODE >= 4.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 6000
    "01F144",
#endif /* UNICODE >= 6.0.0 */
    NULL
};

static const char *table_v[] = {
    "1E7C", "1E7D", "1E7E", "1E7F", "2164", "2174", "24CB",
    "24E5", "FF36", "FF56",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D415", "01D42F", "01D449", "01D463", "01D47D", "01D497",
    "01D4B1", "01D4CB", "01D4E5", "01D4FF", "01D519", "01D533",
    "01D54D", "01D567", "01D581", "01D59B", "01D5B5", "01D5CF",
    "01D5E9", "01D603", "01D61D", "01D637", "01D651", "01D66B",
    "01D685", "01D69F",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 4000
    "1D5B", "1D65",
#endif /* UNICODE >= 4.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 5100
    "2C7D",
#endif /* UNICODE >= 5.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 6000
    "01F145",
#endif /* UNICODE >= 6.0.0 */
    NULL
};

static const char *table_w[] = {
    "0174", "0175", "02B7", "1E80", "1E81", "1E82", "1E83",
    "1E84", "1E85", "1E86", "1E87", "1E88", "1E89", "1E98",
    "24CC", "24E6", "FF37", "FF57",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D416", "01D430", "01D44A", "01D464", "01D47E", "01D498",
    "01D4B2", "01D4CC", "01D4E6", "01D500", "01D51A", "01D534",
    "01D54E", "01D568", "01D582", "01D59C", "01D5B6", "01D5D0",
    "01D5EA", "01D604", "01D61E", "01D638", "01D652", "01D66C",
    "01D686", "01D6A0",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 4000
    "1D42",
#endif /* UNICODE >= 4.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 5200
    "01F146",
#endif /* UNICODE >= 5.2.0 */
    NULL
};

static const char *table_x[] = {
    "02E3", "1E8A", "1E8B", "1E8C", "1E8D", "2169", "2179",
    "24CD", "24E7", "FF38", "FF58",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D417", "01D431", "01D44B", "01D465", "01D47F", "01D499",
    "01D4B3", "01D4CD", "01D4E7", "01D501", "01D51B", "01D535",
    "01D54F", "01D569", "01D583", "01D59D", "01D5B7", "01D5D1",
    "01D5EB", "01D605", "01D61F", "01D639", "01D653", "01D66D",
    "01D687", "01D6A1",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 4100
    "2093",
#endif /* UNICODE >= 4.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 6000
    "01F147",
#endif /* UNICODE >= 6.0.0 */
    NULL
};

static const char *table_y[] = {
    "00DD", "00FD", "00FF", "0176", "0177", "0178", "02B8",
    "1E8E", "1E8F", "1E99", "1EF2", "1EF3", "1EF4", "1EF5",
    "1EF6", "1EF7", "1EF8", "1EF9", "24CE", "24E8", "FF39",
    "FF59",
#if CAPTCHA_UNICODE_VERSION >= 3000
    "0232", "0233",
#endif /* UNICODE >= 3.0.0 */
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D418", "01D432", "01D44C", "01D466", "01D480", "01D49A",
    "01D4B4", "01D4CE", "01D4E8", "01D502", "01D51C", "01D536",
    "01D550", "01D56A", "01D584", "01D59E", "01D5B8", "01D5D2",
    "01D5EC", "01D606", "01D620", "01D63A", "01D654", "01D66E",
    "01D688", "01D6A2",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 6000
    "01F148",
#endif /* UNICODE >= 6.0.0 */
    NULL
};

static const char *table_z[] = {
    "0179", "017A", "017B", "017C", "017D", "017E", "1E90",
    "1E91", "1E92", "1E93", "1E94", "1E95", "2124", "2128",
    "24CF", "24E9", "FF3A", "FF5A",
#if CAPTCHA_UNICODE_VERSION >= 3100
    "01D419", "01D433", "01D44D", "01D467", "01D481", "01D49B",
    "01D4B5", "01D4CF", "01D4E9", "01D503", "01D537", "01D56B",
    "01D585", "01D59F", "01D5B9", "01D5D3", "01D5ED", "01D607",
    "01D621", "01D63B", "01D655", "01D66F", "01D689", "01D6A3",
#endif /* UNICODE >= 3.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 4100
    "1DBB",
#endif /* UNICODE >= 4.1.0 */
#if CAPTCHA_UNICODE_VERSION >= 6000
    "01F149",
#endif /* UNICODE >= 6.0.0 */
    NULL
};

struct table_component {
    const char **tbl;
    size_t length;
} table[] = {
    { table_0, ARRAY_SIZE(table_0) - 1 },
    { table_1, ARRAY_SIZE(table_1) - 1 },
    { table_2, ARRAY_SIZE(table_2) - 1 },
    { table_3, ARRAY_SIZE(table_3) - 1 },
    { table_4, ARRAY_SIZE(table_4) - 1 },
    { table_5, ARRAY_SIZE(table_5) - 1 },
    { table_6, ARRAY_SIZE(table_6) - 1 },
    { table_7, ARRAY_SIZE(table_7) - 1 },
    { table_8, ARRAY_SIZE(table_8) - 1 },
    { table_9, ARRAY_SIZE(table_9) - 1 },
    { table_a, ARRAY_SIZE(table_a) - 1 },
    { table_b, ARRAY_SIZE(table_b) - 1 },
    { table_c, ARRAY_SIZE(table_c) - 1 },
    { table_d, ARRAY_SIZE(table_d) - 1 },
    { table_e, ARRAY_SIZE(table_e) - 1 },
    { table_f, ARRAY_SIZE(table_f) - 1 },
    { table_g, ARRAY_SIZE(table_g) - 1 },
    { table_h, ARRAY_SIZE(table_h) - 1 },
    { table_i, ARRAY_SIZE(table_i) - 1 },
    { table_j, ARRAY_SIZE(table_j) - 1 },
    { table_k, ARRAY_SIZE(table_k) - 1 },
    { table_l, ARRAY_SIZE(table_l) - 1 },
    { table_m, ARRAY_SIZE(table_m) - 1 },
    { table_n, ARRAY_SIZE(table_n) - 1 },
    { table_o, ARRAY_SIZE(table_o) - 1 },
    { table_p, ARRAY_SIZE(table_p) - 1 },
    { table_q, ARRAY_SIZE(table_q) - 1 },
    { table_r, ARRAY_SIZE(table_r) - 1 },
    { table_s, ARRAY_SIZE(table_s) - 1 },
    { table_t, ARRAY_SIZE(table_t) - 1 },
    { table_u, ARRAY_SIZE(table_u) - 1 },
    { table_v, ARRAY_SIZE(table_v) - 1 },
    { table_w, ARRAY_SIZE(table_w) - 1 },
    { table_x, ARRAY_SIZE(table_x) - 1 },
    { table_y, ARRAY_SIZE(table_y) - 1 },
    { table_z, ARRAY_SIZE(table_z) - 1 }
};

static const char *ignorables[] = {
    /*"\\0009", "\\000A", "\\000C", "\\000D", */"\\0020", "\\00A0",
    "\\1680", "\\180E", "\\2000", "\\2001", "\\2002", "\\2003",
    "\\2004", "\\2005", "\\2006", "\\2007", "\\2008", "\\2009",
    "\\200A", "\\2028", "\\2029", "\\202F", "\\205F", "\\3000"
};

static const char shuffling[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, /* reserved to challenge characters */
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F  /* reserved to fake characters */
};

#define MAX_CHALLENGE_LENGTH (ARRAY_SIZE(shuffling) / 2)

#define CAPTCHA_FETCH_OBJ(/*Captcha_object **/ co, /*zval **/ object)                                              \
    do {                                                                                                           \
        co = (Captcha_object *) zend_object_store_get_object(object TSRMLS_CC);                                    \
        if (NULL == co->key) {                                                                                     \
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid or unitialized %s object", Captcha_ce_ptr->name); \
            RETURN_FALSE;                                                                                          \
        }                                                                                                          \
    } while (0);

#define COMPLETE_SESSION_KEY(/*Captcha_object **/ co, /*char **/ name, /*long*/ name_len) \
    do {                                                                                  \
        name_len = strlen(CAPTCHA_ATTR(session_prefix)) + co->key_len;                    \
        name = emalloc(name_len + 1);                                                     \
        strcpy(name, CAPTCHA_ATTR(session_prefix));                                       \
        strcat(name, co->key);                                                            \
    } while (0);

#define SESSION_IS_ACTIVE() \
    (PS(http_session_vars) && IS_ARRAY == PS(http_session_vars)->type)

static void php_string_shuffle(char *str, long len TSRMLS_DC)
{
    long n_elems, rnd_idx, n_left;
    char temp;
    /* The implementation is stolen from array_data_shuffle       */
    /* Thus the characteristics of the randomization are the same */
    n_elems = len;

    if (n_elems <= 1) {
        return;
    }

    n_left = n_elems;

    while (--n_left) {
        rnd_idx = php_rand(TSRMLS_C);
        RAND_RANGE(rnd_idx, 0, n_left, PHP_RAND_MAX);
        if (rnd_idx != n_left) {
            temp = str[n_left];
            str[n_left] = str[rnd_idx];
            str[rnd_idx] = temp;
        }
    }
}

static long captcha_rand(long max TSRMLS_DC)
{
    long rnd_idx;

    rnd_idx = php_rand(TSRMLS_C);
    RAND_RANGE(rnd_idx, 0, max, PHP_RAND_MAX);

    return rnd_idx;
}

static long captcha_rand_range(long min, long max TSRMLS_DC)
{
    long rnd_idx;

    rnd_idx = php_rand(TSRMLS_C);
    RAND_RANGE(rnd_idx, min, max, PHP_RAND_MAX);

    return rnd_idx;
}

static char *random_string(long length TSRMLS_DC)
{
    long i;
    char *k;

    k = emalloc(length + 1);
    for (i = 0; i < length; i++) {
        k[i] = alphabet[captcha_rand(STR_LEN(alphabet) - 1 TSRMLS_CC)];
    }
    k[i] = '\0';

    return k;
}

static int is_subset_of(zval *string, const char *set, size_t set_len)
{
    size_t i;
    uint8_t characters[256];

    memset(characters, 0, ARRAY_SIZE(characters));
    for (i = 0; i < set_len; i++) {
        characters[(unsigned char) set[i]] = 1;
    }
    for (i = 0; i < Z_STRLEN_P(string); i++) {
        if (!characters[(unsigned char) Z_STRVAL_P(string)[i]]) {
            return 0;
        }
    }

    return 1;
}

static void captcha_fetch_or_create_challenge(Captcha_object* co, int renew TSRMLS_DC)
{
    char *name;
    size_t name_len;
    zval **zcontainer, **zchallenge, **zattemps, **zfakes;

    if (SESSION_IS_ACTIVE()) {
        COMPLETE_SESSION_KEY(co, name, name_len);
        if (
            !renew
            && SUCCESS == zend_symtable_find(Z_ARRVAL_P(PS(http_session_vars)), name, name_len + 1, (void **) &zcontainer)
            && IS_ARRAY == Z_TYPE_PP(zcontainer)
            && SUCCESS == zend_hash_find(Z_ARRVAL_PP(zcontainer), "challenge", sizeof("challenge"), (void **) &zchallenge)
            && IS_STRING == Z_TYPE_PP(zchallenge)
            && is_subset_of(*zchallenge, alphabet, STR_LEN(alphabet))
            && SUCCESS == zend_hash_find(Z_ARRVAL_PP(zcontainer), "attempts", sizeof("attempts"), (void **) &zattemps)
            && IS_LONG == Z_TYPE_PP(zattemps)
        ) {
            co->container = *zcontainer;
            co->challenge = *zchallenge;
            co->attempts = *zattemps;
            co->fakes = NULL;
            if (
                SUCCESS == zend_hash_find(Z_ARRVAL_PP(zcontainer), "fakes", sizeof("fakes"), (void **) &zfakes)
                && IS_ARRAY == Z_TYPE_PP(zfakes)
                && zend_hash_num_elements(Z_ARRVAL_PP(zfakes)) > 0
            ) {
                co->fakes = *zfakes;
            }
        } else {
            long challenge_len;
            const char *challenge;

            co->fakes = NULL;
            challenge_len = CAPTCHA_ATTR(challenge_length);
            challenge = random_string(challenge_len TSRMLS_CC);
            ALLOC_INIT_ZVAL(co->challenge);
            ZVAL_STRINGL(co->challenge, challenge, challenge_len, 0);

            ALLOC_INIT_ZVAL(co->attempts);
            ZVAL_LONG(co->attempts, 0);

            ALLOC_INIT_ZVAL(co->container);
            array_init(co->container);
            add_assoc_zval_ex(co->container, "attempts", sizeof("attempts"), co->attempts);
            add_assoc_zval_ex(co->container, "challenge", sizeof("challenge"), co->challenge);
            if (CAPTCHA_ATTR(fake_characters_length)) {
                int i;
                char index[MAX_CHALLENGE_LENGTH];

                memcpy(index, shuffling, challenge_len);
                php_string_shuffle(index, challenge_len TSRMLS_CC);

                ALLOC_INIT_ZVAL(co->fakes);
                array_init(co->fakes);
                for (i = 0; i < CAPTCHA_ATTR(fake_characters_length); i++) {
                    add_index_stringl(co->fakes, index[i], alphabet + captcha_rand(STR_LEN(alphabet) - 1 TSRMLS_CC), 1, 1);
                }
                add_assoc_zval_ex(co->container, "fakes", sizeof("fakes"), co->fakes);
            }
            ZEND_SET_SYMBOL_WITH_LENGTH(Z_ARRVAL_P(PS(http_session_vars)), name, name_len + 1, co->container, 1, 0);
        }
        efree(name);
    } else {
        zend_throw_exception_ex(NULL, 0 TSRMLS_CC, CAPTCHA_CLASS_NAME " implies an active session");
    }
}

static int check_color_attribute(zval *value TSRMLS_DC)
{
    // we can assume that value is a "long" due to anterior convert_to_long
    if (Z_LVAL_P(value) < 0 || Z_LVAL_P(value) >= CAPTCHA_ATTR_PREFIX(COUNT)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid color value: %ld", Z_LVAL_P(value));
        return 0;
    } else {
        return 1;
    }
}

static int check_challenge_length_attribute(zval *value TSRMLS_DC)
{
    // we can assume that value is a "long" due to anterior convert_to_long
    if (Z_LVAL_P(value) <= 0 || Z_LVAL_P(value) >= MAX_CHALLENGE_LENGTH) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Challenge length must be in the range ]0;%lu]", MAX_CHALLENGE_LENGTH);
        return 0;
    } else {
        return 1;
    }
}

static int check_fake_characters_length_attribute(zval *value TSRMLS_DC)
{
    // we can assume that value is a "long" due to anterior convert_to_long
    if (Z_LVAL_P(value) < 0 || Z_LVAL_P(value) >= MAX_CHALLENGE_LENGTH) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Fake characters length must be in the range [0;%lu]", MAX_CHALLENGE_LENGTH);
        return 0;
    } else {
        return 1;
    }
}

static int check_zero_or_positive_attribute(zval *value TSRMLS_DC)
{
    // we can assume that value is a "long" due to anterior convert_to_long
    if (Z_LVAL_P(value) < 0) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Noise length can't be negative");
        return 0;
    } else {
        return 1;
    }
}

static long captcha_set_attribute(Captcha_object* co, ulong attribute, zval **value TSRMLS_DC)
{
    switch (attribute) {
#define BOOL_CAPTCHA_ATTRIBUTE(member, name, defaultvalue) \
        case CAPTCHA_ATTR_PREFIX(name):
#define LONG_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb)
#define STRING_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb)
#include "captcha_attributes.h"
#undef BOOL_CAPTCHA_ATTRIBUTE
#undef LONG_CAPTCHA_ATTRIBUTE
#undef STRING_CAPTCHA_ATTRIBUTE
        {
            convert_to_boolean(*value);
            *((zend_bool *) (((char *) co) + attributes[attribute].offset)) = Z_BVAL_PP(value);
            return 1;
        }
#define BOOL_CAPTCHA_ATTRIBUTE(member, name, defaultvalue)
#define LONG_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb) \
        case CAPTCHA_ATTR_PREFIX(name):
#define STRING_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb)
#include "captcha_attributes.h"
#undef BOOL_CAPTCHA_ATTRIBUTE
#undef LONG_CAPTCHA_ATTRIBUTE
#undef STRING_CAPTCHA_ATTRIBUTE
        {
            convert_to_long(*value);
            if (NULL == attributes[attribute].cb || attributes[attribute].cb(*value TSRMLS_CC)) {
                *((long *) (((char *) co) + attributes[attribute].offset)) = Z_LVAL_PP(value);
                return 1;
            }
            break;
        }
#define BOOL_CAPTCHA_ATTRIBUTE(member, name, defaultvalue)
#define LONG_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb)
#define STRING_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb) \
        case CAPTCHA_ATTR_PREFIX(name):
#include "captcha_attributes.h"
#undef BOOL_CAPTCHA_ATTRIBUTE
#undef LONG_CAPTCHA_ATTRIBUTE
#undef STRING_CAPTCHA_ATTRIBUTE
        {
            convert_to_string(*value);
            efree(*((char **) (((char *) co) + attributes[attribute].offset)));
            *((char **) (((char *) co) + attributes[attribute].offset)) = estrndup(Z_STRVAL_PP(value), Z_STRLEN_PP(value));
            *((long *) (((char *) co) + attributes[attribute].offset + sizeof(char *))) = Z_STRLEN_PP(value);
            return 1;
        }
        default:
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unknown attribute %lu", attribute);
    }

    return 0;
}

static void captcha_ctor(INTERNAL_FUNCTION_PARAMETERS)
{
    char *name;
    size_t name_len;
    char *key = NULL;
    long key_len = 0;
    Captcha_object* co;
    zval *object, *options = NULL;

    object = return_value;
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|a", &key, &key_len, &options)) {
        zval_dtor(return_value);
        RETURN_NULL();
    }
    co = (Captcha_object *) zend_object_store_get_object(object TSRMLS_CC);
#define BOOL_CAPTCHA_ATTRIBUTE(member, name, defaultvalue) \
    co->member = defaultvalue;
#define LONG_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb) \
    co->member = defaultvalue;
#define STRING_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb) \
    co->member = estrndup(defaultvalue, co->member##_len = STR_LEN(defaultvalue));
#include "captcha_attributes.h"
#undef BOOL_CAPTCHA_ATTRIBUTE
#undef LONG_CAPTCHA_ATTRIBUTE
#undef STRING_CAPTCHA_ATTRIBUTE
    if (options) {
        char *str_key;
        ulong long_key;
        zval **attr_value;

        zend_hash_internal_pointer_reset(Z_ARRVAL_P(options));
        while (SUCCESS == zend_hash_get_current_data(Z_ARRVAL_P(options), (void**) &attr_value) && HASH_KEY_IS_LONG == zend_hash_get_current_key(Z_ARRVAL_P(options), &str_key, &long_key, 0)) {
            captcha_set_attribute(co, long_key, attr_value TSRMLS_CC);
            zend_hash_move_forward(Z_ARRVAL_P(options));
        }
    }
    if (key_len == 0) {
        zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "Key must not be empty");
        zval_dtor(return_value);
        RETURN_NULL();
    } else {
        co->key = estrdup(key);
        co->key_len = key_len;
    }
    captcha_fetch_or_create_challenge(co, 0 TSRMLS_CC);
}

PHP_FUNCTION(captcha_create)
{
    object_init_ex(return_value, Captcha_ce_ptr);
    captcha_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

PHP_METHOD(Captcha, __construct)
{
    return_value = getThis();
    captcha_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

PHP_FUNCTION(captcha_cleanup)
{
    char *name;
    size_t name_len;
    zval *object = NULL;
    Captcha_object *co = NULL;

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, Captcha_ce_ptr)) {
        RETURN_FALSE;
    }
    if (SESSION_IS_ACTIVE()) {
        CAPTCHA_FETCH_OBJ(co, object);
        COMPLETE_SESSION_KEY(co, name, name_len);
        if (SUCCESS == zend_hash_del(Z_ARRVAL_P(PS(http_session_vars)), name, name_len + 1)) {
            RETURN_TRUE;
        } else {
            RETURN_FALSE;
        }
    } else {
        RETURN_FALSE;
    }
}

#define smart_str_append_static(/*smart_str **/ self, /*const char **/ s) \
    do {                                                                  \
        smart_str_appendl(self, s, STR_LEN(s));                           \
    } while (0);

#define smart_str_append_static_repeated(/*smart_str **/ self, /*size_t*/ times, /*const char **/ s) \
    do {                                                                                             \
        register size_t __nl, __i;                                                                   \
        smart_str *__dest = (smart_str *) (self);                                                    \
        smart_str_alloc4(__dest, STR_LEN((s)) * (times), 0, __nl);                                   \
        for (__i = 0; __i < (times); __i++) {                                                        \
            memcpy(__dest->c + __dest->len, (s), STR_LEN((s)));                                      \
            __dest->len += STR_LEN((s));                                                             \
        }                                                                                            \
    } while (0);

#define char2int(/*char*/ c) \
    ((c & 0x40) ? (10 + c - 0x61) : (c - 0x30))

static double hue_to_rgb(double m1, double m2, uint16_t h)
{
    if (h < 0) {
        h += 360;
    }
    if (h > 360) {
        h -= 360;
    }
    if (h < 60)  {
        return (m1 + (m2 - m1) * (h / 60.0)) * 255.5;
    }
    if (h < 180) {
        return (m2 * 255.5);
    }
    if (h < 240) {
        return (m1 + (m2 - m1) * ((240 - h) / 60.0)) * 255.5;
    }

    return m1 * 255.5;
}

static void hsl_to_rgb(uint16_t h, uint8_t _s, uint8_t _l, uint8_t *r, uint8_t *g, uint8_t *b)
{
    double s, l, m1, m2;

    if (_l == 0) {
        *r = *g = *b = 0;
    } else {
        s = _s / 100.0;
        l = _l / 100.0;
        if (l <= 0.5) {
            m2 = l * (s + 1);
        } else {
            m2 = l + s - l * s;
        }
        m1 = l * 2 - m2;
        *r = hue_to_rgb(m1, m2, h + 120);
        *g = hue_to_rgb(m1, m2, h);
        *b = hue_to_rgb(m1, m2, h - 120);
    }
}

static void set_color(smart_str *ret, Captcha_object *co, int significant TSRMLS_DC)
{
    uint16_t h;
    uint8_t s, l, r, g, b;
    struct captcha_colordef_t *c;
    static const char hexdigits[] = "0123456789ABCDEF";

    if (significant) {
        if (CAPTCHA_ATTR(significant_characters_color)) {
            c = &colordefs[CAPTCHA_ATTR(significant_characters_color)];
        } else {
            return;
        }
    } else {
        if (CAPTCHA_ATTR(fake_characters_color)) {
            c = &colordefs[CAPTCHA_ATTR(fake_characters_color)];
        } else {
            return;
        }
    }
    h = captcha_rand_range(c->hmin, c->hmax TSRMLS_CC);
    s = captcha_rand_range(c->smin, c->smax TSRMLS_CC);
    l = captcha_rand_range(c->lmin, c->lmax TSRMLS_CC);
    hsl_to_rgb(h, s, l, &r, &g, &b);
    smart_str_append_static(ret, "color: #");
    smart_str_appendc(ret, hexdigits[r / 16]);
    smart_str_appendc(ret, hexdigits[r % 16]);
    smart_str_appendc(ret, hexdigits[g / 16]);
    smart_str_appendc(ret, hexdigits[g % 16]);
    smart_str_appendc(ret, hexdigits[b / 16]);
    smart_str_appendc(ret, hexdigits[b % 16]);
    smart_str_append_static(ret, "; ");
}

static void generate_char(smart_str *ret, Captcha_object *co, long index, char c, int significant TSRMLS_DC)
{
    long noise, p;
    const char *e;

//     smart_str_append_static(ret, "#captcha span:nth-child(");
    smart_str_appendc(ret, '#');
    smart_str_appendl(ret, co->html_wrapper_id, co->html_wrapper_id_len);
    smart_str_appendc(ret, ' ');
    smart_str_appendl(ret, co->html_letter_tag, co->html_letter_tag_len);
    smart_str_append_static(ret, ":nth-child(");
    if (captcha_rand(1 TSRMLS_CC)) {
        smart_str_append_static(ret, "0n+");
    }
    smart_str_append_long(ret, index + 1);
    smart_str_append_static(ret, "):after { content: \"");
    if (CAPTCHA_ATTR(noise_length)) {
        noise = captcha_rand(CAPTCHA_ATTR(noise_length) TSRMLS_CC);
        if (noise) {
            long l;

            for (l = 0; l < noise; l++) {
                e = ignorables[captcha_rand(ARRAY_SIZE(ignorables) - 1 TSRMLS_CC)];
                smart_str_appends(ret, e);
            }
        }
    }
    smart_str_appendc(ret, '\\');
    p = char2int(c);
    e = table[p].tbl[captcha_rand(table[p].length - 1 TSRMLS_CC)];
    smart_str_appends(ret, e);
    if (CAPTCHA_ATTR(noise_length)) {
        noise = captcha_rand(CAPTCHA_ATTR(noise_length) TSRMLS_CC);
        if (noise) {
            long l;

            for (l = 0; l < noise; l++) {
                e = ignorables[captcha_rand(ARRAY_SIZE(ignorables) - 1 TSRMLS_CC)];
                smart_str_appends(ret, e);
            }
        }
    }
    smart_str_append_static(ret, "\"; ");
    set_color(ret, co, significant TSRMLS_CC);
    if (significant) {
        smart_str_appends(ret, CAPTCHA_ATTR(significant_characters_style));
    } else {
        smart_str_appends(ret, CAPTCHA_ATTR(fake_characters_style));
    }
    smart_str_append_static(ret, " }\n");
}

#define UNINITIALIZED_CHAR 0x81
#define UNSIGNIFICANT_CHAR 0x82
PHP_FUNCTION(captcha_render)
{
    long i;
    zval *object = NULL;
    smart_str ret = { 0 };
    Captcha_object *co = NULL;
    long total_len, /*noise = 0,*/ what = CAPTCHA_RENDER_CSS | CAPTCHA_RENDER_HTML;

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O|l", &object, Captcha_ce_ptr, &what)) {
        RETURN_FALSE;
    }
    CAPTCHA_FETCH_OBJ(co, object);

    if (NULL == co->fakes) {
        total_len = Z_STRLEN_P(co->challenge);
    } else {
        total_len = Z_STRLEN_P(co->challenge) + zend_hash_num_elements(Z_ARRVAL_P(co->fakes));
    }
    if (what & CAPTCHA_RENDER_CSS) {
        unsigned char index[ARRAY_SIZE(shuffling)], map[ARRAY_SIZE(shuffling)];

        if (what & CAPTCHA_RENDER_HTML) {
            smart_str_append_static(&ret, "<style type=\"text/css\">\n");
        }
        memcpy(index, shuffling, total_len);
        php_string_shuffle(index, total_len TSRMLS_CC);
        if (NULL != co->fakes) {
            long j;
            char *str_index;
            ulong num_index;
            HashPosition pos;

            memset(map, UNINITIALIZED_CHAR, total_len);
            zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(co->fakes), &pos);
            while (HASH_KEY_IS_LONG == zend_hash_get_current_key_ex(Z_ARRVAL_P(co->fakes), &str_index, NULL, &num_index, 0, &pos)) {
                map[num_index] = UNSIGNIFICANT_CHAR;
                zend_hash_move_forward_ex(Z_ARRVAL_P(co->fakes), &pos);
            }
            for (i = j = 0; i < Z_STRLEN_P(co->challenge); j++) {
                if (UNINITIALIZED_CHAR == map[j]) {
                    map[j] = i++;
                }
            }
        } else {
            memcpy(map, shuffling, total_len);
        }
        for (i = 0; i < total_len; i++) {
            if (UNSIGNIFICANT_CHAR == map[index[i]]) {
                zval **zchar;

                if (SUCCESS == zend_hash_index_find(Z_ARRVAL_P(co->fakes), (ulong) index[i], (void **) &zchar)) {
                    generate_char(&ret, co, index[i], Z_STRVAL_PP(zchar)[0], 0 TSRMLS_CC);
                }
            } else {
                generate_char(&ret, co, index[i], Z_STRVAL_P(co->challenge)[map[index[i]]], 1 TSRMLS_CC);
            }
        }
        if (what & CAPTCHA_RENDER_HTML) {
            smart_str_append_static(&ret, "</style>\n");
        }
    }

    if (what & CAPTCHA_RENDER_HTML) {
//         smart_str_append_static(&ret, "<div id=\"captcha\">");
        smart_str_appendc(&ret, '<');
        smart_str_appendl(&ret, co->html_wrapper_tag, co->html_wrapper_tag_len);
        smart_str_append_static(&ret, " id=\"");
        smart_str_appendl(&ret, co->html_wrapper_id, co->html_wrapper_id_len);
        smart_str_append_static(&ret, "\">");
//         smart_str_append_static_repeated(&ret, total_len, "<span></span>");
        for (i = 0; i < total_len; i++) {
            smart_str_appendc(&ret, '<');
            smart_str_appendl(&ret, co->html_letter_tag, co->html_letter_tag_len);
            smart_str_append_static(&ret, "></");
            smart_str_appendl(&ret, co->html_letter_tag, co->html_letter_tag_len);
            smart_str_appendc(&ret, '>');
        }
//         smart_str_append_static(&ret, "</div>");
        smart_str_append_static(&ret, "</");
        smart_str_appendl(&ret, co->html_wrapper_tag, co->html_wrapper_tag_len);
        smart_str_appendc(&ret, '>');
    }

    if (0 == ret.len) {
        RETURN_FALSE;
    } else {
        smart_str_0(&ret);
        RETVAL_STRINGL(ret.c, ret.len, 0);
    }
}

PHP_FUNCTION(captcha_renew)
{
    zval *object;
    Captcha_object* co;

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, Captcha_ce_ptr)) {
        RETURN_FALSE;
    }
    CAPTCHA_FETCH_OBJ(co, object);
    captcha_fetch_or_create_challenge(co, 1 TSRMLS_CC);
}

PHP_FUNCTION(captcha_validate)
{
    zval *object;
    Captcha_object* co;
    char *input = NULL;
    long input_len = 0;

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os", &object, Captcha_ce_ptr, &input, &input_len)) {
        RETURN_FALSE;
    }
    CAPTCHA_FETCH_OBJ(co, object);
    ++Z_LVAL_P(co->attempts);
    if (input_len == Z_STRLEN_P(co->challenge) && 0 == zend_binary_strcasecmp(input, input_len, Z_STRVAL_P(co->challenge), Z_STRLEN_P(co->challenge))) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

PHP_FUNCTION(captcha_get_key)
{
    zval *object = NULL;
    Captcha_object *co = NULL;

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, Captcha_ce_ptr)) {
        RETURN_FALSE;
    }
    CAPTCHA_FETCH_OBJ(co, object);

    RETURN_STRINGL(co->key, co->key_len, 1);
}

PHP_FUNCTION(captcha_get_challenge)
{
    zval *object = NULL;
    Captcha_object *co = NULL;

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, Captcha_ce_ptr)) {
        RETURN_FALSE;
    }
    CAPTCHA_FETCH_OBJ(co, object);

    MAKE_COPY_ZVAL(&co->challenge, return_value);
}

PHP_FUNCTION(captcha_get_attempts)
{
    zval *object = NULL;
    Captcha_object *co = NULL;

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, Captcha_ce_ptr)) {
        RETURN_FALSE;
    }
    CAPTCHA_FETCH_OBJ(co, object);

    MAKE_COPY_ZVAL(&co->attempts, return_value);
}

PHP_FUNCTION(captcha_get_attribute)
{
    long attr;
    zval *object = NULL;
    Captcha_object *co = NULL;

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol", &object, Captcha_ce_ptr, &attr)) {
        RETURN_FALSE;
    }
    CAPTCHA_FETCH_OBJ(co, object);
    switch (attr) {
#define BOOL_CAPTCHA_ATTRIBUTE(member, name, defaultvalue) \
        case CAPTCHA_ATTR_PREFIX(name):
#define LONG_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb)
#define STRING_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb)
#include "captcha_attributes.h"
#undef BOOL_CAPTCHA_ATTRIBUTE
#undef LONG_CAPTCHA_ATTRIBUTE
#undef STRING_CAPTCHA_ATTRIBUTE
        {
            RETURN_BOOL(*((zend_bool *) (((char *) co) + attributes[attr].offset)));
        }
#define BOOL_CAPTCHA_ATTRIBUTE(member, name, defaultvalue)
#define LONG_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb) \
        case CAPTCHA_ATTR_PREFIX(name):
#define STRING_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb)
#include "captcha_attributes.h"
#undef BOOL_CAPTCHA_ATTRIBUTE
#undef LONG_CAPTCHA_ATTRIBUTE
#undef STRING_CAPTCHA_ATTRIBUTE
        {
            RETURN_LONG(*((long *) (((char *) co) + attributes[attr].offset)));
        }
#define BOOL_CAPTCHA_ATTRIBUTE(member, name, defaultvalue)
#define LONG_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb)
#define STRING_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb) \
        case CAPTCHA_ATTR_PREFIX(name):
#include "captcha_attributes.h"
#undef BOOL_CAPTCHA_ATTRIBUTE
#undef LONG_CAPTCHA_ATTRIBUTE
#undef STRING_CAPTCHA_ATTRIBUTE
        {
            RETURN_STRINGL(
                *((char **) (((char *) co) + attributes[attr].offset)),
                *((long *) (((char *) co) + attributes[attr].offset + sizeof(char *))),
                1 // duplicate
            );
        }
    }
    RETURN_NULL();
}

PHP_FUNCTION(captcha_set_attribute)
{
    long attr;
    zval *value = NULL;
    zval *object = NULL;
    Captcha_object *co = NULL;

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Olz", &object, Captcha_ce_ptr, &attr, &value)) {
        RETURN_FALSE;
    }
    CAPTCHA_FETCH_OBJ(co, object);

    RETURN_BOOL(captcha_set_attribute(co, attr, &value TSRMLS_CC));
}

static PHP_METHOD(Captcha, __wakeup)
{
    zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "You cannot serialize or unserialize %s instances", Captcha_ce_ptr->name);
}

static PHP_METHOD(Captcha, __sleep)
{
    zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "You cannot serialize or unserialize %s instances", Captcha_ce_ptr->name);
}

PHP_INI_BEGIN()
PHP_INI_END()

ZEND_BEGIN_ARG_INFO_EX(arginfo_captcha_void, 0, 0, 1)
    ZEND_ARG_INFO(0, captcha)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_captcha_0or1arg, 0, 0, 1)
    ZEND_ARG_INFO(0, captcha)
    ZEND_ARG_INFO(0, arg1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_captcha_1or2arg, 0, 0, 2)
    ZEND_ARG_INFO(0, captcha)
    ZEND_ARG_INFO(0, arg1)
    ZEND_ARG_INFO(0, arg2)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_captcha_1arg, 0, 0, 2)
    ZEND_ARG_INFO(0, captcha)
    ZEND_ARG_INFO(0, arg1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_captcha_2arg, 0, 0, 3)
    ZEND_ARG_INFO(0, captcha)
    ZEND_ARG_INFO(0, arg1)
    ZEND_ARG_INFO(0, arg2)
ZEND_END_ARG_INFO()

static const zend_function_entry captcha_functions[] = {
    PHP_FE(captcha_create, arginfo_captcha_1or2arg)
    PHP_FE(captcha_render, arginfo_captcha_0or1arg)
    PHP_FE(captcha_validate, arginfo_captcha_1arg)
    PHP_FE(captcha_renew, arginfo_captcha_void)
    PHP_FE(captcha_cleanup, arginfo_captcha_void)
    PHP_FE(captcha_get_key, arginfo_captcha_void)
    PHP_FE(captcha_get_challenge, arginfo_captcha_void)
    PHP_FE(captcha_get_attempts, arginfo_captcha_void)
    PHP_FE(captcha_get_attribute, arginfo_captcha_1arg)
    PHP_FE(captcha_set_attribute, arginfo_captcha_2arg)
    PHP_FE_END
};

static PHP_RINIT_FUNCTION(captcha)
{
    return SUCCESS;
}

static PHP_RSHUTDOWN_FUNCTION(captcha)
{
    return SUCCESS;
}

ZEND_BEGIN_ARG_INFO_EX(ainfo_captcha_void, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ainfo_captcha_0or1arg, 0, 0, 0)
    ZEND_ARG_INFO(0, arg1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ainfo_captcha_1or2arg, 0, 0, 1)
    ZEND_ARG_INFO(0, arg1)
    ZEND_ARG_INFO(0, arg2)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ainfo_captcha_1arg, 0, 0, 1)
    ZEND_ARG_INFO(0, arg1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ainfo_captcha_2arg, 0, 0, 2)
    ZEND_ARG_INFO(0, arg1)
    ZEND_ARG_INFO(0, arg1)
ZEND_END_ARG_INFO()

zend_function_entry Captcha_class_functions[] = {
    PHP_ME(Captcha, __sleep, ainfo_captcha_void, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(Captcha, __wakeup, ainfo_captcha_void, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(Captcha, __construct, ainfo_captcha_1or2arg, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME_MAPPING(create, captcha_create, ainfo_captcha_1or2arg, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME_MAPPING(render, captcha_render, ainfo_captcha_0or1arg, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(validate, captcha_validate, ainfo_captcha_1arg, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(renew, captcha_renew, ainfo_captcha_void, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(cleanup, captcha_cleanup, ainfo_captcha_void, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(getKey, captcha_get_key, ainfo_captcha_void, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(getChallenge, captcha_get_challenge, ainfo_captcha_void, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(getAttempts, captcha_get_attempts, ainfo_captcha_void, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(getAttribute, captcha_get_attribute, ainfo_captcha_1arg, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(setAttribute, captcha_set_attribute, ainfo_captcha_2arg, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

static void Captcha_objects_free(zend_object *object TSRMLS_DC)
{
    Captcha_object *co = (Captcha_object *) object;

    zend_object_std_dtor(&co->zo TSRMLS_CC);

#define BOOL_CAPTCHA_ATTRIBUTE(member, name, defaultvalue)
#define LONG_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb)
#define STRING_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb) \
        if (NULL != co->member) { \
            efree(co->member); \
        }
#include "captcha_attributes.h"
#undef BOOL_CAPTCHA_ATTRIBUTE
#undef LONG_CAPTCHA_ATTRIBUTE
#undef STRING_CAPTCHA_ATTRIBUTE
    if (NULL != co->key) {
        efree(co->key);
    }

    efree(co);
}

static zend_object_value Captcha_object_create(zend_class_entry *ce TSRMLS_DC)
{
    zend_object_value retval;
    Captcha_object *intern;

    intern = emalloc(sizeof(*intern));
    intern->key = NULL;
    memset(&intern->zo, 0, sizeof(zend_object));
    zend_object_std_init(&intern->zo, ce TSRMLS_CC);

    retval.handle = zend_objects_store_put(intern, NULL, (zend_objects_free_object_storage_t) Captcha_objects_free, NULL TSRMLS_CC);
    retval.handlers = &Captcha_handlers;

    return retval;
}

static PHP_MINIT_FUNCTION(captcha)
{
    size_t i;
    zend_class_entry ce;

    REGISTER_INI_ENTRIES();

    INIT_CLASS_ENTRY(ce, CAPTCHA_CLASS_NAME, Captcha_class_functions);
    ce.create_object = Captcha_object_create;
    Captcha_ce_ptr = zend_register_internal_class(&ce TSRMLS_CC);
    memcpy(&Captcha_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

    zend_declare_class_constant_long(Captcha_ce_ptr, "RENDER_CSS",  STR_LEN("RENDER_CSS"),  CAPTCHA_RENDER_CSS TSRMLS_CC);
    zend_declare_class_constant_long(Captcha_ce_ptr, "RENDER_HTML", STR_LEN("RENDER_HTML"), CAPTCHA_RENDER_HTML TSRMLS_CC);

#define CAPTCHA_COLOR(hminx, hmax, smin, smax, lmin, lmax, name) \
    zend_declare_class_constant_long(Captcha_ce_ptr, "COLOR_" #name, STR_LEN("COLOR_" #name), CAPTCHA_COLOR_PREFIX(name) TSRMLS_CC);
#include "captcha_colors.h"
#undef CAPTCHA_COLOR

// printf("%s %d\n", "ATTR_" #name, STR_LEN("ATTR_" #name));
#define BOOL_CAPTCHA_ATTRIBUTE(member, name, defaultvalue) \
    zend_declare_class_constant_long(Captcha_ce_ptr, "ATTR_" #name, STR_LEN("ATTR_" #name), CAPTCHA_ATTR_PREFIX(name) TSRMLS_CC);
# define LONG_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb) \
    zend_declare_class_constant_long(Captcha_ce_ptr, "ATTR_" #name, STR_LEN("ATTR_" #name), CAPTCHA_ATTR_PREFIX(name) TSRMLS_CC);
# define STRING_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb) \
    zend_declare_class_constant_long(Captcha_ce_ptr, "ATTR_" #name, STR_LEN("ATTR_" #name), CAPTCHA_ATTR_PREFIX(name) TSRMLS_CC);
# include "captcha_attributes.h"
# undef LONG_CAPTCHA_ATTRIBUTE
# undef STRING_CAPTCHA_ATTRIBUTE

    return SUCCESS;
}

static PHP_MSHUTDOWN_FUNCTION(captcha)
{
    UNREGISTER_INI_ENTRIES();

    return SUCCESS;
}

#define STRINGIFY(x) #x
#define STRINGIFY_EXPANDED(x) STRINGIFY(x)

static PHP_MINFO_FUNCTION(captcha)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "CSS Captcha", "enabled");
    php_info_print_table_row(2, "Alphabet", alphabet);
    php_info_print_table_row(2, "Unicode version", STRINGIFY_EXPANDED(CAPTCHA_UNICODE_MAJOR) "." STRINGIFY_EXPANDED(CAPTCHA_UNICODE_MINOR) "." STRINGIFY_EXPANDED(CAPTCHA_UNICODE_PATCH));
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}

static const zend_module_dep captcha_deps[] = {
    ZEND_MOD_REQUIRED("session")
    ZEND_MOD_END
};

zend_module_entry captcha_module_entry = {
    STANDARD_MODULE_HEADER_EX,
    NULL,
    captcha_deps,
    "captcha",
    captcha_functions,
    PHP_MINIT(captcha),
    PHP_MSHUTDOWN(captcha),
    PHP_RINIT(captcha),
    PHP_RSHUTDOWN(captcha),
    PHP_MINFO(captcha),
    NO_VERSION_YET,
    NO_MODULE_GLOBALS,
    NULL,
    STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_CAPTCHA
ZEND_GET_MODULE(captcha)
#endif
