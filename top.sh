#!/bin/sh
value=`cat pid.txt`
top -p $value
