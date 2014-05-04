<?php
// ini_set('captcha.fake_characters_style', '');
// ini_set('captcha.fake_characters_color', CSSCaptcha::LIGHT);
// ini_set('captcha.significant_characters_color', CSSCaptcha::BLUE);

header('Content-Type: text/html; charset=utf-8');
session_start();
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

if (isset($_POST['captcha'])) {
    $captcha = new CSSCaptcha(KEY);
    $catpcha->setAttribute(CSSCaptcha::FAKE_CHARACTERS_STYLE, '');
    $catpcha->setAttribute(CSSCaptcha::FAKE_CHARACTERS_COLOR, CSSCaptcha::LIGHT);
    $catpcha->setAttribute(CSSCaptcha::SIGNIFICANT_CHARACTERS_COLOR, CSSCaptcha::BLUE);
    if ($captcha->validate($_POST['captcha'])) {
        $captcha->renew();
        echo '<p>You pass. New token created.</p>';
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
                Captcha : <input type="text" name="captcha"/> (enter only blue characters)
            </div>
            <p>Expect: <?php var_dump($captcha->getChallenge()); ?></p>
            <p>Attempts: <?php var_dump($captcha->getAttempts()); ?></p>
            <input type="submit" value="Envoyer"/>
        </form>
    </body>
</html>
