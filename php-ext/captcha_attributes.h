LONG_CAPTCHA_ATTRIBUTE(reversed, REVERSED, CAPTCHA_RANDOM, check_never_always_random_attribute)
LONG_CAPTCHA_ATTRIBUTE(unicode_version, UNICODE_VERSION, UNICODE_6_0_0, check_unicode_version_attribute)
LONG_CAPTCHA_ATTRIBUTE(noise_length, NOISE_LENGTH, 2, check_zero_or_positive_attribute)
LONG_CAPTCHA_ATTRIBUTE(challenge_length, CHALLENGE_LENGTH, 8, check_challenge_length_attribute)
LONG_CAPTCHA_ATTRIBUTE(fake_characters_color, FAKE_CHARACTERS_COLOR, 0, check_color_attribute)
LONG_CAPTCHA_ATTRIBUTE(fake_characters_length, FAKE_CHARACTERS_LENGTH, 2, check_fake_characters_length_attribute)
LONG_CAPTCHA_ATTRIBUTE(significant_characters_color, SIGNIFICANT_CHARACTERS_COLOR, 0, check_color_attribute)

// default_alphabet is defined in php_captcha.c
STRING_CAPTCHA_ATTRIBUTE(alphabet, ALPHABET, default_alphabet, NULL)        // TODO: check ~ /^[0-9a-z]{2,}$/ (and distinct)
STRING_CAPTCHA_ATTRIBUTE(session_prefix, SESSION_PREFIX, "captcha_", NULL)  // TODO: check !empty
STRING_CAPTCHA_ATTRIBUTE(html_wrapper_id, HTML_WRAPPER_ID, "captcha", NULL) // TODO: check !empty
STRING_CAPTCHA_ATTRIBUTE(html_letter_tag, HTML_LETTER_TAG, "span", NULL)    // TODO: check !empty
STRING_CAPTCHA_ATTRIBUTE(html_wrapper_tag, HTML_WRAPPER_TAG, "div", NULL)   // TODO: check !empty
STRING_CAPTCHA_ATTRIBUTE(fake_characters_style, FAKE_CHARACTERS_STYLE, "display: none", NULL)
STRING_CAPTCHA_ATTRIBUTE(significant_characters_style, SIGNIFICANT_CHARACTERS_STYLE, "", NULL)
