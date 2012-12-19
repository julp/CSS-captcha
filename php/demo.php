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
if (isset($_POST['captcha'])) {
    $captcha = CSSCaptcha::createFromKey($_POST['captcha_key']);
    if ($captcha->validate($_POST['captcha'])) {
        echo '<p>You pass. New token created.</p>';
        $captcha = CSSCaptcha::create();
    } else {
        echo '<p>You fail.</p>';
    }
} else {
    $captcha = CSSCaptcha::create();
}
?>
        <form method="post" action="">
            <?php echo $captcha->render(); ?>
            <div>
                Captcha : <input type="text" name="captcha"/> (only lower cased letter and digit)
                <input type="hidden" name="captcha_key" value="<?php echo $captcha->getKey(); ?>"/>
            </div>
            <input type="submit" value="Envoyer"/>
        </form>
    </body>
</html>
