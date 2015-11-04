--TEST--
Captcha: leaks on internal renew, destruction and string attribute
--SKIPIF--
<?php
if (!extension_loaded('captcha')) { die('skip, captcha extension not available'); }
?>
--FILE--
<?php
session_id(md5(__FILE__));
@ session_start();

$captcha = new CSSCaptcha(pathinfo(__FILE__, PATHINFO_FILENAME), new CSSCaptchaSessionStore, array(CSSCaptcha::ATTR_FAKE_CHARACTERS_STYLE => 'xxx'));
$captcha->validate('wrong');
$captcha->validate('wrong');
?>
--EXPECT--
