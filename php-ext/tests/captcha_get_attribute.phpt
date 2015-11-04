--TEST--
Captcha: get attribute
--SKIPIF--
<?php
if (!extension_loaded('captcha')) { die('skip, captcha extension not available'); }
?>
--FILE--
<?php
session_id(md5(__FILE__));
@ session_start();

$attr = CSSCaptcha::ATTR_FAKE_CHARACTERS_STYLE;
$oldvalue = 'color: white';
$newvalue = 'color: black';
$captcha = new CSSCaptcha('foo', new CSSCaptchaSessionStore, array($attr => $oldvalue));
var_dump($captcha->getAttribute($attr));
var_dump($captcha->setAttribute($attr, $newvalue));
unset($newvalue);
var_dump($captcha->getAttribute($attr));
?>
--EXPECTF--
string(12) "color: white"
bool(true)
string(12) "color: black"
