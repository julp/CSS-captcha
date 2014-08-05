<?php
header('Content-Type: text/html; charset=utf-8');
session_start();

if (!extension_loaded('captcha')) {
    require(dirname(__FILE__) . '/../php-plain/CSSCaptcha.php');
}

define('COMMENT_MIN_WORDS', 3);
define('KEY', pathinfo(__FILE__, PATHINFO_FILENAME));

function h($texte) {
    return htmlspecialchars($texte, ENT_QUOTES, 'UTF-8');
}

$options = array(
    CSSCaptcha::ATTR_FAKE_CHARACTERS_LENGTH => 0,
    CSSCaptcha::ATTR_SKIP_UNICODE_FOR_CHALLENGE => TRUE,
);
if (defined('CSSCaptcha::ATTR_ONLY_LTR')) {
    $options[CSSCaptcha::ATTR_ONLY_LTR] = TRUE;
}

$errors = array();
$captcha = new CSSCaptcha(KEY/*, new CSSCaptchaSessionWriter*/, $options);
if ('POST' == $_SERVER['REQUEST_METHOD']) {
    if (array_key_exists('from', $_POST)) {
        if (!filter_var($_POST['from'], FILTER_VALIDATE_EMAIL)) {
            $errors['from'] = 'email address seems to be invalid';
        }
    } else {
        $errors['from'] = 'email address is missing';
    }
    if (array_key_exists('comment', $_POST)) {
        preg_replace('~\b[[:alpha:]]{3,}\b~u', '', $_POST['comment'], COMMENT_MIN_WORDS, $words_found);
        if (COMMENT_MIN_WORDS != $words_found) {
            $errors['comment'] = sprintf('comment needs at least %d words (only %d found)', COMMENT_MIN_WORDS, $words_found);
        }
    } else {
        $errors['comment'] = 'comment is missing';
    }
    if (array_key_exists('captcha', $_POST)) {
        if (!$captcha->validate($_POST['captcha'])) {
            $errors['security code'] = 'security code is incorrect';
        }
    } else {
        $errors['security code'] = 'security code is missing';
    }
    if (!$errors) {
        # don't really send an email, just log for testing
        file_put_contents(
            'log.txt',
            "[" . date('Y-m-d H:i:s') . "] \"{$_POST['from']}\" as {$_SERVER['REMOTE_ADDR']} wrote \"{$_POST['comment']}\"" . PHP_EOL,
            FILE_APPEND
        );
        $captcha->renew();
        $_SESSION['flash'] = 'Comment sent!';
        header('Location: ' . $_SERVER['REQUEST_URI']);
        exit;
    }
}
?>
<!DOCTYPE html>
<html>
    <head>
        <meta charset="utf-8" />
        <link rel="stylesheet" href="//maxcdn.bootstrapcdn.com/bootstrap/3.2.0/css/bootstrap.min.css"/>
        <style type="text/css">
#captcha {
    padding: 4px 20px;
    display: inline-block;
    border: 1px solid #AAAAAA;
}

.row {
    padding: 1em;
}
        </style>
        <script type="text/javascript">
window.onload = function() {
    var captcha = document.getElementById('captcha-wrapper');
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
        <div class="container-fluid">
            <div class="row">
                <div class="col-xs-12">
<?php
if ($errors) {
    echo '<div class="alert alert-warning"><p>Please fix the following errors to achieve your request:</p><ul><li>', implode('</li><li>', $errors), '</li></ul></div>';
} else if (array_key_exists('flash', $_SESSION)) {
    echo '<p class="alert alert-success">', h($_SESSION['flash']), '</p>';
    unset($_SESSION['flash']);
}
?>
                    <form method="post" action="">
                        <fieldset>
                            <legend>Contact the author:</legend>
                            <div class="form-group <?php if (array_key_exists('from', $errors)) echo 'has-error'; ?>">
                                <label for="from">Your email address:</label>
                                <input type="text" id="from" name="from" class="form-control" value="<?php if (array_key_exists('from', $_POST)) echo h($_POST['from']); ?>" />
                            </div>
                            <div class="form-group <?php if (array_key_exists('comment', $errors)) echo 'has-error'; ?>">
                                <label for="comment">Your comment:</label>
                                <textarea id="comment" name="comment" class="form-control"><?php if (array_key_exists('comment', $_POST)) echo h($_POST['comment']); ?></textarea>
                            </div>
                        </fieldset>
                        <fieldset id="captcha-wrapper">
                            <legend>Security code</legend>
                            <?php echo $captcha->render(); ?>
                            <!--<span> -&gt; </span>
                            <input type="text" name="captcha"/>-->
                            <div class="form-group <?php if (array_key_exists('security code', $errors)) echo 'has-error'; ?>">
                                <p>Retype letters in the field below:</p>
                                <input type="text" name="captcha" class="form-control" />
                                <p><small>(only lower cased letter and digit)</small></p>
                            </div>
                        </fieldset>
                        <input type="submit" value="Send" class="btn btn-default" />
                    </form>
                </div>
            </div>
            <div class="row">
                <div class="col-xs-12">
                    <p class="alert alert-info">Also try this example with javascript disabled.</p>
                </div>
            </div>
        </div>
    </body>
</html>
