--TEST--
Captcha: bad inheritance
--SKIPIF--
<?php
if (!extension_loaded('captcha')) { die('skip, captcha extension not available'); }
?>
--FILE--
<?php
session_id(md5(__FILE__));
@ session_start();

class CSSCaptchaBis extends CSSCaptcha {
    public function __construct($key) {
        // ommitting parent::__construct($key);
    }
}

$c = new CSSCaptchaBis('foo');
$c->render();
?>
--EXPECTF--
Warning: %s::%s(): Invalid or unitialized CSSCaptcha object in %s on line %d
