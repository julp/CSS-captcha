--TEST--
Captcha: redefine color attribute
--SKIPIF--
<?php
if (!extension_loaded('captcha')) { die('skip, captcha extension not available'); }
?>
--FILE--
<?php
session_id(md5(__FILE__));
@ session_start();

$captcha = new CSSCaptcha(pathinfo(__FILE__, PATHINFO_FILENAME), new CSSCaptchaSessionStore);

function set_color($captcha, $value)
{
    var_dump($value, $captcha->setAttribute(CSSCaptcha::ATTR_FAKE_CHARACTERS_COLOR, $value));
}

set_color($captcha, -1);
set_color($captcha, 1000);

set_color($captcha, NULL);

# control checks on boundaries (test [<>]=? in check_color_attribute)
$max = 0;
$ok = TRUE;
$ro = new  ReflectionClass($captcha);
foreach ($ro->getConstants() as $n => $v) {
    if (0 == strncmp($n, 'COLOR_', strlen('COLOR_'))) {
        if ($v > $max) {
            $max = $v;
        }
        $ok &= $captcha->setAttribute(CSSCaptcha::ATTR_FAKE_CHARACTERS_COLOR, $v);
    }
}
var_dump($ok);
var_dump($captcha->setAttribute(CSSCaptcha::ATTR_FAKE_CHARACTERS_COLOR, $max + 1));
?>
--EXPECTF--

Warning: %s::%s(): Invalid color value: -1 in %s on line %d
int(-1)
bool(false)

Warning: %s::%s(): Invalid color value: 1000 in %s on line %d
int(1000)
bool(false)
NULL
bool(true)
int(1)

Warning: %s::%s(): Invalid color value: %d in %s on line %d
bool(false)
