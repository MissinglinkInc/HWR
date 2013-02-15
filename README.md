g++ -o index hwr.cpp -L /usr/local/lib /usr/local/lib/libfcgi.a /usr/local/lib/libzinnia.a

spawn-fcgi -a127.0.0.1 -p9000 -n index
