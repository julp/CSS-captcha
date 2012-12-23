#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "php.h"
#include "php_ini.h"
#include "php_captcha.h"
#include "ext/standard/php_rand.h"
#include "ext/session/php_session.h"
#include "ext/standard/php_smart_str.h"

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#define STR_LEN(str)      (ARRAY_SIZE(str) - 1)
#define STR_SIZE(str)     (ARRAY_SIZE(str))

#define CAPTCHA_CSS  (1 << 0)
#define CAPTCHA_HTML (1 << 1)

zend_class_entry *Captcha_ce_ptr = NULL;
zend_object_handlers Captcha_handlers;

#if 0
static const char alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyz";
#else
static const char alphabet[] = "0123456789a";
#endif

static const char *table_0[] = {
    "2070", "2080", "24EA", "FF10", "01D7CE", "01D7D8", "01D7E2", "01D7EC", "01D7F6"
};

static const char *table_1[] = {
    "00B9", "2081", "2460", "FF11", "01D7CF", "01D7D9", "01D7E3", "01D7ED", "01D7F7"
};

static const char *table_2[] = {
    "00B2", "2082", "2461", "FF12", "01D7D0", "01D7DA", "01D7E4", "01D7EE", "01D7F8"
};

static const char *table_3[] = {
    "00B3", "2083", "2462", "FF13", "01D7D1", "01D7DB", "01D7E5", "01D7EF", "01D7F9"
};

static const char *table_4[] = {
    "2074", "2084", "2463", "FF14", "01D7D2", "01D7DC", "01D7E6", "01D7F0", "01D7FA"
};

static const char *table_5[] = {
    "2075", "2085", "2464", "FF15", "01D7D3", "01D7DD", "01D7E7", "01D7F1", "01D7FB"
};

static const char *table_6[] = {
    "2076", "2086", "2465", "FF16", "01D7D4", "01D7DE", "01D7E8", "01D7F2", "01D7FC"
};

static const char *table_7[] = {
    "2077", "2087", "2466", "FF17", "01D7D5", "01D7DF", "01D7E9", "01D7F3", "01D7FD"
};

static const char *table_8[] = {
    "2078", "2088", "2467", "FF18", "01D7D6", "01D7E0", "01D7EA", "01D7F4", "01D7FE"
};

static const char *table_9[] = {
    "2079", "2089", "2468", "FF19", "01D7D7", "01D7E1", "01D7EB", "01D7F5", "01D7FF"
};

static const char *table_a[] = {
    "00AA", "00C0", "00C1", "00C2", "00C3", "00C4", "00C5",
    "00E0", "00E1", "00E2", "00E3", "00E4", "00E5", "0100",
    "0101", "0102", "0103", "0104", "0105", "01CD", "01CE",
    "01DE", "01DF", "01E0", "01E1", "01FA", "01FB", "0200",
    "0201", "0202", "0203", "0226", "0227", "1D2C", "1D43",
    "1E00", "1E01", "1EA0", "1EA1", "1EA2", "1EA3", "1EA4",
    "1EA5", "1EA6", "1EA7", "1EA8", "1EA9", "1EAA", "1EAB",
    "1EAC", "1EAD", "1EAE", "1EAF", "1EB0", "1EB1", "1EB2",
    "1EB3", "1EB4", "1EB5", "1EB6", "1EB7", "2090", "212B",
    "24B6", "24D0", "FF21", "FF41", "01D400", "01D41A",
    "01D434", "01D44E", "01D468", "01D482", "01D49C", "01D4B6",
    "01D4D0", "01D4EA", "01D504", "01D51E", "01D538", "01D552",
    "01D56C", "01D586", "01D5A0", "01D5BA", "01D5D4", "01D5EE",
    "01D608", "01D622", "01D63C", "01D656", "01D670", "01D68A",
    "01F130"
};

struct table_component {
    const char **tbl;
    size_t length;
} table[] = {
    { table_0, ARRAY_SIZE(table_0) },
    { table_1, ARRAY_SIZE(table_1) },
    { table_2, ARRAY_SIZE(table_2) },
    { table_3, ARRAY_SIZE(table_3) },
    { table_4, ARRAY_SIZE(table_4) },
    { table_5, ARRAY_SIZE(table_5) },
    { table_6, ARRAY_SIZE(table_6) },
    { table_7, ARRAY_SIZE(table_7) },
    { table_8, ARRAY_SIZE(table_8) },
    { table_9, ARRAY_SIZE(table_9) },
    { table_a, ARRAY_SIZE(table_a) }
};

static const char *ignorables[] = {
    "\\0009", "\\000A", "\\000C", "\\000D", "\\0020", "\\00A0",
    "\\1680", "\\180E", "\\2000", "\\2001", "\\2002", "\\2003",
    "\\2004", "\\2005", "\\2006", "\\2007", "\\2008", "\\2009",
    "\\200A", "\\2028", "\\2029", "\\202F", "\\205F", "\\3000",
};

static const char shuffling[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

#define MAX_CHALLENGE_LENGTH (ARRAY_SIZE(shuffling))

#define GENERATE_CHALLENGE(/*char **/ challenge, /*long*/ challenge_len) \
    do {                                                                 \
        challenge_len = CAPTCHA_G(challenge_length);                     \
        challenge = random_string(challenge_len TSRMLS_CC);              \
    } while (0);

#define COMPLETE_SESSION_KEY(/*Captcha_object **/ co, /*char **/ name, /*long*/ name_len) \
    do {                                                                                  \
        name_len = strlen(CAPTCHA_G(session_prefix)) + co->key_len;                       \
        name = emalloc(name_len + 1);                                                     \
        strcpy(name, CAPTCHA_G(session_prefix));                                          \
        strcat(name, co->key);                                                            \
    } while (0);

ZEND_DECLARE_MODULE_GLOBALS(captcha)

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

static void captcha_fetch_or_create_challenge(Captcha_object* co TSRMLS_DC)
{
    char *name;
    size_t name_len;
    zval **vpp;

    COMPLETE_SESSION_KEY(co, name, name_len);
//     if (SUCCESS == php_get_session_var(name, name_len, &vpp TSRMLS_CC) && IS_STRING == Z_TYPE_PP(vpp)) {
    if (SUCCESS == zend_symtable_find(Z_ARRVAL_P(PS(http_session_vars)), name, name_len + 1, (void **) &vpp) && IS_STRING == Z_TYPE_PP(vpp)) {
        co->challenge = *vpp;
    } else {
        long challenge_len;
        const char *challenge;

        GENERATE_CHALLENGE(challenge, challenge_len);
//         ZVAL_STRINGL(&v, co->challenge, co->challenge_len, 0);
//         php_set_session_var(name, name_len, &v, NULL TSRMLS_CC);
//         php_add_session_var(name, name_len TSRMLS_CC);
            ALLOC_INIT_ZVAL(co->challenge);
            ZVAL_STRINGL(co->challenge, challenge, challenge_len, 0);
            ZEND_SET_SYMBOL_WITH_LENGTH(Z_ARRVAL_P(PS(http_session_vars)), name, name_len + 1, co->challenge, 1, 0);
//         zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "");
    }
    efree(name);
}

static void captcha_ctor(INTERNAL_FUNCTION_PARAMETERS)
{
    zval *object;
    Captcha_object* co;
    char *key = NULL;
    long key_len = 0;
    char *name;
    size_t name_len;

    object = return_value;
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len)) {
        zval_dtor(return_value);
        RETURN_NULL();
    }
    co = (Captcha_object *) zend_object_store_get_object(object TSRMLS_CC);
    if (key_len == 0) {
        zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "Key must not be empty");
        zval_dtor(return_value);
        RETURN_NULL();
    } else {
        co->key = estrdup(key);
        co->key_len = key_len;
    }
    captcha_fetch_or_create_challenge(co TSRMLS_CC);
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

PHP_FUNCTION(captcha_render)
{
    zval *object = NULL;
    smart_str ret = { 0 };
    Captcha_object *co = NULL;
    long what = CAPTCHA_CSS | CAPTCHA_HTML;

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, Captcha_ce_ptr, &what)) {
        RETURN_FALSE;
    }
    co = (Captcha_object *) zend_object_store_get_object(object TSRMLS_CC);
    if (NULL == co) {
        RETURN_FALSE;
    }

    if (what & CAPTCHA_CSS) {
        long i, p;
        char index[MAX_CHALLENGE_LENGTH];

        smart_str_append_static(&ret, "<style type=\"text/css\">\n");
        memcpy(index, shuffling, Z_STRLEN_P(co->challenge));
        php_string_shuffle(index, Z_STRLEN_P(co->challenge) TSRMLS_CC);
        for (i = 0; i < Z_STRLEN_P(co->challenge); i++) {
            const char *e;

            smart_str_append_static(&ret, "#captcha span:nth-child(");
            smart_str_append_long(&ret, index[i] + 1);
            smart_str_append_static(&ret, "):after { content: \"\\");
            p = char2int(Z_STRVAL_P(co->challenge)[index[i]]);
            e = table[p].tbl[captcha_rand(table[p].length - 1 TSRMLS_CC)];
            smart_str_appends(&ret, e);
            smart_str_append_static(&ret, "\"; }\n");
        }
        smart_str_append_static(&ret, "</style>\n");
    }

    if (what & CAPTCHA_HTML) {
        smart_str_append_static(&ret, "<div id=\"captcha\">");
        smart_str_append_static_repeated(&ret, Z_STRLEN_P(co->challenge), "<span></span>");
        smart_str_append_static(&ret, "</div>");
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
    char *input = NULL;
    long input_len = 0;
    long challenge_len;
    const char *challenge;

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, Captcha_ce_ptr)) {
        RETURN_FALSE;
    }
    co = (Captcha_object *) zend_object_store_get_object(object TSRMLS_CC);
    if (NULL == co) {
        RETURN_FALSE;
    }
    captcha_fetch_or_create_challenge(co TSRMLS_CC);
    GENERATE_CHALLENGE(challenge, challenge_len);
    zval_dtor(co->challenge);
    ZVAL_STRINGL(co->challenge, challenge, challenge_len, 0);
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
    co = (Captcha_object *) zend_object_store_get_object(object TSRMLS_CC);
    if (NULL == co) {
        RETURN_FALSE;
    }
    if (input_len == Z_STRLEN_P(co->challenge) && 0 == memcmp(input, Z_STRVAL_P(co->challenge), Z_STRLEN_P(co->challenge))) {
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
    co = (Captcha_object *) zend_object_store_get_object(object TSRMLS_CC);
    if (NULL == co) {
        RETURN_FALSE;
    }

    RETURN_STRINGL(co->key, co->key_len, 1);
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
    co = (Captcha_object *) zend_object_store_get_object(object TSRMLS_CC);
    if (NULL == co) {
        RETURN_FALSE;
    }
    COMPLETE_SESSION_KEY(co, name, name_len);
    if (SUCCESS == zend_hash_del(Z_ARRVAL_P(PS(http_session_vars)), name, name_len + 1)) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

PHP_FUNCTION(captcha_get_challenge)
{
    zval *object = NULL;
    Captcha_object *co = NULL;

    if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, Captcha_ce_ptr)) {
        RETURN_FALSE;
    }
    co = (Captcha_object *) zend_object_store_get_object(object TSRMLS_CC);
    if (NULL == co) {
        RETURN_FALSE;
    }

    MAKE_COPY_ZVAL(&co->challenge, return_value);
}

static PHP_METHOD(Captcha, __wakeup)
{
    zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "You cannot serialize or unserialize %s instances", Captcha_ce_ptr->name);
}

static PHP_METHOD(Captcha, __sleep)
{
    zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "You cannot serialize or unserialize %s instances", Captcha_ce_ptr->name);
}

ZEND_API ZEND_INI_MH(OnUpdateChallengeLength)
{
    long *p, tmp;
#ifndef ZTS
    char *base = (char *) mh_arg2;
#else
    char *base;

    base = (char *) ts_resource(*((int *) mh_arg2));
#endif /* !ZTS */

    tmp = zend_atol(new_value, new_value_length);
    if (tmp < 0) {
        return FAILURE;
    }
    if (tmp > MAX_CHALLENGE_LENGTH) {
        int err_type;

        if (stage == ZEND_INI_STAGE_RUNTIME) {
            err_type = E_WARNING;
        } else {
            err_type = E_ERROR;
        }
        php_error_docref(NULL TSRMLS_CC, err_type, "Challenge cannot exceed %lu", MAX_CHALLENGE_LENGTH);
        return FAILURE;
    }

    p = (long *) (base+(size_t) mh_arg1);
    *p = tmp;

    return SUCCESS;
}

PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("captcha.challenge_length", "8", PHP_INI_ALL, OnUpdateChallengeLength, challenge_length, zend_captcha_globals, captcha_globals)
    STD_PHP_INI_ENTRY("captcha.fake_characters", "2", PHP_INI_ALL, OnUpdateLong, fake_characters, zend_captcha_globals, captcha_globals)
    STD_PHP_INI_ENTRY("captcha.session_prefix", "captcha_", PHP_INI_ALL, OnUpdateStringUnempty, session_prefix, zend_captcha_globals, captcha_globals)
PHP_INI_END()

ZEND_BEGIN_ARG_INFO_EX(arginfo_captcha_void, 0, 0, 1)
    ZEND_ARG_INFO(0, captcha)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_captcha_0or1arg, 0, 0, 1)
    ZEND_ARG_INFO(0, captcha)
    ZEND_ARG_INFO(0, arg1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_captcha_1arg, 0, 0, 2)
    ZEND_ARG_INFO(0, captcha)
    ZEND_ARG_INFO(0, arg1)
ZEND_END_ARG_INFO()

static const zend_function_entry captcha_functions[] = {
    PHP_FE(captcha_create, arginfo_captcha_1arg)
    PHP_FE(captcha_render, arginfo_captcha_0or1arg)
    PHP_FE(captcha_validate, arginfo_captcha_1arg)
    PHP_FE(captcha_renew, arginfo_captcha_void)
    PHP_FE(captcha_cleanup, arginfo_captcha_void)
    PHP_FE(captcha_get_key, arginfo_captcha_void)
    PHP_FE(captcha_get_challenge, arginfo_captcha_void)
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

ZEND_BEGIN_ARG_INFO_EX(ainfo_captcha_1arg, 0, 0, 1)
    ZEND_ARG_INFO(0, arg1)
ZEND_END_ARG_INFO()

zend_function_entry Captcha_class_functions[] = {
    PHP_ME(Captcha, __sleep, ainfo_captcha_void, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(Captcha, __wakeup, ainfo_captcha_void, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    PHP_ME(Captcha, __construct, ainfo_captcha_1arg, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME_MAPPING(create, captcha_create, ainfo_captcha_1arg, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME_MAPPING(render, captcha_render, ainfo_captcha_0or1arg, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(validate, captcha_validate, ainfo_captcha_1arg, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(renew, captcha_renew, ainfo_captcha_void, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(cleanup, captcha_cleanup, ainfo_captcha_void, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(getKey, captcha_get_key, ainfo_captcha_void, ZEND_ACC_PUBLIC)
    PHP_ME_MAPPING(getChallenge, captcha_get_challenge, ainfo_captcha_void, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

static void Captcha_objects_free(zend_object *object TSRMLS_DC)
{
    Captcha_object *co = (Captcha_object *) object;

    zend_object_std_dtor(&co->zo TSRMLS_CC);

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
    zend_class_entry ce;

    REGISTER_INI_ENTRIES();

    INIT_CLASS_ENTRY(ce, "CSSCaptcha", Captcha_class_functions);
    ce.create_object = Captcha_object_create;
    Captcha_ce_ptr = zend_register_internal_class(&ce TSRMLS_CC);
    memcpy(&Captcha_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

    zend_declare_class_constant_long(Captcha_ce_ptr, "CSS",  STR_LEN("CSS"),  CAPTCHA_CSS TSRMLS_CC);
    zend_declare_class_constant_long(Captcha_ce_ptr, "HTML", STR_LEN("HTML"), CAPTCHA_HTML TSRMLS_CC);

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
    php_info_print_table_header(2, "CSS Captcha", "enabled");
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}

#if 0
static PHP_GINIT_FUNCTION(captcha)
{
}
#endif

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
    PHP_MODULE_GLOBALS(captcha),
//     PHP_GINIT(captcha),
    NULL,
    NULL,
    NULL,
    STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_CAPTCHA
ZEND_GET_MODULE(captcha)
#endif
