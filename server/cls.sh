#!/bin/bash
if [ -f atemp.txt ]; then
	echo "rm atemp.txt"
	rm atemp.txt
fi
netstat -anp |grep 8812 |tee atemp.txt
tasknum=`sed -e 's/\/\.\/jqserver//g'  atemp.txt |awk '{print $7}'` 
echo "kill -9 $tasknum"
kill -9 $tasknum
