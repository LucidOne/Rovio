1. Modify this macro to yourself's condition
#define TEST_REMOTEFUNC_LOCAL_ADDR "10.132.249.4"
#define TEST_REMOTEFUNC_SERVER_ADDR "10.132.11.10"

2. make
make clean; make

3. run all server
test_server.sh

4. run client to connect W802
the sequence should be:
test_netread
test_recv
test_recvmsg
test_recvfrom
test_netselect
test_getsockname



