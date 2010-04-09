#include <windows.h>
#include <assert.h>
#include <stdio.h>


char usb_data[1024*1024];

DWORD WINAPI writeThreadProcess( LPVOID vcom ) 
{
	OVERLAPPED os_write;
	unsigned long ret;
	memset(&os_write, 0 ,sizeof(OVERLAPPED));

	os_write.hEvent = CreateEvent(
        NULL,   // default security attributes 
        FALSE,  // auto reset event 
        FALSE,  // not signaled 
        NULL    // no name
		);

	while(1) 
	{
		SetLastError(0);
		WriteFile((HANDLE)vcom, "Server, I am client!", 20, &ret, &os_write);
		// wait the write file to finish
		 DWORD dwRet = GetLastError();
		 if (ERROR_IO_PENDING == dwRet) {
			WaitForSingleObject(os_write.hEvent, INFINITE );
			ResetEvent(os_write.hEvent);			
		 }

	}
}


HANDLE openVCom(DWORD flag) 
{
	HANDLE hCom;
    hCom = CreateFile( "COM5",
        GENERIC_READ | GENERIC_WRITE,
        0,    // exclusive access 
        NULL, // default security attributes 
        OPEN_EXISTING,
        flag,
        NULL 
        );
	return hCom;

}

/* test read function */
void test1() {
	HANDLE handle;
	DWORD flag;
	DWORD retlen;
	flag = FILE_ATTRIBUTE_NORMAL ;
	handle = openVCom(flag);
	if (handle == INVALID_HANDLE_VALUE) {
		printf("Cann't open virtual com\n");
	}
	char buff[1026];
	for (int i =0; i < 1026; i++) {
		buff[i] = 'C';
	}
//	while(1) 
//	{
	
	/* 1023 char */
	WriteFile(handle,buff,1023,&retlen, NULL);

	/* 1024 char */
	{
	// write 1023 bytes
	WriteFile(handle,buff,1023,&retlen, NULL);
	//write 1 bytes
	WriteFile(handle,"C",1,&retlen, NULL);
	
	}

	
//	}


		/* 1023 char */
	//WriteFile(handle,buff,102,&retlen, NULL);
	
}

void test2() 
{
	HANDLE handle;
	DWORD flag;
	DWORD retlen;
	char buff[1027];
	flag = FILE_ATTRIBUTE_NORMAL;
	handle = openVCom(flag);
	if (handle == INVALID_HANDLE_VALUE) {
		printf("Cann't open virtual com\n");
	}
	
	while (1) 
	{
	
	
	ReadFile(handle,buff, 1024, &retlen, NULL);
	buff[1024] = 0;
	printf("read from server%s\n", buff);
	
	memset(buff, 0, 1027);
	ReadFile(handle,buff, 1026, &retlen, NULL);
	buff[1026] = 0;
	printf("read from server%s\n", buff);
	
	}


}
/*  simultaneity read many thread in server  */
void test4() {
	
	HANDLE handle;
	DWORD flag;
	DWORD retlen;
	flag = FILE_ATTRIBUTE_NORMAL;
	handle = openVCom(flag);
	if (handle == INVALID_HANDLE_VALUE) {
		printf("Cann't open virtual com\n");
	}
	while(1)  {
		WriteFile(handle,"C1", 2, &retlen, NULL);
		WriteFile(handle,"C2", 2, &retlen, NULL);
		WriteFile(handle,"C3", 2, &retlen, NULL);

		printf("Client write:[C1 C2 C3]\n");
	}
	
}

/*  simultaneity write many thread in server  */
void test5() 
{
	HANDLE handle;
	DWORD flag;
	DWORD retlen;
	char ch;
	flag = FILE_ATTRIBUTE_NORMAL;
	handle = openVCom(flag);
	if (handle == INVALID_HANDLE_VALUE) {
		printf("Cann't open virtual com\n");
	}
	while(1)  {
		ReadFile(handle,&ch, 1, &retlen, NULL);
		printf("Server write [%c]", ch);
	}
}
int usbsnd(HANDLE handle, char* buff, int _size)
{
	unsigned long pack_size = 1023;
	unsigned long size = _size;
	unsigned long retlen = 0;
	while( size > 0)
	{
		if ( size < pack_size)
		{
			pack_size = size;
		}
		WriteFile(handle, buff, pack_size, &retlen, NULL);
		if ( retlen != pack_size )
		{
			printf("Write failed\n");
			return _size - size;
		}
		size = size - pack_size;
	}
}
static int total = 0;
int usbrcv(HANDLE handle, char* buff, int _size)
{
	unsigned long headlen;
	unsigned char head[4] = {0};
	//read the head
	unsigned long size = 0;
	unsigned long retlen = 0;
	unsigned long remain = 0;
	char* pos = NULL;
	ReadFile(handle, head, 4, &retlen, NULL);
	//printf("the read head is %8x\n", (int)head);
	if ( retlen != 4)
	{
		printf("Read header failed\n");
	}
	size = head[0] + (head[1] << 8)+ (head[2] << 16) + (head[3] << 24);
	printf("the read size is %d\n", size);
	if ( size > _size)
	{
		size = _size;
	}
	remain = size;
	int package =  113;
	pos = buff;
	while ( remain > 0)
	{
		if ( remain < package)
		{
			package = remain;
		}
		ReadFile(handle, pos, package, &retlen, NULL);
		if ( package != retlen )
		{
			printf("Read body failed\n");
			return size - remain;
		}
		total = total + retlen;
		remain = remain - package;
	    pos = pos + package;			
	}
	return size;

}
void test()
{
	HANDLE handle;
	DWORD flag;
	DWORD retlen;
	char ch;
	int ret = 0;
	int times = 0;
	char *tmp;
	flag = FILE_ATTRIBUTE_NORMAL;
	handle = openVCom(flag);
	if (handle == INVALID_HANDLE_VALUE) {
		printf("Cann't open virtual com\n");
	}
	while(1)  
	{
		//int len = rand()%3019;
		int len = 1024;
		printf("This the [%d] times send [%d] chars\n", times++, len);
		tmp = usb_data; 
		*tmp ++ = (len & 0xff);
	//	printf("%d %d %d %d\n", len, (len >> 4), (len >> 8), (len >> 12));
		*tmp ++ = ((len >> 8) & 0xff);
		*tmp ++ = ((len >> 16) & 0xff);
		*tmp ++ = ((len >> 24) & 0xff);
		for (int i = 0; i < len; i++)
		{
			*tmp ++ = i%117;
		}
		usbsnd(handle, usb_data, len + 4);
		memset(usb_data, 0, 1024*1024);
		ret = usbrcv(handle, usb_data, 1024*1024);
		for (int k = 0; k < ret; k++)
		{
			if ( usb_data[k] != (k%117) )
			{
				printf("Test faild\n");
				return;
			}
			if ( (k%16) == 0 )
			{
				if ( k != 0)
				{
					printf("\n");
				}
			}
			printf("%4d", usb_data[k]);
		}
		printf("\n");
		printf("Until now total receive %d chars\n", total);
		//ReadFile(handle,&ch, 1, &retlen, NULL);
		//printf("Server write [%c]", ch);
	}

}


void test6() 
{
	
	HANDLE hCom;
	OVERLAPPED os_read;
    BOOL fSuccess;
    DWORD dwEvtMask;


	DWORD flag = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED;

    hCom = openVCom(flag);

    if (hCom == INVALID_HANDLE_VALUE) 
    {
        // Handle the error. 
        printf("CreateFile failed with error %d.\n", GetLastError());
        return;
    }

	//create a thread to write some thing to server;

	HANDLE writeThread;
	writeThread = ::CreateThread((LPSECURITY_ATTRIBUTES)NULL, 0,
									(LPTHREAD_START_ROUTINE)writeThreadProcess, 
									hCom, 0, NULL); 
	if (writeThread == NULL)
	{
		printf("Can not create read thread \n");
		return;
	}

	memset(&os_read, 0 ,sizeof(OVERLAPPED));
	os_read.hEvent = CreateEvent(
        NULL,   // default security attributes 
        FALSE,  // auto reset event 
        FALSE,  // not signaled 
        NULL    // no name
		);
  

    // Intialize the rest of the OVERLAPPED structure to zero.

	DWORD wCount;
	char ch[15];

	while(1) 
	{
        SetLastError(0);
		// read char
		ReadFile(hCom, ch, 13, &wCount, &os_read);
		DWORD dwRet = GetLastError();
		if (ERROR_IO_PENDING == dwRet) {
			WaitForSingleObject(os_read.hEvent, INFINITE );
			ResetEvent(os_read.hEvent);	
		}
		ch[14] = 0;
		printf("Server writting: %s\n", ch);
	}
}


int main() {
	test();
	return;
	while(1)
	{
//	test1();
//	test2();
//	test4();
	test6();
	}
//	printf("test over \n");

}
