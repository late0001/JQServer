#!/bin/bash
if [ -f atemp.txt ]; then
	echo "rm atemp.txt"
	rm atemp.txt
fi
#netstat -anp |grep 8812 |tee atemp.txt
#tasknum=`sed -e 's/\/\.\/jqserver//g'  atemp.txt |awk '{print $7}'` 

ps -aux  > atemp.txt
tasknum=`sed -n -e '/jq/p' atemp.txt| awk '{print $2}'`
sed -n -e '/jq/p' atemp.txt
echo "kill -9 $tasknum"
kill -9 $tasknum
