#!/bin/bash
set -e

rm -f mm*.moos 

# current time in microseconds since unix
t=$(printf `date -u +%s`000000)

for i in 1 2
do 
cpp -D_MODEM_ID_=$i -D_TIME_DATE_=$t mm.moos.in | sed -e 's/#.*//' -e 's/[ ^I]*$//' -e '/^$/ d' > mm$i.moos 
done

