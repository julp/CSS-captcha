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
    </head>
    <body>
<?php
define('KEY', pathinfo(__FILE__, PATHINFO_FILENAME));

$options = array(
    CSSCaptcha::ATTR_FAKE_CHARACTERS_STYLE => '',
    CSSCaptcha::ATTR_FAKE_CHARACTERS_COLOR => CSSCaptcha::COLOR_RED,
    CSSCaptcha::ATTR_SIGNIFICANT_CHARACTERS_COLOR => CSSCaptcha::COLOR_BLUE,
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
