--TEST--
Captcha: redefine challenge length attribute
--SKIPIF--
<?php
if (!extension_loaded('captcha')) { die('skip, captcha extension not available'); }
?>
--FILE--
<?php
session_id(md5(__FILE__));
@ session_start();

$captcha = new CSSCaptcha(pathinfo(__FILE__, PATHINFO_FILENAME), new CSSCaptchaSessionStore);

function set_challenge_length($captcha, $value)
{
    var_dump($value, $captcha->setAttribute(CSSCaptcha::ATTR_CHALLENGE_LENGTH, $value));
}

set_challenge_length($captcha, '');
set_challenge_length($captcha, NULL);
set_challenge_length($captcha, 'x_');
set_challenge_length($captcha, 0);
set_challenge_length($captcha, 24);
set_challenge_length($captcha, 16);
?>
--EXPECTF--

Warning: %s::%s(): Challenge length must be in the range ]0;16] in %s on line %d
string(0) ""
bool(false)

Warning: %s::%s(): Challenge length must be in the range ]0;16] in %s on line %d
NULL
bool(false)

Warning: %s::%s(): Challenge length must be in the range ]0;16] in %s on line %d
string(2) "x_"
bool(false)

Warning: %s::%s(): Challenge length must be in the range ]0;16] in %s on line %d
int(0)
bool(false)

Warning: %s::%s(): Challenge length must be in the range ]0;16] in %s on line %d
int(24)
bool(false)
int(16)
bool(true)
