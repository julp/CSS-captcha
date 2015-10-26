<?php
header('Content-Type: text/html; charset=utf-8');
session_start();
if (!extension_loaded('captcha')) {
    require(dirname(__FILE__) . '/../php-plain/CSSCaptcha.php');
}
?>
<!DOCTYPE html>
<html>
    <head>
        <meta charset="utf-8" />
        <style type="text/css">
ul#foo {
    padding: 0;
    list-style-type: none;
}

ul#foo li {
    display: inline-block;
}
        </style>
    </head>
    <body>
<?php
define('KEY', pathinfo(__FILE__, PATHINFO_FILENAME));

// $colors = array('red', 'green', 'blue');
$options = array(
//     CSSCaptcha::ATTR_ALPHABET => implode(range('0', '9')),
//     CSSCaptcha::ATTR_UNICODE_VERSION => CSSCaptcha::UNICODE_3_0_0,
    CSSCaptcha::ATTR_HTML_WRAPPER_ID => 'foo',
    CSSCaptcha::ATTR_HTML_WRAPPER_TAG => 'ul',
    CSSCaptcha::ATTR_HTML_LETTER_TAG => 'li',
    CSSCaptcha::ATTR_FAKE_CHARACTERS_STYLE => 'color: red;',
    CSSCaptcha::ATTR_SIGNIFICANT_CHARACTERS_STYLE => 'color: blue;',
);

$captcha = new CSSCaptcha(KEY, new CSSCaptchaSessionStore, $options);
if (isset($_POST['captcha'])) {
    if ($captcha->validate($_POST['captcha'])) {
        echo '<p>You pass, new token created.</p>';
    } else {
        echo '<p>You fail. Retry.</p>';
    }
}
?>
        <form method="post" action="">
            <fieldset>
                <legend>Security code</legend>
                <?php echo $captcha->render(); ?>
                <div style="clear: both;">
                    <p>Retype (only) blue letters in the field below:</p>
                    <input type="text" name="captcha"/>
                    <p><small>(only lower cased letter and digit)</small></p>
                </div>
            </fieldset>
            <fieldset>
                <legend>Debug: internal states</legend>
                <p>Expect: <?php var_dump($captcha->getChallenge()); ?></p>
            </fieldset>
            <input type="submit" value="Send"/>
        </form>
    </body>
</html>
