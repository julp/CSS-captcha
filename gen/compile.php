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
    $string .= '    ' . 'array(';
    foreach ($a as $v) {
        $string .= "'" . addslashes($v) . "', ";
    }
    $string .= '),' . "\n";
}
$string .= ');';
echo $string;
