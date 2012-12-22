<?php
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
define('KEY', pathinfo(__FILE__, PATHINFO_FILENAME));

if (isset($_POST['captcha'])) {
    $captcha = new CSSCaptcha(KEY);
    if ($captcha->validate($_POST['captcha'])) {
        $captcha->renew();
        echo '<p>You pass. New token created.</p>';
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
            <input type="submit" value="Envoyer"/>
        </form>
    </body>
</html>
