#!/bin/sh
while true
do
value=`cat pid.txt`
top -p $value
sleep 1
kill $$
done
