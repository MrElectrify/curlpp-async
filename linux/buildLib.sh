[ ! -d "lib/" ] && mkdir lib
g++ --std=c++17 -fPIC -Wall -I../include -I../dependencies/curl/include/ -I../dependencies/openssl/include/ ../src/Handle.cpp ../src/WebClient.cpp -c
ar rcs libcurlpp-async.a Handle.o WebClient.o
mv libcurlpp-async.a lib/
rm *.o