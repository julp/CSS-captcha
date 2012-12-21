<?php
$table = array();
$letters = array_merge(range('0', '9'), range('a', 'z'));
foreach ($letters as $k => $l) {
    $table[$k] = array();
    $fp = fopen($l . '.txt', 'r');
    while ($line = fgets($fp)) {
        $table[$k][] = rtrim($line);
    }
    fclose($fp);
}
#var_export($table);
$string = 'array(' . "\n";
foreach ($table as $a) {
    $line = '';
    $string .= '    array(' . "\n";
    foreach ($a as $v) {
        $line .= "'" . $v . "', ";
    }
    $string .= '        ' . wordwrap($line, 60, "\n        ") . "\n" . '    ),' . "\n";
}
$string .= ');';
echo $string;

echo str_repeat('=', 50);

$table = array();
$fp = fopen('ignorable.txt', 'r');
while ($line = fgets($fp)) {
    $table[] = rtrim($line);
}
fclose($fp);
$string = 'array(' . "\n" . '    ';
foreach ($table as $v) {
    $string .= "'" . $v . "', ";
}
echo wordwrap($string, 60, "\n    ") . "\n" . ');';
