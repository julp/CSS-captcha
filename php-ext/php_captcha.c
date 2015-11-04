#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

/* Uncommnent the following line to enable, by default, confusable characters ('0', '1', 'i', 'l' and 'o') */
/*#define CAPTCHA_WITH_CONFUSABLE 1*/

#include <stdint.h>
#include "php.h"
#include "php_ini.h"
#include "php_captcha.h"
#include "ext/standard/php_rand.h"
// #if HAVE_PHP_SESSION && !defined(COMPILE_DL_SESSION)
# include "ext/session/php_session.h"
// #endif

#include "captcha_table.h"
#define IGNORABLE_INDEX (ARRAY_SIZE(offsets) - 1)

#define STRINGIFY(x) #x
#define STRINGIFY_EXPANDED(x) STRINGIFY(x)

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#define STR_LEN(str)      (ARRAY_SIZE(str) - 1)
#define STR_SIZE(str)     (ARRAY_SIZE(str))

enum {
    FETCH_NO_CHECK,
    FETCH_CHECK
};

enum {
    CAPTCHA_NEVER,
    CAPTCHA_ALWAYS,
    CAPTCHA_RANDOM
};

// PHP >= 7
#if PHP_MAJOR_VERSION >= 7

// class entry's name is a zend_string, not a char * anymore
# define CE_NAME(n) \
    ZSTR_VAL(n)
# define MAKE_STD_ZVAL(z) /* NOP */
// ... so Z_*_PP macros were removed and boolean type was split into 2 (true/false are 2 different types)
# define Z_BVAL_PP(z) (IS_TRUE == Z_TYPE(**(z)))
# define Z_LVAL_PP(z) Z_LVAL(**(z))
# define Z_STRVAL_PP(z) Z_STRVAL(**(z))
# define Z_STRLEN_PP(z) Z_STRLEN(**(z))
// declare a zend string vs char *n + long n_len
# define ZSTR_DECL(n) \
    zend_string *n
// zend_parse_* modifier for zend_string vs char * + long *
# define ZSTR_MODIFIER "S"
// zend_parse_* arguments for zend_string vs char * + long *
# define ZSTR_ARG(s) &s
// for key lookups in hashtable (since PHP 7, '\0' have to be "ignored")
# define S(s) s, STR_LEN(s)
// smart_str was renamed smart_string and smart_str is now a "wrapper" on zend_string
# include "zend_smart_str.h"
// a macro for portability with old RETURN_STRINGL where string is copied (ie 3rd argument was 1)
# define RETURN_STRINGL_COPY(string, length) \
    RETURN_STRINGL(string, length)
// a macro for portability to test if smart string is empty or not
# define EMPTY_SMART_STR(str) \
    ('\0' == (str).s)
// a macro for portability to test if a session is currently active
# define SESSION_IS_ACTIVE() \
    (Z_ISREF_P(&PS(http_session_vars)) && IS_ARRAY == Z_TYPE_P(Z_REFVAL(PS(http_session_vars))))
// fetch/initialize our object from zval
# define CAPTCHA_FETCH_OBJ_P(/*Captcha_object **/ co, /*zend_object **/ object) \
        co = (Captcha_object *)((char *) (object) - XtOffsetOf(Captcha_object, zo))
// NOTE: no need to have 2 different macros for with and without check, let the compiler optimize
# define CAPTCHA_FETCH_OBJ(/*Captcha_object **/ co, /*zval **/ object, /*int*/ check) \
    do { \
        CAPTCHA_FETCH_OBJ_P(co, Z_OBJ_P(object)); \
        if (check && NULL == co->key) { \
            php_error_docref(NULL, E_WARNING, "Invalid or unitialized %s object", CE_NAME(Captcha_ce_ptr->name)); \
            RETURN_FALSE; \
        } \
    } while (0);

#else /* PHP < 7 */

# define CE_NAME(n) \
    n
# define Z_TRY_ADDREF_P(z) \
    Z_ADDREF_P((z))
# define ZVAL_UNDEF(z) \
    (z) = NULL
# define Z_ISUNDEF(z) \
    (NULL == (z))
# define ZSTR_DECL(n) \
    char *n; \
    zend_strlen_t n##_len
# define ZSTR_MODIFIER "s"
# define ZSTR_ARG(s) &s, &s##_len
# define S(s) s, STR_SIZE(s)
# define ZEND_HASH_FOREACH_END()
# include "ext/standard/php_smart_str.h"
# define RETURN_STRINGL_COPY(string, length) \
    RETURN_STRINGL(string, length, 1)
# define EMPTY_SMART_STR(str) \
    ('\0' == (str).c)
# define SESSION_IS_ACTIVE() \
    (PS(http_session_vars) && IS_ARRAY == PS(http_session_vars)->type)
# define CAPTCHA_FETCH_OBJ(/*Captcha_object **/ co, /*zval **/ object, /*int*/ check)                              \
    do {                                                                                                           \
        co = (Captcha_object *) zend_object_store_get_object(object TSRMLS_CC);                                    \
        if (check && NULL == co->key) {                                                                            \
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid or unitialized %s object", Captcha_ce_ptr->name); \
            RETURN_FALSE;                                                                                          \
        }                                                                                                          \
    } while (0);

#endif /* PHP >= 7 */

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
static int check_unicode_version_attribute(zval * TSRMLS_DC);
static int check_challenge_length_attribute(zval * TSRMLS_DC);
static int check_zero_or_positive_attribute(zval * TSRMLS_DC);
static int check_never_always_random_attribute(zval * TSRMLS_DC);
static int check_fake_characters_length_attribute(zval * TSRMLS_DC);
static int check_non_empty_string_attribute(zval * TSRMLS_DC);
static int check_alphabet_content_attribute(zval * TSRMLS_DC);

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

zend_object_handlers Captcha_handlers;
zend_class_entry *Captcha_ce_ptr = NULL;
zend_class_entry *captcha_store_iface_entry = NULL;
zend_class_entry *captcha_session_store_entry = NULL;

static const char default_alphabet[] =
#ifdef CAPTCHA_WITH_CONFUSABLE
    "0123456789abcdefghijklmnopqrstuvwxyz"
#else
    "23456789abcdefghjkmnpqrstuvwxyz"
#endif /* CAPTCHA_WITH_CONFUSABLE */
;

enum {
#define UNICODE_FIRST(M, m, p) \
    UNICODE_FIRST = UNICODE_##M##_##m##_##p,
#define UNICODE_LAST(M, m, p) \
    UNICODE_LAST = UNICODE_##M##_##m##_##p,
#define UNICODE_VERSION(M, m, p) \
    UNICODE_##M##_##m##_##p,
#include "../gen/unicode_versions.h"
    _UNICODE_VERSIONS_COUNT
#undef UNICODE_FIRST
#undef UNICODE_LAST
#undef UNICODE_VERSION
};

static const char shuffling[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, /* reserved to challenge characters */
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F  /* reserved to fake characters */
};

#define MAX_CHALLENGE_LENGTH (ARRAY_SIZE(shuffling) / 2)

static void build_session_key(Captcha_object *co, zval *out)
{
    smart_str ret = { 0 };

    smart_str_appendl(&ret, CAPTCHA_ATTR(session_prefix), CAPTCHA_ATTR(session_prefix_len));
    smart_str_appendl(&ret, co->key, co->key_len);
    smart_str_0(&ret);
#if PHP_MAJOR_VERSION >= 7
    ZVAL_NEW_STR(out, ret.s);
#else
    ZVAL_STRINGL(out, ret.c, ret.len, 0);
#endif /* PHP >= 7 */
}

static void php_string_shuffle(char *str, int len TSRMLS_DC)
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

static long captcha_rand_range(long min, long max TSRMLS_DC)
{
    long rnd_idx;

    rnd_idx = php_rand(TSRMLS_C);
    RAND_RANGE(rnd_idx, min, max, PHP_RAND_MAX);

    return rnd_idx;
}

static long captcha_rand(long max TSRMLS_DC)
{
    return captcha_rand_range(0, max TSRMLS_CC);
}

static int random_string(const char *alphabet, zend_strlen_t alphabet_len, long length, zval *ZVALPX(dst) TSRMLS_DC)
{
    long i;
    char buffer[MAX_CHALLENGE_LENGTH];

    for (i = 0; i < length; i++) {
        buffer[i] = alphabet[captcha_rand(alphabet_len - 1 TSRMLS_CC)];
    }
    buffer[i] = '\0';
#if PHP_MAJOR_VERSION >= 7
    ZVAL_NEW_STR(dst, zend_string_init(buffer, length, 0));
#else
    ALLOC_INIT_ZVAL(*dst);
    ZVAL_STRINGL(*dst, buffer, length, 1);
#endif /* PHP >= 7 */

    return SUCCESS;
}

static int is_subset_of(zval *string, const char *set, zend_strlen_t set_len)
{
    zend_strlen_t i;
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

enum {
    DTOR_SKIP,
    DTOR_CALL
};

static int ps_call_handler(zval *func, int argc, zval *ZVALPX(argv), zval *ZVALPX(return_value), int call_dtor TSRMLS_DC)
{
    int i, ret;
    zval ZVALPX(retval), *ZVALPX(retval_p);

    if (NULL == return_value) {
        retval_p = &retval;
    } else {
        retval_p = return_value;
    }
    MAKE_STD_ZVAL(*retval_p);
    if (FAILURE == (ret = call_user_function(EG(function_table), NULL, func, ZVALPX(retval_p), argc, argv TSRMLS_CC))) {
        zval_ptr_dtor(retval_p);
#if PHP_MAJOR_VERSION >= 7
        ZVAL_UNDEF(retval_p);
    } else if (Z_ISUNDEF_P(retval_p)) {
        ZVAL_NULL(retval_p);
#else
        *retval_p = NULL;
#endif /* PHP >= 7 */
    }
    if (call_dtor) {
        for (i = 0; i < argc; i++) {
            zval_ptr_dtor(&argv[i]);
        }
    }
    if (NULL == return_value) {
        zval_ptr_dtor(retval_p);
    }

    return ret;
}

static void captcha_void(Captcha_object *co TSRMLS_DC)
{
//     Z_TRY_DELREF_P(ZVALRX(co->challenge));
    if (!Z_ISUNDEF(co->challenge)) {
        zval_ptr_dtor(&co->challenge);
    }
//     Z_TRY_DELREF_P(ZVALRX(co->fakes));
    if (!Z_ISUNDEF(co->fakes)) {
        zval_ptr_dtor(&co->fakes);
    }
}

static void captcha_fetch_or_create_challenge(Captcha_object *co, int renew TSRMLS_DC)
{
    zval ZVALPX(args[2]), ZVALPX(zcontainer), ZVALPX(*zchallenge), ZVALPX(*zattemps), ZVALPX(*zfakes);

    ZVAL_UNDEF(ZVALRX(zcontainer));
    MAKE_STD_ZVAL(args[0]);
    build_session_key(co, ZVALRX(args[0]));
    if (renew) {
        captcha_void(co TSRMLS_CC);
    }
    if (
        !renew
        && SUCCESS == ps_call_handler(ZVALRX(co->callback.cb_get), 1, args, &zcontainer, DTOR_SKIP TSRMLS_CC)
        && IS_ARRAY == Z_TYPE_P(ZVALRX(zcontainer))
#if PHP_MAJOR_VERSION >= 7
        && (NULL != (zchallenge = zend_hash_str_find(Z_ARRVAL_P(&zcontainer), S("challenge"))))
#else
        && SUCCESS == zend_hash_find(Z_ARRVAL_P(zcontainer), S("challenge"), (void **) &zchallenge)
#endif /* PHP >= 7 */
        && IS_STRING == Z_TYPE_P(ZVALPX(zchallenge))
        && is_subset_of(ZVALPX(zchallenge), CAPTCHA_ATTR(alphabet), CAPTCHA_ATTR(alphabet_len))
    ) {
#if PHP_MAJOR_VERSION >= 7
        ZVAL_COPY_VALUE(&co->challenge, zchallenge);
#else
        co->challenge = *zchallenge;
        zval_add_ref(zchallenge);
#endif /* PHP >= 7 */
        ZVAL_UNDEF(ZVALRX(co->fakes));
        if (
#if PHP_MAJOR_VERSION >= 7
            (NULL != (zfakes = zend_hash_str_find(Z_ARRVAL_P(&zcontainer), S("fakes"))))
#else
            SUCCESS == zend_hash_find(Z_ARRVAL_P(zcontainer), S("fakes"), (void **) &zfakes)
#endif /* PHP >= 7 */
            && IS_ARRAY == Z_TYPE_P(ZVALPX(zfakes))
            && zend_hash_num_elements(Z_ARRVAL_P(ZVALPX(zfakes))) > 0
        ) {
            // TODO: check keys/values
#if PHP_MAJOR_VERSION >= 7
            ZVAL_COPY(&co->fakes, zfakes);
#else
            co->fakes = *zfakes;
            zval_add_ref(zfakes);
//             ZVAL_COPY_VALUE(co->fakes, *zfakes);
//             zval_copy_ctor(co->fakes);
#endif /* PHP >= 7 */
        }
        zval_ptr_dtor(&zcontainer);
    } else {
        zend_strlen_t challenge_len;

        challenge_len = CAPTCHA_ATTR(challenge_length);
        random_string(CAPTCHA_ATTR(alphabet), CAPTCHA_ATTR(alphabet_len),challenge_len, &co->challenge TSRMLS_CC);
#if PHP_MAJOR_VERSION < 7
        if (NULL != zcontainer) {
            zval_ptr_dtor(&zcontainer);
        }
        MAKE_STD_ZVAL(zcontainer);
#endif /* PHP < 7 */
        ZVAL_UNDEF(ZVALRX(co->fakes));
        array_init(ZVALRX(zcontainer));
        Z_TRY_ADDREF_P(ZVALRX(co->challenge));
        add_assoc_zval_ex(ZVALRX(zcontainer), S("challenge"), ZVALRX(co->challenge));
        if (CAPTCHA_ATTR(fake_characters_length) > 0) {
            zend_strlen_t i;
            char index[MAX_CHALLENGE_LENGTH];

            memcpy(index, shuffling, challenge_len);
            php_string_shuffle(index, challenge_len TSRMLS_CC);
#if PHP_MAJOR_VERSION < 7
            MAKE_STD_ZVAL(co->fakes);
#endif /* PHP < 7 */
            array_init(ZVALRX(co->fakes));
            for (i = 0; i < CAPTCHA_ATTR(fake_characters_length); i++) {
                add_index_stringl(ZVALRX(co->fakes), index[i], CAPTCHA_ATTR(alphabet) + captcha_rand(CAPTCHA_ATTR(alphabet_len) - 1 TSRMLS_CC), 1
#if PHP_MAJOR_VERSION < 7
                , 1
#endif /* PHP < 7 */
                );
            }
            Z_TRY_ADDREF_P(ZVALRX(co->fakes));
            add_assoc_zval_ex(ZVALRX(zcontainer), S("fakes"), ZVALRX(co->fakes));
        }
        args[1] = zcontainer;
        Z_TRY_ADDREF_P(ZVALRX(zcontainer));
        ps_call_handler(ZVALRX(co->callback.cb_set), 2, args, NULL, DTOR_SKIP TSRMLS_CC);
        /**
         * PHP 5 : without = segfault
         * PHP 7 : with = leaks
         **/
#if PHP_MAJOR_VERSION >= 7
        zval_ptr_dtor(&args[1]);
#endif /* PHP < 7 */
    }
    zval_ptr_dtor(&args[0]); // we may use it twice (to call cb_get and cb_set) so don't do it earlier
}

static int check_never_always_random_attribute(zval *value TSRMLS_DC)
{
    int valid;

    // we can assume that value is a "long" due to anterior convert_to_long
    valid = Z_LVAL_P(value) >= 0 && Z_LVAL_P(value) <= CAPTCHA_RANDOM;
    if (!valid) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid value: %ld, one of CAPTCHA_[NEVER|ALWAYS|RANDOM] expected", Z_LVAL_P(value));
    }

    return valid;
}

static int check_unicode_version_attribute(zval *value TSRMLS_DC)
{
    int valid;

    // we can assume that value is a "long" due to anterior convert_to_long
    valid = Z_LVAL_P(value) >= UNICODE_FIRST && Z_LVAL_P(value) <= UNICODE_LAST;
    if (!valid) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid unicode version defined: %ld", Z_LVAL_P(value));
    }

    return valid;
}

static int check_color_attribute(zval *value TSRMLS_DC)
{
    int valid;

    // we can assume that value is a "long" due to anterior convert_to_long
    valid = Z_LVAL_P(value) >= 0 && Z_LVAL_P(value) < CAPTCHA_COLOR_PREFIX(COUNT);
    if (!valid) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid color value: %ld", Z_LVAL_P(value));
    }

    return valid;
}

static int check_challenge_length_attribute(zval *value TSRMLS_DC)
{
    int valid;

    // we can assume that value is a "long" due to anterior convert_to_long
    valid = Z_LVAL_P(value) > 0 && Z_LVAL_P(value) <= MAX_CHALLENGE_LENGTH;
    if (!valid) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Challenge length must be in the range ]0;%lu]", MAX_CHALLENGE_LENGTH);
    }

    return valid;
}

static int check_fake_characters_length_attribute(zval *value TSRMLS_DC)
{
    int valid;

    // we can assume that value is a "long" due to anterior convert_to_long
    valid = Z_LVAL_P(value) >= 0 && Z_LVAL_P(value) <= MAX_CHALLENGE_LENGTH;
    if (!valid) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Fake characters length must be in the range [0;%lu]", MAX_CHALLENGE_LENGTH);
    }

    return valid;
}

static int check_zero_or_positive_attribute(zval *value TSRMLS_DC)
{
    int valid;

    // we can assume that value is a "long" due to anterior convert_to_long
    valid = Z_LVAL_P(value) >= 0;
    if (!valid) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Noise length can't be negative");
    }

    return valid;
}

static int check_alphabet_content_attribute(zval *value TSRMLS_DC)
{
    if (Z_STRLEN_P(value) <= 0) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Alphabet can be empty");
        return 0;
    }
    if (!is_subset_of(value, "0123456789abcdefghijklmnopqrstuvwxyz", STR_LEN("0123456789abcdefghijklmnopqrstuvwxyz"))) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Alphabet can only contains characters from [0-9a-z]");
        return 0;
    }

    return 1;
}

static int check_non_empty_string_attribute(zval *value TSRMLS_DC)
{
    int valid;

    valid = Z_STRLEN_P(value) > 0;
    if (!valid) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Value for the given attribute can't be empty");
    }

    return valid;
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
            if (NULL == attributes[attribute].cb || attributes[attribute].cb(*value TSRMLS_CC)) {
                efree(*((char **) (((char *) co) + attributes[attribute].offset)));
                *((char **) (((char *) co) + attributes[attribute].offset)) = estrndup(Z_STRVAL_PP(value), Z_STRLEN_PP(value));
                *((zend_strlen_t *) (((char *) co) + attributes[attribute].offset + sizeof(char *))) = Z_STRLEN_PP(value);
                return 1;
            }
            break;
        }
        default:
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unknown attribute %lu", attribute);
    }

    return 0;
}

static void captcha_ctor(INTERNAL_FUNCTION_PARAMETERS)
{
    int i;
    char *name;
    zend_strlen_t name_len;
    char *key = NULL;
    zend_strlen_t key_len = 0;
    Captcha_object *co;
    zval *object, *serializer, *options = NULL;

    object = return_value;
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sO|a", &key, &key_len, &serializer, captcha_store_iface_entry, &options)) {
        zval_dtor(return_value);
        RETURN_NULL();
    }
    CAPTCHA_FETCH_OBJ(co, object, FETCH_NO_CHECK);
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
#if PHP_MAJOR_VERSION < 7
        char *str_key;
#endif /* PHP < 7 */
        ulong long_key;
        zval ZVALPX(*attr_value);

#if PHP_MAJOR_VERSION >= 7
        ZEND_HASH_FOREACH_NUM_KEY_VAL(Z_ARRVAL_P(options), long_key, attr_value) {
            captcha_set_attribute(co, long_key, &attr_value TSRMLS_CC);
        } ZEND_HASH_FOREACH_END();
#else
        zend_hash_internal_pointer_reset(Z_ARRVAL_P(options));
        while (SUCCESS == zend_hash_get_current_data(Z_ARRVAL_P(options), (void **) &attr_value) && HASH_KEY_IS_LONG == zend_hash_get_current_key(Z_ARRVAL_P(options), &str_key, &long_key, 0)) {
            captcha_set_attribute(co, long_key, attr_value TSRMLS_CC);
            zend_hash_move_forward(Z_ARRVAL_P(options));
        }
#endif /* PHP >= 7 */
    }
    if (0 == key_len) {
        zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "Key must not be empty");
        zval_dtor(return_value);
        RETURN_NULL();
    } else {
        co->key = estrdup(key);
        co->key_len = key_len;
    }
    i = 0;
#if PHP_MAJOR_VERSION >= 7
    {
        zend_string *func_name;
        zend_function *current_mptr;

        ZEND_HASH_FOREACH_STR_KEY(&captcha_store_iface_entry->function_table, func_name) {
            if (NULL == (current_mptr = zend_hash_find_ptr(&Z_OBJCE_P(serializer)->function_table, func_name))) {
                // TODO: cleaner
                php_error_docref(NULL, E_ERROR, "Session handler's function table is corrupt");
                RETURN_FALSE;
            } else {
                array_init_size(&co->callbacks[i], 2);
                Z_ADDREF_P(serializer);
                add_next_index_zval(&co->callbacks[i], serializer);
                add_next_index_str(&co->callbacks[i], zend_string_copy(func_name));
            }
            ++i;
        } ZEND_HASH_FOREACH_END();
    }
#else
    {
        char *func_name;
        ulong func_index;
        HashPosition pos;
        zend_uint func_name_len;
        zend_function *default_mptr, *current_mptr;

        zend_hash_internal_pointer_reset_ex(&captcha_store_iface_entry->function_table, &pos);
        while (SUCCESS == zend_hash_get_current_data_ex(&captcha_store_iface_entry->function_table, (void **) &default_mptr, &pos)) {
            zend_hash_get_current_key_ex(&captcha_store_iface_entry->function_table, &func_name, &func_name_len, &func_index, 0, &pos);
            if (SUCCESS == zend_hash_find(&Z_OBJCE_P(serializer)->function_table, func_name, func_name_len, (void **) &current_mptr)) {
                MAKE_STD_ZVAL(co->callbacks[i]);
                array_init_size(co->callbacks[i], 2);
                Z_ADDREF_P(serializer);
                add_next_index_zval(co->callbacks[i], serializer);
                add_next_index_stringl(co->callbacks[i], func_name, func_name_len - 1, 1);
            } else {
                php_error_docref(NULL TSRMLS_CC, E_ERROR, "Session handler's function table is corrupt");
                RETURN_FALSE;
            }
            zend_hash_move_forward_ex(&captcha_store_iface_entry->function_table, &pos);
            ++i;
        }
    }
#endif /* PHP >= 7 */
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
    zval *object = NULL;
    zval ZVALPX(args[1]);
    Captcha_object *co = NULL;

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, Captcha_ce_ptr)) {
        RETURN_FALSE;
    }
    MAKE_STD_ZVAL(args[0]);
    build_session_key(co, ZVALRX(args[0]));
    ps_call_handler(ZVALRX(co->callback.cb_remove), 1, args, NULL, DTOR_SKIP TSRMLS_CC);
    zval_ptr_dtor(&args[0]);
}

#define smart_str_append_hexa(/*smart_str **/ self, /*uint32_t*/ n)                                  \
    do {                                                                                             \
        int written;                                                                                 \
        char buffer[16];                                                                             \
        written = snprintf(buffer, ARRAY_SIZE(buffer), n > 0xffff ? "%06" PRIX32 : "%04" PRIX32, n); \
        smart_str_appendl(self, buffer, written);                                                    \
    } while (0);

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

static void generate_char(smart_str *ret, Captcha_object *co, long index, char c, int significant, int reversed TSRMLS_DC)
{
    uint32_t e;
    long noise, p;

//     smart_str_append_static(ret, "#captcha span:nth-child(");
    if (reversed) {
        smart_str_appendc(ret, '#');
        smart_str_appendl(ret, co->html_wrapper_id, co->html_wrapper_id_len);
        smart_str_appendc(ret, ' ');
        smart_str_appendl(ret, co->html_letter_tag, co->html_letter_tag_len);
        smart_str_append_static(ret, ":nth-child(");
        if (captcha_rand(1 TSRMLS_CC)) {
            smart_str_append_static(ret, "0n+");
        }
        smart_str_append_long(ret, index + 1);
        smart_str_append_static(ret, ") { order: ");
        smart_str_append_long(ret, Z_STRLEN_P(ZVALRX(co->challenge)) - index);
        smart_str_append_static(ret, "; }\n");
    }
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
    /* <TODO:DRY> */
    if (CAPTCHA_ATTR(noise_length)) {
        noise = captcha_rand(CAPTCHA_ATTR(noise_length) TSRMLS_CC);
        if (noise) {
            long l;

            for (l = 0; l < noise; l++) {
                e = table[captcha_rand_range(offsets[IGNORABLE_INDEX][0], offsets[IGNORABLE_INDEX][CAPTCHA_ATTR(unicode_version) + 1] - 1 TSRMLS_CC)];
                smart_str_appendc(ret, '\\');
                smart_str_append_hexa(ret, e);
            }
        }
    }
    /* </TODO:DRY> */
    smart_str_appendc(ret, '\\');
    p = char2int(c);
    e = table[captcha_rand_range(offsets[p][0], offsets[p][CAPTCHA_ATTR(unicode_version) + 1] - 1 TSRMLS_CC)];
    smart_str_append_hexa(ret, e);
    /* <TODO:DRY> */
    if (CAPTCHA_ATTR(noise_length)) {
        noise = captcha_rand(CAPTCHA_ATTR(noise_length) TSRMLS_CC);
        if (noise) {
            long l;

            for (l = 0; l < noise; l++) {
                e = table[captcha_rand_range(offsets[IGNORABLE_INDEX][0], offsets[IGNORABLE_INDEX][CAPTCHA_ATTR(unicode_version) + 1] - 1 TSRMLS_CC)];
                smart_str_appendc(ret, '\\');
                smart_str_append_hexa(ret, e);
            }
        }
    }
    /* </TODO:DRY> */
    smart_str_append_static(ret, "\"; ");
    set_color(ret, co, significant TSRMLS_CC);
    if (significant) {
        smart_str_appends(ret, CAPTCHA_ATTR(significant_characters_style));
    } else {
        smart_str_appends(ret, CAPTCHA_ATTR(fake_characters_style));
    }
    smart_str_append_static(ret, " }\n");
}

enum {
    UNINITIALIZED_CHAR = 0x81,
    UNSIGNIFICANT_CHAR = 0x82
};

PHP_FUNCTION(captcha_render)
{
    zval *object = NULL;
    smart_str ret = { 0 };
    zend_strlen_t i, total_len;
    Captcha_object *co = NULL;
    long what = CAPTCHA_RENDER_CSS | CAPTCHA_RENDER_HTML;

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O|l", &object, Captcha_ce_ptr, &what)) {
        RETURN_FALSE;
    }
    CAPTCHA_FETCH_OBJ(co, object, FETCH_CHECK);
    if (Z_ISUNDEF(co->fakes)) {
        total_len = Z_STRLEN_P(ZVALRX(co->challenge));
    } else {
        total_len = Z_STRLEN_P(ZVALRX(co->challenge)) + zend_hash_num_elements(Z_ARRVAL_P(ZVALRX(co->fakes)));
    }
    if (what & CAPTCHA_RENDER_CSS) {
        int reversed;
        unsigned char index[ARRAY_SIZE(shuffling)], map[ARRAY_SIZE(shuffling)];

        reversed = CAPTCHA_ALWAYS == CAPTCHA_ATTR(reversed) || (CAPTCHA_RANDOM == CAPTCHA_ATTR(reversed) && captcha_rand(1 TSRMLS_CC));
        if (what & CAPTCHA_RENDER_HTML) {
            smart_str_append_static(&ret, "<style type=\"text/css\">\n");
        }
        if (reversed) {
            smart_str_appendc(&ret, '#');
            smart_str_appendl(&ret, co->html_wrapper_id, co->html_wrapper_id_len);
            smart_str_append_static(&ret, " { display: flex; flex-direction: row-reverse; }\n");
        }
        memcpy(index, shuffling, total_len);
        php_string_shuffle(index, total_len TSRMLS_CC);
        if (!Z_ISUNDEF(co->fakes)) {
            long j;
            ulong num_index;
#if PHP_MAJOR_VERSION < 7
            char *str_index;
            HashPosition pos;
#endif /* PHP < 7 */

            memset(map, UNINITIALIZED_CHAR, total_len);
#if PHP_MAJOR_VERSION >= 7
            ZEND_HASH_FOREACH_NUM_KEY(Z_ARRVAL(co->fakes), num_index) {
#else
            zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(co->fakes), &pos);
            while (HASH_KEY_IS_LONG == zend_hash_get_current_key_ex(Z_ARRVAL_P(co->fakes), &str_index, NULL, &num_index, 0, &pos)) {
#endif /* PHP >= 7 */
                map[num_index] = UNSIGNIFICANT_CHAR;
#if PHP_MAJOR_VERSION < 7
                zend_hash_move_forward_ex(Z_ARRVAL_P(co->fakes), &pos);
#endif /* PHP < 7 */
            } ZEND_HASH_FOREACH_END();
            for (i = j = 0; i < Z_STRLEN_P(ZVALRX(co->challenge)); j++) {
                if (UNINITIALIZED_CHAR == map[j]) {
                    map[j] = i++;
                }
            }
        } else {
            memcpy(map, shuffling, total_len);
        }
        for (i = 0; i < total_len; i++) {
            if (UNSIGNIFICANT_CHAR == map[index[i]]) {
                zval ZVALPX(*zchar);

#if PHP_MAJOR_VERSION >= 7
                if (NULL != (zchar = zend_hash_index_find(Z_ARRVAL(co->fakes), (zend_ulong) index[i]))) {
                    generate_char(&ret, co, index[i], Z_STRVAL_P(zchar)[0], 0, reversed TSRMLS_CC);
#else
                if (SUCCESS == zend_hash_index_find(Z_ARRVAL_P(co->fakes), (ulong) index[i], (void **) &zchar)) {
                    generate_char(&ret, co, index[i], Z_STRVAL_PP(zchar)[0], 0, reversed TSRMLS_CC);
#endif /* PHP >= 7 */
                }
            } else {
                generate_char(&ret, co, index[i], Z_STRVAL_P(ZVALRX(co->challenge))[map[index[i]]], 1, reversed TSRMLS_CC);
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
    smart_str_0(&ret);
    if (EMPTY_SMART_STR(ret)) {
        RETURN_FALSE;
    } else {
#if PHP_MAJOR_VERSION >= 7
        ZVAL_NEW_STR(return_value, ret.s);
#else
        RETVAL_STRINGL(ret.c, ret.len, 0);
#endif /* PHP >= 7 */
    }
}

PHP_FUNCTION(captcha_validate)
{
    zval *object;
    Captcha_object* co;
    char *input = NULL;
    zend_strlen_t input_len = 0;

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os", &object, Captcha_ce_ptr, &input, &input_len)) {
        RETURN_FALSE;
    }
    CAPTCHA_FETCH_OBJ(co, object, FETCH_CHECK);
    RETVAL_BOOL(input_len == Z_STRLEN_P(ZVALRX(co->challenge)) && 0 == zend_binary_strcasecmp(input, input_len, Z_STRVAL_P(ZVALRX(co->challenge)), Z_STRLEN_P(ZVALRX(co->challenge))));
    captcha_fetch_or_create_challenge(co, 1 TSRMLS_CC);
}

PHP_FUNCTION(captcha_get_key)
{
    zval *object = NULL;
    Captcha_object *co = NULL;

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, Captcha_ce_ptr)) {
        RETURN_FALSE;
    }
    CAPTCHA_FETCH_OBJ(co, object, FETCH_CHECK);

    RETURN_STRINGL_COPY(co->key, co->key_len);
}

PHP_FUNCTION(captcha_get_challenge)
{
    zval *object = NULL;
    Captcha_object *co = NULL;

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, Captcha_ce_ptr)) {
        RETURN_FALSE;
    }
    CAPTCHA_FETCH_OBJ(co, object, FETCH_CHECK);
#if PHP_MAJOR_VERSION >= 7
    if (Z_ISUNDEF(co->challenge)) {
        RETVAL_NULL();
    } else {
        ZVAL_COPY(return_value, &co->challenge);
    }
#else
    MAKE_COPY_ZVAL(&co->challenge, return_value);
#endif /* PHP >= 7 */
}

PHP_FUNCTION(captcha_get_attribute)
{
    long attr;
    zval *object = NULL;
    Captcha_object *co = NULL;

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol", &object, Captcha_ce_ptr, &attr)) {
        RETURN_FALSE;
    }
    CAPTCHA_FETCH_OBJ(co, object, FETCH_CHECK);
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
            RETURN_STRINGL_COPY(
                *((char **) (((char *) co) + attributes[attr].offset)),
                *((zend_strlen_t *) (((char *) co) + attributes[attr].offset + sizeof(char *)))
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
    CAPTCHA_FETCH_OBJ(co, object, FETCH_CHECK);

    RETURN_BOOL(captcha_set_attribute(co, attr, &value TSRMLS_CC));
}

static PHP_METHOD(Captcha, __wakeup)
{
    zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "You cannot serialize or unserialize %s instances", CE_NAME(Captcha_ce_ptr->name));
}

static PHP_METHOD(Captcha, __sleep)
{
    zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "You cannot serialize or unserialize %s instances", CE_NAME(Captcha_ce_ptr->name));
}

static PHP_METHOD(CSSCaptchaSessionStore, get)
{
    ZSTR_DECL(key);
    zval *ZVALPX(zret);

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, ZSTR_MODIFIER, ZSTR_ARG(key))) {
        RETURN_FALSE;
    }
    RETVAL_NULL();
    if (SESSION_IS_ACTIVE()) {
#if PHP_MAJOR_VERSION >= 7
        if (NULL != (zret = zend_symtable_find(Z_ARRVAL_P(Z_REFVAL(PS(http_session_vars))), key))) {
            ZVAL_DUP(return_value, zret);
#else
        if (SUCCESS == zend_symtable_find(Z_ARRVAL_P(PS(http_session_vars)), key, key_len + 1, (void **) &zret)) {
            RETVAL_ZVAL(*zret, 1 /* copy */, 0 /* dtor */);
#endif /* PHP >= 7 */
//         ZVAL_COPY(return_value, zret);
//         Z_TRY_ADDREF_P(return_value);
        }
    } else {
        // TODO: throw Exception ?
    }
}

static PHP_METHOD(CSSCaptchaSessionStore, set)
{
    ZSTR_DECL(key);
    zval *val = NULL;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, ZSTR_MODIFIER "z", ZSTR_ARG(key), &val)) {
        RETURN_FALSE;
    }
    if (SESSION_IS_ACTIVE()) {
#if PHP_MAJOR_VERSION >= 7
        zend_hash_update(Z_ARRVAL_P(Z_REFVAL(PS(http_session_vars))), key, val);
#else
        ZEND_SET_SYMBOL_WITH_LENGTH(Z_ARRVAL_P(PS(http_session_vars)), key, key_len + 1, val, 2, 0);
#endif /* PHP >= 7 */
    } else {
        // TODO: throw Exception ?
        RETURN_FALSE;
    }
}

static PHP_METHOD(CSSCaptchaSessionStore, remove)
{
    ZSTR_DECL(key);

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, ZSTR_MODIFIER, ZSTR_ARG(key))) {
        RETURN_FALSE;
    }
    if (SESSION_IS_ACTIVE()) {
        RETURN_BOOL(SUCCESS ==
#if PHP_MAJOR_VERSION >= 7
            zend_hash_del(Z_ARRVAL_P(Z_REFVAL(PS(http_session_vars))), key)
#else
            zend_hash_del(Z_ARRVAL_P(PS(http_session_vars)), key, key_len + 1)
#endif /* PHP >= 7 */
        );
    } else {
        // TODO: throw Exception ?
        RETURN_FALSE;
    }
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_captcha_2or3arg, 0, 0, 3)
    ZEND_ARG_INFO(0, captcha)
    ZEND_ARG_INFO(0, arg1)
    ZEND_ARG_INFO(0, arg2)
    ZEND_ARG_INFO(0, arg3)
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
    PHP_FE(captcha_create, arginfo_captcha_2or3arg)
    PHP_FE(captcha_render, arginfo_captcha_0or1arg)
    PHP_FE(captcha_validate, arginfo_captcha_1arg)
    PHP_FE(captcha_cleanup, arginfo_captcha_void)
    PHP_FE(captcha_get_key, arginfo_captcha_void)
    PHP_FE(captcha_get_challenge, arginfo_captcha_void)
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

ZEND_BEGIN_ARG_INFO_EX(ainfo_captcha_2or3arg, 0, 0, 2)
    ZEND_ARG_INFO(0, arg1)
    ZEND_ARG_INFO(0, arg2)
    ZEND_ARG_INFO(0, arg3)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ainfo_captcha_1arg, 0, 0, 1)
    ZEND_ARG_INFO(0, arg1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ainfo_captcha_2arg, 0, 0, 2)
    ZEND_ARG_INFO(0, arg1)
    ZEND_ARG_INFO(0, arg2)
ZEND_END_ARG_INFO()

zend_function_entry Captcha_class_functions[] = {
    PHP_ME(Captcha, __sleep, ainfo_captcha_void, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(Captcha, __wakeup, ainfo_captcha_void, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(Captcha, __construct, ainfo_captcha_2or3arg, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME_MAPPING(create, captcha_create, ainfo_captcha_2or3arg, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME_MAPPING(render, captcha_render, ainfo_captcha_0or1arg, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(validate, captcha_validate, ainfo_captcha_1arg, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(cleanup, captcha_cleanup, ainfo_captcha_void, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(getKey, captcha_get_key, ainfo_captcha_void, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(getChallenge, captcha_get_challenge, ainfo_captcha_void, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(getAttribute, captcha_get_attribute, ainfo_captcha_1arg, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(setAttribute, captcha_set_attribute, ainfo_captcha_2arg, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

static const zend_function_entry captcha_store_iface_functions[] = {
    PHP_ABSTRACT_ME(CSSCaptchaStoreInterface, get, ainfo_captcha_1arg)
    PHP_ABSTRACT_ME(CSSCaptchaStoreInterface, set, ainfo_captcha_2arg)
    PHP_ABSTRACT_ME(CSSCaptchaStoreInterface, remove, ainfo_captcha_1arg)
    { NULL, NULL, NULL }
};

static const zend_function_entry captcha_session_store_functions[] = {
    PHP_ME(CSSCaptchaSessionStore, get, ainfo_captcha_1arg, ZEND_ACC_PUBLIC)
    PHP_ME(CSSCaptchaSessionStore, set, ainfo_captcha_2arg, ZEND_ACC_PUBLIC)
    PHP_ME(CSSCaptchaSessionStore, remove, ainfo_captcha_1arg, ZEND_ACC_PUBLIC)
    { NULL, NULL, NULL }
};

static void Captcha_objects_free(zend_object *object TSRMLS_DC)
{
    int i;
    Captcha_object *co;

    i = 0;
#if PHP_MAJOR_VERSION >= 7
    CAPTCHA_FETCH_OBJ_P(co, object);
#else
    co = (Captcha_object *) object;
#endif /* PHP >= 7 */

    captcha_void(co TSRMLS_CC);
#define BOOL_CAPTCHA_ATTRIBUTE(member, name, defaultvalue)
#define LONG_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb)
#define STRING_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb) \
        if (NULL != CAPTCHA_ATTR(member)) { \
            efree(CAPTCHA_ATTR(member)); \
        }
#include "captcha_attributes.h"
#undef BOOL_CAPTCHA_ATTRIBUTE
#undef LONG_CAPTCHA_ATTRIBUTE
#undef STRING_CAPTCHA_ATTRIBUTE
    if (NULL != co->key) {
        efree(co->key);
    }
    for (i = 0; i < ARRAY_SIZE(co->callbacks); i++) {
        if (!Z_ISUNDEF(co->callbacks[i])) {
            zval_ptr_dtor(&co->callbacks[i]);
        }
    }
    zend_object_std_dtor(&co->zo TSRMLS_CC);

#if PHP_MAJOR_VERSION < 7
    efree(co);
#endif /* PHP < 7 */
}

static
#if PHP_MAJOR_VERSION >= 7
zend_object *
#else
zend_object_value
#endif /* PHP >= 7 */
Captcha_object_create(zend_class_entry *ce TSRMLS_DC)
{
    int i;
    Captcha_object *co;

#if PHP_MAJOR_VERSION >= 7
    co = ecalloc(1, sizeof(*co) + zend_object_properties_size(ce));
#else
    zend_object_value retval;

    co = emalloc(sizeof(*co));
    memset(&co->zo, 0, sizeof(zend_object));
#endif /* PHP >= 7 */
    zend_object_std_init(&co->zo, ce TSRMLS_CC);

    co->key = NULL;

    ZVAL_UNDEF(ZVALRX(co->fakes));
    ZVAL_UNDEF(ZVALRX(co->challenge));

    for (i = 0; i < ARRAY_SIZE(co->callbacks); i++) {
        ZVAL_UNDEF(ZVALRX(co->callbacks[i]));
    }

#define BOOL_CAPTCHA_ATTRIBUTE(member, name, defaultvalue) \
    co->member = defaultvalue;
#define LONG_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb) \
    co->member = defaultvalue;
#define STRING_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb) \
    co->member = NULL; \
    co->member##_len = 0;
#include "captcha_attributes.h"
#undef BOOL_CAPTCHA_ATTRIBUTE
#undef LONG_CAPTCHA_ATTRIBUTE
#undef STRING_CAPTCHA_ATTRIBUTE

#if PHP_MAJOR_VERSION >= 7
    co->zo.handlers
#else
    retval.handle = zend_objects_store_put(co, NULL, (zend_objects_free_object_storage_t) Captcha_objects_free, NULL TSRMLS_CC);
    retval.handlers
#endif /* PHP >= 7 */
        = &Captcha_handlers;

    return
#if PHP_MAJOR_VERSION >= 7
        &co->zo
#else
        retval
#endif /* PHP >= 7 */
    ;
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
#if PHP_MAJOR_VERSION >= 7
    Captcha_handlers.offset = XtOffsetOf(Captcha_object, zo);
    Captcha_handlers.free_obj = Captcha_objects_free;
#endif /* PHP >= 7 */

    zend_declare_class_constant_long(Captcha_ce_ptr, "RENDER_CSS",  STR_LEN("RENDER_CSS"),  CAPTCHA_RENDER_CSS TSRMLS_CC);
    zend_declare_class_constant_long(Captcha_ce_ptr, "RENDER_HTML", STR_LEN("RENDER_HTML"), CAPTCHA_RENDER_HTML TSRMLS_CC);

#define UNICODE_FIRST(M, m, p) \
    zend_declare_class_constant_long(Captcha_ce_ptr, "UNICODE_FIRST",  STR_LEN("UNICODE_FIRST"), UNICODE_FIRST TSRMLS_CC);
#define UNICODE_LAST(M, m, p) \
    zend_declare_class_constant_long(Captcha_ce_ptr, "UNICODE_LAST",  STR_LEN("UNICODE_LAST"), UNICODE_LAST TSRMLS_CC);
#define UNICODE_VERSION(M, m, p) \
    zend_declare_class_constant_long(Captcha_ce_ptr, "UNICODE_" #M "_" #m "_" #p,  STR_LEN("UNICODE_" #M "_" #m "_" #p), UNICODE_##M##_##m##_##p TSRMLS_CC);
#include "../gen/unicode_versions.h"
#undef UNICODE_FIRST
#undef UNICODE_LAST
#undef UNICODE_VERSION

    zend_declare_class_constant_long(Captcha_ce_ptr, "NEVER", STR_LEN("NEVER"), CAPTCHA_NEVER TSRMLS_CC);
    zend_declare_class_constant_long(Captcha_ce_ptr, "ALWAYS", STR_LEN("ALWAYS"), CAPTCHA_ALWAYS TSRMLS_CC);
    zend_declare_class_constant_long(Captcha_ce_ptr, "RANDOM", STR_LEN("RANDOM"), CAPTCHA_RANDOM TSRMLS_CC);

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

    INIT_CLASS_ENTRY(ce, "CSSCaptchaStoreInterface", captcha_store_iface_functions);
    captcha_store_iface_entry = zend_register_internal_class(&ce TSRMLS_CC);
    captcha_store_iface_entry->ce_flags |= ZEND_ACC_INTERFACE;

    INIT_CLASS_ENTRY(ce, "CSSCaptchaSessionStore", captcha_session_store_functions);
    captcha_session_store_entry = zend_register_internal_class(&ce TSRMLS_CC);
    // Declare it final for now while this is not a real class
    captcha_session_store_entry->ce_flags |= ZEND_ACC_FINAL;
    zend_class_implements(captcha_session_store_entry TSRMLS_CC, 1, captcha_store_iface_entry);

    return SUCCESS;
}

static PHP_MSHUTDOWN_FUNCTION(captcha)
{
    UNREGISTER_INI_ENTRIES();

    return SUCCESS;
}

static PHP_MINFO_FUNCTION(captcha)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "CSS Captcha", "enabled");
    php_info_print_table_row(2, "Default alphabet", default_alphabet);
#define UNICODE_FIRST(M, m, p) \
    php_info_print_table_row(2, "Minimum supported unicode version", #M "." #m "." #p);
#define UNICODE_LAST(M, m, p) \
    php_info_print_table_row(2, "Maximum supported unicode version", #M "." #m "." #p);
#define UNICODE_VERSION(M, m, p) /* NOP */
#include "../gen/unicode_versions.h"
#undef UNICODE_FIRST
#undef UNICODE_LAST
#undef UNICODE_VERSION
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
