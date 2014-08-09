* [ ] play with CSS3 selector ? (first, last, nth)
* [ ] placement, right to left, left to right (float) ? => experimental in php-plain
* [x] insert random \s before and after ?
* [x] don't generate class from first character to last one: shuffle them ?
* [ ] play with position and/or overflow ?
* [x] insert fake hidden element (display: none) ? + allow user to set CSS code (not necessary display: none ; allow to play on colors?)
* [x] remove confusables ?
  - lower L
  - upper I
  - upper O
* [x] remove code points newer than Unicode 5.0.0 ? (see unicode/uchar.h, function u_charAge and unicode/uversion.h, function u_versionFromString, then memcmp both)
* [ ] generate a random id instead of `<div id="captcha">` ? (without digits)
* [ ] wider use of Unicode ?
* [ ] add CSSCaptcha::ATTR_ALPHABET ?
* [ ] ruby gem ?
* [ ] split code in 2 or 3 parts (challenge generation, rendering, data serialization - to use something else than sessions, redis for example)

CSS obfuscation:
* [ ] synonyms for nth-child ? 1/X = first-child, X/X = last-child, ...
* [x] mathematical expressions for nth-child ? (5 => 3 + 2 ?) => only prefix "0n+" is "valid"
* [ ] random comments (/* ... */)

php-plain:
* [ ] missing checks on \*\_length attributes
* [x] fake characters are not kept
* [ ] external files for serialized tables
* [x] colors
