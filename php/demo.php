<?php
header('Content-Type: text/html; charset=utf-8');
session_start();
include('CSSCaptcha.php');
ini_set('display_errors', FALSE); // hide array_rand warning
?>
<!DOCTYPE html>
<html>
    <head>
        <meta charset="utf-8" />
    </head>
    <body>
<?php
define('KEY', 'demo');

if (isset($_POST['captcha'])) {
    $captcha = new CSSCaptcha(KEY);
    if ($captcha->validate($_POST['captcha'])) {
        $captcha->reset();
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
            <div>
                Captcha : <input type="text" name="captcha"/> (only lower cased letter and digit)
                <!--<input type="hidden" name="captcha_key" value="<?php echo $captcha->getKey(); ?>"/>-->
            </div>
            <input type="submit" value="Envoyer"/>
        </form>
    </body>
</html>
