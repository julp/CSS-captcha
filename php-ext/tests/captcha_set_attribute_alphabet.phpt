--TEST--
Captcha: redefine alphabet attribute
--SKIPIF--
<?php
if (!extension_loaded('captcha')) { die('skip, captcha extension not available'); }
?>
--FILE--
<?php
session_id(md5(__FILE__));
@ session_start();

$captcha = new CSSCaptcha(pathinfo(__FILE__, PATHINFO_FILENAME), new CSSCaptchaSessionStore, array(CSSCaptcha::ATTR_ALPHABET => 'xxx'));

function set_alphabet($captcha, $value)
{
    var_dump($value, $captcha->setAttribute(CSSCaptcha::ATTR_ALPHABET, $value));
}

set_alphabet($captcha, '');
set_alphabet($captcha, NULL);
set_alphabet($captcha, 'x_');

set_alphabet($captcha, 0);
set_alphabet($captcha, 'ab');
?>
--EXPECTF--

Warning: %s::%s(): Alphabet can't be empty in %s on line %d
string(0) ""
bool(false)

Warning: %s::%s(): Alphabet can't be empty in %s on line %d
NULL
bool(false)

Warning: %s::%s(): Alphabet can only contains characters from [0-9a-z] in %s on line %d
string(2) "x_"
bool(false)
int(0)
bool(true)
string(2) "ab"
bool(true)
