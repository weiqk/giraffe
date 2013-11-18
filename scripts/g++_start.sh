rm -f error.out.*
cd ..
rm -f 9969.log
rm -f 9268.log
rm -f 22224.log
rm -f 39967.log
rm -f app.log
rm -f business_error.log
ulimit -c unlimited
./monitor --flagfile=flags.conf 
