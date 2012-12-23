<?php
$letters = array_merge(range('0', '9'), range('a', 'z'));
foreach ($letters as $l) {
    echo 'static const char *table_' . $l . '[] = {' . "\n" . '    ';
    $string = '';
    $fp = fopen($l . '.txt', 'r');
    while ($line = fgets($fp)) {
        $string .= '"' . rtrim($line) . '", ';
    }
    fclose($fp);
    echo wordwrap($string, 60, "\n    ") . "\n";
    echo '};' . "\n\n";
}

echo '/* ', str_repeat('=', 50), " */\n\n";

$fp = fopen('ignorable.txt', 'r');
$string = 'static const char *ignorables[] = {' . "\n" . '    ';
while ($line = fgets($fp)) {
    $string .= '"\\' . rtrim($line) . '", ';
}
fclose($fp);
echo wordwrap($string, 60, "\n    ") . "\n" . '};' . "\n";
