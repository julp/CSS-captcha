PHP_ARG_ENABLE(captcha, whether to enable PHP captcha,
[  --enable-captcha       Enable captcha support], no)

if test "$PHP_CAPTCHA" != "no"; then
  dnl PHP_PWRITE_TEST
  dnl PHP_PREAD_TEST
  PHP_NEW_EXTENSION(captcha, php_captcha.c, $ext_shared)
  PHP_ADD_EXTENSION_DEP(captcha, session, true)
  PHP_SUBST(CAPTCHA_SHARED_LIBADD)
  PHP_INSTALL_HEADERS(ext/captcha, [php_captcha.h])
  AC_DEFINE(HAVE_PHP_CAPTCHA,1,[ ])
fi
