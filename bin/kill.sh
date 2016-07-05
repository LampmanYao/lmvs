PID=`cat my.pid`
ps aux | grep -v grep | grep ./lmvs | grep $PID | awk '{print $2}' | xargs kill -9
