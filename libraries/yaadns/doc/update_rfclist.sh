#!/bin/bash

#
# 
#

if test "x$1" = "x" ; then
    echo "Please download the file located at http://www.bind9.net/rfc"
    echo "Bind9 maintains a list of all DNS related RFCs."
    echo "Please give the path to this downloaded file as first parameter."
    exit 1;
fi

cat "$1" | grep -E 'RFC [0-9]+' -o | grep -o -E '[0-9]+' | sort -g -u | grep -v -E '810|822|920|959|1122|1327|1348|1436|1537|1637|1664|1825|1826|1827|1945|2048|2052|2276|2535|2537|2538|2541|2743|2915|3445|4703' >/tmp/ons_rfclist_new

cat detailed-rfc.txt | grep -E '^[0-9]+' -o >/tmp/ons_rfclist_old

diff -u /tmp/ons_rfclist_old /tmp/ons_rfclist_new
