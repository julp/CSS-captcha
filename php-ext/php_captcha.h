#ifndef PHP_CAPTCHA_H

# define PHP_CAPTCHA_H 1

extern zend_module_entry captcha_module_entry;
# define phpext_captcha_ptr &captcha_module_entry

# if PHP_MAJOR_VERSION >= 7
// add one level of indirection depending on version
#  define ZVALPX(z) z
// its "opposite"
#  define ZVALRX(z) &z
typedef size_t zend_strlen_t;
# else
#  define ZVALPX(z) *z
#  define ZVALRX(z) z
typedef int zend_strlen_t;
# endif /* PHP >= 7 */

typedef struct {
# if PHP_MAJOR_VERSION < 7
    zend_object zo;
# endif /* PHP < 7 */

    union {
        zval ZVALPX(callbacks[3]);
        struct {
            zval ZVALPX(cb_get);
            zval ZVALPX(cb_set);
            zval ZVALPX(cb_remove);
        } callback;
    };

    char *key;
    zend_strlen_t key_len;
    zval ZVALPX(challenge);
    zval ZVALPX(fakes);
# define BOOL_CAPTCHA_ATTRIBUTE(member, name, defaultvalue) \
    zend_bool member;
# define LONG_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb) \
    long member;
# define STRING_CAPTCHA_ATTRIBUTE(member, name, defaultvalue, cb) \
    char *member; \
    zend_strlen_t member##_len;
# include "captcha_attributes.h"
# undef BOOL_CAPTCHA_ATTRIBUTE
# undef LONG_CAPTCHA_ATTRIBUTE
# undef STRING_CAPTCHA_ATTRIBUTE

# if PHP_MAJOR_VERSION >= 7
    zend_object zo;
# endif /* PHP >= 7 */
} Captcha_object;

extern zend_class_entry *Captcha_ce_ptr;

#endif /* !PHP_CAPTCHA_H */
