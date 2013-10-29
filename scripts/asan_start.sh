rm -f error.out.*
cd ..
rm -f 9969.log
rm -f 9268.log
rm -f 22224.log
rm -f 39967.log
rm -f app.log
ulimit -c unlimited
ASAN_OPTIONS=verbosity=1:log_path=scripts/error.out ./monitor --flagfile=flags.conf 2>&1 | tee scripts/print.txt 
