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
#captcha {
    padding: 4px 20px;
    display: inline-block;
}

input[type="text"], input[type="password"] {
    height: 23px;
}

input[type="text"], input[type="password"], textarea, #captcha {
    border: 1px solid #AAAAAA;
}
        </style>
        <script type="text/javascript">
window.onload = function() {
    var captcha = document.getElementById('captcha');
    if (!captcha) {
        return;
    }
    var s = '';
    var l = document.querySelectorAll('#captcha > span');
    for (var i = 0; i < l.length; i++) {
        s += getComputedStyle(l[i], ':after').content.replace(/[^a-z\d]/gi, '');
    }
    var input = document.querySelector('input[name="captcha"]');
    if (!input) {
        return;
    }
    input.setAttribute('value', s);
    captcha.style.display = 'none';
}
        </script>
    </head>
    <body>
<?php
define('KEY', pathinfo(__FILE__, PATHINFO_FILENAME));

$options = array(
    CSSCaptcha::ATTR_FAKE_CHARACTERS_LENGTH => 0,
    CSSCaptcha::ATTR_SKIP_UNICODE_FOR_CHALLENGE => TRUE,
);
if (defined('CSSCaptcha::ATTR_ONLY_LTR')) {
    $options[CSSCaptcha::ATTR_ONLY_LTR] = TRUE;
}

$captcha = new CSSCaptcha(KEY/*, new CSSCaptchaSessionWriter*/, $options);
if (isset($_POST['captcha'])) {
    if ($captcha->validate($_POST['captcha'])) {
        echo '<p>You pass.</p>';
    } else {
        echo '<p>You fail.</p>';
    }
}
?>
        <form method="post" action="">
            <fieldset id="captcha">
                <legend>Security code</legend>
                <?php echo $captcha->render(); ?>
                <!--<span> -&gt; </span>
                <input type="text" name="captcha"/>-->
                <div style="clear: both;">
                    <p>Retype letters in the field below:</p>
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
