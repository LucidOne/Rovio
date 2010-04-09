1 .This director contains a sample for libUSB, and unit test case, if you want to test the libUSB, please define macro _UNIT_TEST.
2 There are 6 cases for this library
	test1(): test the boundary condition for read operation.
	test2(): test the boundary condition for write operation.
	test3(): test USBPrintf();
	test4(): test the case of reading simultaneously.
	test5(): test the case of writing simultaneously.
	test6():test the case of read and write invoked at the same time.
3 Except test3(), we all need an assistant application who provides data to USB or consumes data from usb.This application locates in the director "assistant", If we want to test test1(), on board side, we need to add function test1() in function main(), and on host side, we also add its function test1() to its function main().
4 From the UATR port 1, we can get the result.
