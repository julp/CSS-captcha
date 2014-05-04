## Description

A captcha engine based on CSS3 and Unicode.

A demonstration of the plain php implementation is available online: http://julp.lescigales.org/captcha/

Run dependency: nothing you should already have (only PHP's session extension, no gd or imagemagick extension)

## Implementation

### Attributes

* CSSCaptcha::ATTR_CHALLENGE_LENGTH (default: 8): integer, maximum 16, challenge length.
* CSSCaptcha::ATTR_FAKE_CHARACTERS_LENGTH (default: 2): integer, from 0 (disabled) to 16, number of irrelevant characters added to the challenge when displayed.
* CSSCaptcha::ATTR_NOISE_LENGTH (default: 2): integer (0 for none), define the maximum number of noisy characters to add before and after each character composing the challenge. A random number of whitespaces (may be punctuations in the future) will be picked between 0 and this maximum.
* CSSCaptcha::ATTR_SESSION_PREFIX (default: "captcha_"): string, prefix prepended to session key to minimize risks of overwrites
* CSSCaptcha::ATTR_FAKE_CHARACTERS_STYLE (default: "display: none"): string, fragment of CSS code to append to irrelevant characters of the challenge
* CSSCaptcha::ATTR_FAKE_CHARACTERS_COLOR (default: "" - none): one constant among `CSSCaptcha::COLOR_[RED|GREEN|BLUE|LIGHT|DARK]` to generate a random nuance of the given color
* CSSCaptcha::ATTR_SIGNIFICANT_CHARACTERS_STYLE (default: ""): string, fragment of CSS code to append to significant characters of the challenge
* CSSCaptcha::ATTR_SIGNIFICANT_CHARACTERS_COLOR (default: "" - none): one constant among `CSSCaptcha::COLOR_[RED|GREEN|BLUE|LIGHT|DARK]` to generate a random nuance of the given color

Notes:
* `CSSCaptcha::ATTR_CHALLENGE_LENGTH` and `CSSCaptcha::ATTR_SESSION_PREFIX` are only effective when set through the constructor, not after (for example, `CSSCaptcha::setAttribute` won't work)
* `CSSCaptcha::ATTR_CHALLENGE_LENGTH` only affects the generation of a **new** challenge

### Functions

* create a captcha object: `object captcha_create(string $key [, array $options ]) or CSSCaptcha::__construct(string $key [, array $options ])`
* render captcha (HTML and CSS can be obtained separately): `string captcha_render(object $captcha [, integer $what = CSSCaptcha::RENDER_HTML | CSSCaptcha::RENDER_CSS ]) or string Captcha::render([ integer $what = CSSCaptcha::RENDER_HTML | CSSCaptcha::RENDER_CSS ])`
* does user input match current challenge (case insensitive and internal counter for attempts incremented): `boolean captcha_validate(object $captcha, string $input) or boolean CSSCaptcha::validate(string $input)`
* renew challenge: `void captcha_renew(object $captcha) or void CSSCaptcha::renew()`
* cleanup session (remove session key): `void captcha_cleanup(object $captcha) or void CSSCaptcha::cleanup()`
* get initial key associated to the captcha: `string captcha_get_key(object $captcha) or string CSSCaptcha::get_key()`
* get current challenge: `string captcha_get_challenge(object $captcha) or string CSSCaptcha::get_challenge()`
* get the number of attempts: `integer captcha_get_attempts(object $captcha) or integer CSSCaptcha::get_attempts()`
* get current value of an attribute: `mixed captcha_get_attribute(object $captcha, int $attribute) or mixed CSSCaptcha::getAttribute(int $attribute)`
* set value for an attribute: `mixed captcha_set_attribute(object $captcha, int $attribute, mixed $value) or mixed CSSCaptcha::setAttribute(int $attribute, mixed $value)`

## Example

```php
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
define('MAX_ATTEMPTS', 10);
define('KEY', pathinfo(__FILE__, PATHINFO_FILENAME));

$options = array(
    CSSCaptcha::ATTR_FAKE_CHARACTERS_STYLE => '',
    CSSCaptcha::ATTR_FAKE_CHARACTERS_COLOR => CSSCaptcha::COLOR_LIGHT,
    CSSCaptcha::ATTR_SIGNIFICANT_CHARACTERS_COLOR => CSSCaptcha::COLOR_BLUE,
);

if (isset($_POST['captcha'])) {
    $captcha = new CSSCaptcha(KEY, $options);
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
    $captcha = new CSSCaptcha(KEY, $options);
}
?>
        <form method="post" action="">
            <?php echo $captcha->render(); ?>
            <div style="clear: both;">
                Captcha : <input type="text" name="captcha"/> (enter only blue characters)
            </div>
            <p>Expect: <?php var_dump($captcha->getChallenge()); ?></p>
            <p>Attempts: <?php echo $captcha->getAttempts(), ' / ', MAX_ATTEMPTS; ?></p>
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
Here for the token 8z2cx6yw with 2 fake characters - 8**w**z2**u**cx6yw.

## Status of the different implementations

| "Feature" | PHP extension | Plain PHP | Note |
| --------- | ------------- | --------- | ---- |
| Confusables can be en/disabled | at compile time | enabled without source modification | - |
| Choice in Unicode version | at compile time | no (based on 6.1.0 or generate your own tables) | - |
| Random direction (left/right, through float) | not implemented | implemented (for testing, can be en/disabled through `CSSCaptcha::ATTR_ONLY_LTR`) | - |
| Random nuance of a given color (`CSSCaptcha::ATTR_*_COLOR`) | implemented (for testing, can be en/disabled) | not implemented | - |
| Random prefix "0n+" in nth-child | implemented | not implemented | - |
| Fake characters | implemented | implemented | 0 to disable |
| Noisy characters (spaces for now) | implemented | implemented | 0 to disable |
| Way to use it | procedural or OOP | procedural or OOP | - |
