## Description

A captcha engine based on CSS3 and Unicode.

Run dependency: nothing you should already have (only PHP's session extension, no gd or imagemagick extension)

## Implementation

### As a PHP extension

#### INI settings

* captcha.challenge_length (default: 8): integer, maximum 16, challenge length.
* captcha.fake_characters_length (default: 2): integer, from 0 (disabled) to 16, number of irrelevant characters added to the challenge when displayed.
* captcha.noise_length (default: 2): integer (0 for none), define the maximum number of noisy characters to add before and after each character composing the challenge. A random number of whitespaces (may be punctuations in the future) will be picked between 0 and this maximum.
* captcha.session_prefix (default: "captcha_"): string, prefix prepended to session key to minimize risks of overwrites
* captcha.fake_characters_style (default: "display: none"): string, fragment of CSS code to append to irrelevant characters of the challenge
* captcha.fake_characters_color (default: "" - none): one constant among `CSSCaptcha::[RED|GREEN|BLUE|LIGHT|DARK]` to generate a random nuance of the given color
* captcha.significant_characters_style (default: ""): string, fragment of CSS code to append to significant characters of the challenge
* captcha.significant_characters_color (default: "" - none): one constant among `CSSCaptcha::[RED|GREEN|BLUE|LIGHT|DARK]` to generate a random nuance of the given color

#### Functions

* create a captcha object: `object captcha_create(string $key) or Captcha::__construct(string $key)`
* render captcha (HTML and CSS can be obtained separately): `string captcha_render(object $captcha [, integer $what = Captcha::HTML | Captcha::CSS ]) or string Captcha::render([ integer $what = Captcha::HTML | Captcha::CSS ])`
* does user input match current challenge (case insensitive and internal counter for attempts incremented): `boolean captcha_validate(object $captcha, string $input) or boolean Captcha::validate(string $input)`
* renew challenge: `void captcha_renew(object $captcha) or void Captcha::renew()`
* cleanup session (remove session key): `void captcha_cleanup(object $captcha) or void Captcha::cleanup()`
* get initial key associated to the captcha: `string captcha_get_key(object $captcha) or string Captcha::get_key()`
* get current challenge: `string captcha_get_challenge(object $captcha) or string Captcha::get_challenge()`
* get the number of attempts: `integer captcha_get_attempts(object $captcha) or integer Captcha::get_attempts()`

#### Example

```php
<?php
ini_set('captcha.fake_characters_style', '');
ini_set('captcha.fake_characters_color', CSSCaptcha::LIGHT);
ini_set('captcha.significant_characters_color', CSSCaptcha::BLUE);

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
    if ($captcha->validate($_POST['captcha'])) {
        $captcha->renew();
        echo '<p>You pass. New token created.</p>';
    } else if ($captcha->getAttempts() >= MAX_ATTEMPTS) {
        $captcha->renew();
        echo '<p>Too many failures, new token created.</p>';
    } else {
        echo '<p>You fail.</p>';
    }
} else  {
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
```

Generated HTML/CSS code looks like this:
```html
<style type="text/css">
#captcha span:nth-child(0n+6):after { content: "\180E\01D5D6"; color: #095CC3;  }
#captcha span:nth-child(8):after { content: "\FF16\1680\2028"; color: #000AD1;  }
#captcha span:nth-child(4):after { content: "\00B2"; color: #1140BF;  }
#captcha span:nth-child(10):after { content: "\00A0\2006\01D4E6"; color: #1F5FEA;  }
#captcha span:nth-child(0n+2):after { content: "\01D568\2009\2008"; color: #EEF5E5;  }
#captcha span:nth-child(0n+1):after { content: "\01D7FE\000D\0009"; color: #1721BA;  }
#captcha span:nth-child(0n+5):after { content: "\2000\1D64\1680\0020"; color: #EDF8F6;  }
#captcha span:nth-child(0n+7):after { content: "\01D6A1\1680"; color: #1133BB;  }
#captcha span:nth-child(9):after { content: "\202F\FF59\000C"; color: #1351CD;  }
#captcha span:nth-child(3):after { content: "\FF3A\000C"; color: #0668CB;  }
</style>
<div id="captcha">
    <span></span><span></span><span></span><span></span><span></span><span></span><span></span><span></span><span></span><span></span>
</div>
```
(for the token 8z2cx6yw with 2 fake characters - 8*w*z2*u*cx6yw)

### Status of the different implementations

| "Feature" | PHP extension | Plain PHP | Note |
| --------- | ------------- | --------- | ---- |
| Confusables can be en/disabled | at compile time | no (enabled) | - |
| Choice in Unicode version | at compile time | no (based on 6.1.0?) | - |
| Random direction (left/right, through float) | not implemented | implemented (for testing, can be en/disabled) | - |
| Random nuance of a given color (`captcha.*_color`) | implemented (for testing, can be en/disabled) | not implemented | - |
| Random prefix "0n+" in nth-child | implemented | not implemented | - |
| Fake characters | implemented | implemented | 0 to disable |
| Noisy characters (spaces for now) | implemented | implemented | 0 to disable |
| Way to configure it | ini settings (PHP_INI_ALL) | class constants or properties (for now) |
