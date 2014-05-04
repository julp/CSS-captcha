#ifndef PHP_CAPTCHA_H

# define PHP_CAPTCHA_H 1

extern zend_module_entry captcha_module_entry;
# define phpext_captcha_ptr &captcha_module_entry

typedef struct {
    zend_object zo;

    char *key;
    int key_len;
    zval *container;
    zval *challenge;
    zval *attempts;
    zval *fakes;
# define LONG_CAPTCHA_ATTRIBUTE(member, name, cb) \
    long member;
# define STRING_CAPTCHA_ATTRIBUTE(member, name, cb) \
    char *member; \
    int *member##_len;
# include "captcha_attributes.h"
# undef LONG_CAPTCHA_ATTRIBUTE
# undef STRING_CAPTCHA_ATTRIBUTE
} Captcha_object;

extern zend_class_entry *Captcha_ce_ptr;

#endif /* !PHP_CAPTCHA_H */
