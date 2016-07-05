ulimit -c unlimited
nohup ./lmvs ./server.conf &
echo $! > my.pid
