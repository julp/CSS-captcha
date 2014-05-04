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
define('MAX_ATTEMPTS', 10);
define('KEY', pathinfo(__FILE__, PATHINFO_FILENAME));

// $colors = array('red', 'green', 'blue');
$options = array(
    CSSCaptcha::ATTR_FAKE_CHARACTERS_STYLE => 'color: red;',
    CSSCaptcha::ATTR_SIGNIFICANT_CHARACTERS_STYLE => 'color: blue;',
);

$captcha = new CSSCaptcha(KEY, $options);
if (isset($_POST['captcha'])) {
    if ($captcha->validate($_POST['captcha'])) {
        $captcha->renew();
        echo '<p>You pass, new token created.</p>';
    } else if ($captcha->getAttempts() >= MAX_ATTEMPTS) {
        $captcha->renew();
        echo '<p>Too many failures, new token created.</p>';
    } else {
        echo '<p>You fail.</p>';
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
                <p>Attempts: <?php echo $captcha->getAttempts(), ' / ', MAX_ATTEMPTS; ?></p>
            </fieldset>
            <input type="submit" value="Send"/>
        </form>
    </body>
</html>
