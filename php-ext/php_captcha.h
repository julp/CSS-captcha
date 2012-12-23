#ifndef PHP_CAPTCHA_H

# define PHP_CAPTCHA_H 1

extern zend_module_entry captcha_module_entry;
#define phpext_captcha_ptr &captcha_module_entry

typedef struct {
    zend_object zo;

    char *key;
    long key_len;
    zval *challenge;
} Captcha_object;

extern zend_class_entry *Captcha_ce_ptr;

# ifdef ZTS
#  include "TSRM.h"
# endif /* ZTS */

ZEND_BEGIN_MODULE_GLOBALS(captcha)
    long challenge_length;
    long fake_characters;
    char *session_prefix;
ZEND_END_MODULE_GLOBALS(captcha)

/* Macro to access request-wide global variables. */
# ifdef ZTS
#  define CAPTCHA_G(v) TSRMG(captcha_globals_id, zend_captcha_globals *, v)
# else
#  define CAPTCHA_G(v) (captcha_globals.v)
# endif /* ZTS */

// PHP_FUNCTION(collator_create);
// PHP_METHOD(Collator, __construct);

#endif /* !PHP_CAPTCHA_H */