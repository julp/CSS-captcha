<?php
header('Content-Type: text/html; charset=utf-8');
session_start();
require('CSSCaptcha.php');
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
CSSCaptcha::$fake_character_style = 'color: red;';
// CSSCaptcha::$normal_character_style = 'color: blue;';

if (isset($_POST['captcha'])) {
    $captcha = new CSSCaptcha(KEY);
    if ($captcha->validate($_POST['captcha'])) {
        $captcha->renew();
        echo '<p>You pass, new token created.</p>';
    } else if ($captcha->getAttempts() >= MAX_ATTEMPTS) {
        $captcha->renew();
        echo '<p>Too many failures, new token created.</p>';
    } else {
        echo '<p>You fail.</p>';
    }
} else /*if (!isset($_SESSION['']))*/ {
    $captcha = new CSSCaptcha(KEY);
}
?>
        <form method="post" action="">
            <?php echo $captcha->render(); ?>
            <div style="clear: both;">
                Captcha : <input type="text" name="captcha"/> (only lower cased letter and digit)
            </div>
            <p>Expect: <?php var_dump($captcha->getChallenge()); ?></p>
            <p>Attempts: <?php echo $captcha->getAttempts(), ' / ', MAX_ATTEMPTS; ?></p>
            <input type="submit" value="Envoyer"/>
        </form>
    </body>
</html>
