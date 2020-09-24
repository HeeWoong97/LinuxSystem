#!/bin/sh
mul=1
for i in `seq 1 $1`
do
	string=""
	for j in `seq 1 $2`
	do
		mul=`expr $i \* $j`
		tmp=$i"*"$j"="$mul" "
		string=$string$tmp
	done
	echo $string
done
exit 0
