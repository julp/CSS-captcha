--TEST--
Captcha: redefine unicode version attribute
--SKIPIF--
<?php
if (!extension_loaded('captcha')) { die('skip, captcha extension not available'); }
?>
--FILE--
<?php
session_id(md5(__FILE__));
@ session_start();

$captcha = new CSSCaptcha(pathinfo(__FILE__, PATHINFO_FILENAME), new CSSCaptchaSessionStore);

function set_version($captcha, $value)
{
    var_dump($value, $captcha->setAttribute(CSSCaptcha::ATTR_UNICODE_VERSION, $value));
}

set_version($captcha, -1);
set_version($captcha, 1000);

set_version($captcha, NULL);
set_version($captcha, CSSCaptcha::UNICODE_LAST);
set_version($captcha, CSSCaptcha::UNICODE_LAST + 1);
?>
--EXPECTF--

Warning: %s::%s(): Invalid unicode version defined: -1 in %s on line %d
int(-1)
bool(false)

Warning: %s::%s(): Invalid unicode version defined: 1000 in %s on line %d
int(1000)
bool(false)
NULL
bool(true)
int(%d)
bool(true)

Warning: %s::%s(): Invalid unicode version defined: %d in %s on line %d
int(%d)
bool(false)
