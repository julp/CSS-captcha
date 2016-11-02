## Description

A (visual only) captcha engine based on CSS3 and Unicode.

A demonstration of the plain php implementation is available online: http://julp.lescigales.org/captcha/

Run dependency: nothing you should already have (only PHP's session extension, no gd or imagemagick extension)

## Usage/installation

### As PHP extension

To use the php extension, build it as any other extension. For a dynamically linked extension:
```
cd /path/to/CSS-Captcha/php-ext
phpize
./configure
make
(sudo) make install
```
And make sure to have a line `extension=captcha.so` in your php.ini (then restart apache or fpm if needed).

### As plain PHP (PHP >= 5.6)

Just grab the file php-plain/CSSCaptcha.php and load it (require) into your script.

## Implementation

### Attributes

* `CSSCaptcha::ATTR_CHALLENGE_LENGTH` <sup>1</sup> <sup>2</sup> (default: `8`): integer, maximum `16`, challenge length
* `CSSCaptcha::ATTR_REVERSED` (default: `CSSCaptcha::RANDOM`): integer, one of `CSSCaptcha::[ALWAYS|NEVER|RANDOM]`, inverse order of displayed element (set it to `CSSCaptcha::NEVER` to disable it)
* `CSSCaptcha::ATTR_UNICODE_VERSION` <sup>1</sup> <sup>2</sup> (default: `CSSCaptcha::UNICODE_6_0_0`): integer, set maximum version of Unicode from which to pick up code points (redefine it with one the constants `CSSCaptcha::UNICODE_X_X_X` or `CSSCaptcha::UNICODE_FIRST`/`CSSCaptcha::UNICODE_LAST`). `CSSCaptcha::ASCII` can be used here to not use Unicode.
* `CSSCaptcha::ATTR_ALPHABET` <sup>1</sup> <sup>2</sup> (default: `"23456789abcdefghjkmnpqrstuvwxyz"`): string, subset of ASCII alphanumeric characters from which to pick characters to generate the challenge (eg: define it to `implode(range('0', '9'))` to only use digits)
* `CSSCaptcha::ATTR_FAKE_CHARACTERS_LENGTH` (default: `2`): integer, from `0` (disabled) to `16`, number of irrelevant characters added to the challenge when displayed
* `CSSCaptcha::ATTR_NOISE_LENGTH` (default: `2`): integer (`0` for none), define the maximum number of noisy characters to add before and after each character composing the challenge. A random number of whitespaces (may be punctuations in the future) will be picked between 0 and this maximum
* `CSSCaptcha::ATTR_FAKE_CHARACTERS_STYLE` (default: `"display: none"`): string, fragment of CSS code to append to irrelevant characters of the challenge
* `CSSCaptcha::ATTR_FAKE_CHARACTERS_COLOR` (default: `CSSCaptcha::COLOR_NONE`): one constant among `CSSCaptcha::COLOR_[RED|GREEN|BLUE|LIGHT|DARK]` to generate a random nuance of the given color
* `CSSCaptcha::ATTR_SIGNIFICANT_CHARACTERS_STYLE` (default: `""`): string, fragment of CSS code to append to significant characters of the challenge
* `CSSCaptcha::ATTR_SIGNIFICANT_CHARACTERS_COLOR` (default: `CSSCaptcha::COLOR_NONE`): one constant among `CSSCaptcha::COLOR_[RED|GREEN|BLUE|LIGHT|DARK]` to generate a random nuance of the given color
* `CSSCaptcha::ATTR_HTML_WRAPPER_TAG` (default: `"div"`): HTML tag name of container element
* `CSSCaptcha::ATTR_HTML_WRAPPER_ID` (default: `"captcha"`): HTML/CSS ID of container element
* `CSSCaptcha::ATTR_HTML_ELEMENT_TAG` (default: `"span"`): HTML tag to display challenge (and fake) characters

Notes:

1. only effective when set through the constructor, not after (for example, `CSSCaptcha::setAttribute` won't work)
2. only affects the generation of a **new** challenge

### Functions

* create a captcha object: `object captcha_create(string $key, CSSCaptchaStoreInterface $serializer [, array $options ]) or CSSCaptcha::__construct(string $key, CSSCaptchaStoreInterface $serializer [, array $options ])`
* render captcha (HTML and CSS can be obtained separately): `string captcha_render(object $captcha [, integer $what = CSSCaptcha::RENDER_HTML | CSSCaptcha::RENDER_CSS ]) or string Captcha::render([ integer $what = CSSCaptcha::RENDER_HTML | CSSCaptcha::RENDER_CSS ])`
* does user input match current challenge (case insensitive and internal counter for attempts incremented): `boolean captcha_validate(object $captcha, string $input) or boolean CSSCaptcha::validate(string $input)`
* cleanup session (remove session key): `void captcha_cleanup(object $captcha) or void CSSCaptcha::cleanup()`
* get initial key associated to the captcha: `string captcha_get_key(object $captcha) or string CSSCaptcha::getKey()`
* get current challenge: `string captcha_get_challenge(object $captcha) or string CSSCaptcha::getChallenge()`
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
define('KEY', pathinfo(__FILE__, PATHINFO_FILENAME));

$options = array(
    CSSCaptcha::ATTR_FAKE_CHARACTERS_STYLE => '',
    CSSCaptcha::ATTR_FAKE_CHARACTERS_COLOR => CSSCaptcha::COLOR_LIGHT,
    CSSCaptcha::ATTR_SIGNIFICANT_CHARACTERS_COLOR => CSSCaptcha::COLOR_BLUE,
);

$captcha = new CSSCaptcha(KEY, new CSSCaptchaSessionStore, $options);
# or
# $captcha = captcha_create(KEY, new CSSCaptchaSessionStore, $options);
if (isset($_POST['captcha'])) {
    if ($captcha->validate($_POST['captcha'])) {
# or
#   if (captcha_validate($captcha, $_POST['captcha'])) {
        echo '<p>You pass. New token created.</p>';
    } else {
        echo '<p>You fail. Retry.</p>';
    }
}
?>
        <form method="post" action="">
            <?php
                echo $captcha->render();
# or
#               echo captcha_render($captcha);
            ?>
            <div style="clear: both;">
                Captcha : <input type="text" name="captcha"/> (enter only blue characters)
            </div>
            <p>Expect: <?php
                var_dump($captcha->getChallenge());
# or
#               var_dump(captcha_get_challenge($captcha));
            ?></p>
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
| Own alphabet (but subset of ASCII alphanumeric characters) can be defined | implemented | implemented | - |
| Choice in Unicode version | implemented | implemented | From Unicode 1.1.0 to 8.0.0 with embedded table |
| Random direction (left/right, through flex) | implemented | implemented | - |
| Random nuance of a given color (`CSSCaptcha::ATTR_*_COLOR`) | implemented | implemented | for testing, can be en/disabled |
| Random prefix "0n+" in nth-child | implemented | implemented | - |
| Fake characters | implemented | implemented | 0 to disable |
| Noisy characters (spaces for now) | implemented | implemented | 0 to disable |
| Way to use it | procedural or OOP | procedural or OOP | - |

## Notes

Reference implementation is: php-ext

## Fake characters

Fake characters can be hidden in different ways with CSS:
* color them with the same color than the background-color
* and/or don't display them (`display: none`)
* and/or make them too small to be visible
* and/or position out of the visible area of the page (eg: `position: absolute; left: -500px;`)

To do so, define your own CSS rule through `CSSCaptcha::ATTR_FAKE_CHARACTERS_STYLE`.
