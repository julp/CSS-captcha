--TEST--
Captcha: redefine CSSCaptcha::[ALWAYS|NEVER|RANDOM] valued attribute
--SKIPIF--
<?php
if (!extension_loaded('captcha')) { die('skip, captcha extension not available'); }
?>
--FILE--
<?php
session_id(md5(__FILE__));
@ session_start();

$captcha = new CSSCaptcha(pathinfo(__FILE__, PATHINFO_FILENAME), new CSSCaptchaSessionStore);

function set_reversed($captcha, $value)
{
    var_dump($value, $captcha->setAttribute(CSSCaptcha::ATTR_REVERSED, $value));
}

set_reversed($captcha, -1);
set_reversed($captcha, 1000);

set_reversed($captcha, NULL);

# control checks on boundaries (test [<>]=? in check_never_always_random_attribute)
$ok = TRUE;
foreach ([CSSCaptcha::ALWAYS, CSSCaptcha::NEVER, CSSCaptcha::RANDOM] as $v) {
    $ok &= $captcha->setAttribute(CSSCaptcha::ATTR_REVERSED, $v);
}
var_dump($ok);
var_dump($captcha->setAttribute(CSSCaptcha::ATTR_REVERSED, max(CSSCaptcha::ALWAYS, CSSCaptcha::NEVER, CSSCaptcha::RANDOM) + 1));
?>
--EXPECTF--

Warning: %s::%s(): Invalid value: -1, one of CAPTCHA_[NEVER|ALWAYS|RANDOM] expected in %s on line %d
int(-1)
bool(false)

Warning: %s::%s(): Invalid value: 1000, one of CAPTCHA_[NEVER|ALWAYS|RANDOM] expected in %s on line %d
int(1000)
bool(false)
NULL
bool(true)
int(1)

Warning: %s::%s(): Invalid value: %d, one of CAPTCHA_[NEVER|ALWAYS|RANDOM] expected in %s on line %d
bool(false)
