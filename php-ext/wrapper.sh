#!/bin/bash

declare -ri TRUE=0
declare -ri FALSE=1

declare -rA PHP_PREFIX_MAP=(
    ["5"]="/usr/local"
    ["7"]=$HOME
)

if [ $# -lt 2 ]; then
    echo "ArgumentError: ${FUNCNAME[0]}(version, task ...)"
    exit $FALSE
fi

if [ -z "${PHP_PREFIX_MAP[$1]}" ]; then
    echo "ArgumentError: unknown php version ${1}"
    exit $FALSE
fi

if [[ ${PHP_PREFIX_MAP[$1]} == ${HOME}* ]]; then
    SUDO=""
else
    SUDO="sudo"
fi

case $2 in
    fullbuild)
        make distclean && ${PHP_PREFIX_MAP[$1]}/bin/phpize && ./configure --with-php-config=${PHP_PREFIX_MAP[$1]}/bin/php-config && make && $SUDO make install
    ;;
    build)
        make && $SUDO make install
    ;;
    test)
        TEST_PHP_EXECUTABLE=${PHP_PREFIX_MAP[$1]}/bin/php ${PHP_PREFIX_MAP[$1]}/bin/php run-tests.php -q # -m
    ;;
    server)
        ${PHP_PREFIX_MAP[$1]}/bin/php -S localhost:3000 -t ../php-examples/
    ;;
    gdb)
        gdb ${PHP_PREFIX_MAP[$1]}/bin/php
    ;;
    php)
        ${PHP_PREFIX_MAP[$1]}/bin/php ${@:3}
    ;;
    *)
        echo "ArgumentError: unknown task ${2}"
        exit $FALSE
    ;;
esac
