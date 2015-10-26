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
<!--<style stype="text/css">
body {font-family:"Arial Unicode MS","DejaVu Sans",sans-serif}
</style>-->
    </head>
    <body>
<?php
define('KEY', pathinfo(__FILE__, PATHINFO_FILENAME));

$captcha = new CSSCaptcha(KEY, new CSSCaptchaSessionStore);
if (isset($_POST['captcha'])) {
    if ($captcha->validate($_POST['captcha'])) {
        echo '<p>You pass, new token created.</p>';
    } else {
        echo '<p>You fail. Retry.</p>';
    }
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
