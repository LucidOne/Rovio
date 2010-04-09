#include "vp_com.h"


#define PACKAGE_SIZE 1024*2
#define NUM_FOR_WRITE 16

#define MAX_CUSTOM_COMMAND 10
#define MAX_CUSTOM_PARA_PER_CMD 5
#define MAX_CUSTOM_PARA_NAME_LEN 20
#define MAX_CUSTOM_COMMAND_LEN 10


#define MAX_CUSTOM_COMMAND_LINE 128
#define MAX_DEBUG_SIZE	1024

#define VCOM_TOKEN_MSG 	1
#define VCOM_TOKEN_REPLY	2

#define VCOM_EVENT_PRINT	1
#define VCOM_EVENT_COMMAND	2


typedef struct customHandlerArray {
	char cmd[MAX_CUSTOM_COMMAND_LEN + 1];	/* cmd name */
	char paraList[MAX_CUSTOM_PARA_PER_CMD][MAX_CUSTOM_PARA_NAME_LEN + 1];/* the parameter list for current amd */
	int paraNum;
	customHandler handler; /* cmd handle */
	
} CUSTOM_HANDLER_ARRAY;

static CUSTOM_HANDLER_ARRAY cmdHandler[MAX_CUSTOM_COMMAND];
static int max_custom_command = 0;

void vcomThreadEntry (cyg_addrword_t index);
int xdrsystemSettingLenth(struct systemSetting* result);
int xdrsystemSettingStru(int opCode,struct systemSetting* result, char* buff) ;
int xdrextendSettingLenth(struct extendSetting* result);
int xdrextendSettingStru(int opCode,struct extendSetting* result, char* buff);
int xdripSettingStru(int opCode,struct ipSetting* result, char* buff);
int xdripSettingLenth(struct ipSetting* result);
int xdrftpSettingStru(int opCode,struct ftpSetting* result, char* buff);
int xdrftpSettingLenth(struct ftpSetting* result);
int xdremailSettingStru(int opCode,struct emailSetting* result, char* buff) ;
int xdremailSettingLenth(struct emailSetting* result) ;
int xdrimageSettingStru(int opCode,struct imageSetting* result, char* buff) ;
int xdrimageSettingLenth(struct imageSetting* result);
int xdrrequestStatusStru(int opCode,struct requestStatus* result, char* buff);
int xdrrequestStatusLenth(struct requestStatus* result);
int xdrlogonReqStru(int opCode,struct logonReq* logon, char* buff);
int xdrlogonReqLenth(struct logonReq* logon);
int xdrUserSettingStru(int opCode,struct userSetting* set, char* buff);
int xdrUserSettingLenth(struct userSetting* set);
void processClient(void );
int parseCommand(char* cmdLine, char** args, char** token, int num);
void initCustomHander(void);
void vcomDump(void );
//static int myUSBRead(char* des, unsigned int* lenth);

/* creat a thread */
/* this is for create a thread */

#define VCOM_THREAD_STACK_SIZE (16*1024)

int vcom_thread_stack[ VCOM_THREAD_STACK_SIZE ];
cyg_handle_t vcom_thread_handle;
cyg_handle_t vcom_thread_write_handle;
cyg_thread vcom_thread_obj;
cyg_thread vcom_thread_write_obj;

// cyg_sem_t sem_vcom_event; 

/* indicate whether dump console windows is ok */
volatile BOOL g_bIsMac = FALSE;
volatile BOOL g_vcom_init = FALSE;
volatile  int vcom_console_able = 0;
char dumpBuff[MAX_DEBUG_SIZE];
int dumpHead = 0;
int dumpTail = 0;
cyg_mutex_t mut_vcom_write;
int vcomEventType = 0;
//char fmiBuff[PACKAGE_SIZE + 1];
char* fmiBuff = NULL;

#define NEXTONE(p) (((p) + 1 >= MAX_DEBUG_SIZE) ? NULL : (p) + 1)

extern void hi_uart_write( const char *buf, size_t size );


/*
 Koitech's MAC driver has some problem, resend the USB data so that
 their driver can receive the data correctly.
 */
static void USBWrite_Again(const char *buffer, unsigned int *size)
{
	unsigned int length;
	
	//diag_printf("USBWrite_Again\n");
	length = *size;
	if (USBWrite((char *)buffer,&length) < 0)
	{
		USBResetForVCom();
		return;
	}
	
	if (g_bIsMac)
	{
		char magic[] = {0xFF, 0xFE, 0xFD, 0xFC,
			0xFB, 0xFA, 0xF9, 0xF8,
			0xF7, 0xF6, 0xF5, 0xF4,
			0xF3, 0xF2, 0xF1, 0xF0};
		unsigned int magic_len = sizeof(magic);
		if (USBWrite(magic, (unsigned int*)&magic_len) < 0)
		{
			USBResetForVCom();
			return;
		}

		length = *size;
		if (USBWrite((char *)buffer,&length) < 0)
		{
			USBResetForVCom();
			return;
		}
	}
}



/* caculate the lenth of a string, if the string is NULL, the lenth is 1 */
int xdrStringLenth(char* str) {
	int len;
	if (str == NULL)
		return 1; // this position store a ''
	else {
		len = strlen(str) + 1 ;
	
	}
	return len;
	
}
/* must handle end-little and big-little*/

char* xdrInt(int opCode,char* buff,int* aa) 
{
	int a;
	a = *aa; 
	if (opCode == XDR_ENCODE) 
	{
		buff[0] =(char) (a & 0x0ff);
		buff[1] = (char)((a >> 8) & 0x0ff);
		buff[2] = (char)((a >> 16) & 0x0ff);
		buff[3] = (char)((a >> 24) & 0x0ff);
		return (char*)(buff+4);
	} else if (opCode == XDR_DECODE) 
	{
		*aa = 0;
		*aa |= ((int)(unsigned char)buff[3] << 24);
		*aa |= ((int)(unsigned char)buff[2] << 16);
		*aa |= ((int)(unsigned char)buff[1] << 8);
		*aa |= (int)(unsigned char)buff[0];
		return (char*)(buff+4);
	}
	else
	{
		cyg_interrupt_disable();
		//diag_printf("Can not goto here\n");
		while(1);
	}

}

/* decode a signed integer */
char* xdrSignedInt(int opCode,char* buff,int* aa) 
{
	int a;
	//int minus = 0;	/* Bullshit! */
	a = *aa; 
	if (opCode == XDR_ENCODE) 
	{
		buff[0] =(char) (a & 0x0ff);
		buff[1] = (char)((a >> 8) & 0x0ff);
		buff[2] = (char)((a >> 16) & 0x0ff);
		buff[3] = (char)((a >> 24) & 0x0ff);
		return (char*)(buff+4);
	} else if (opCode == XDR_DECODE) 
	{
#if 1
		*aa = 0;
		*aa |= ((int)(unsigned char)buff[3] << 24);
		*aa |= ((int)(unsigned char)buff[2] << 16);
		*aa |= ((int)(unsigned char)buff[1] << 8);
		*aa |= (int)(unsigned char)buff[0];
#else
		// if a minus
		if (buff[3] & 0x40 != 0) {
			minus = 1;
			buff[0] = !buff[0];
			buff[1] = !buff[1];
			buff[2] = !buff[2];
			buff[3] = !buff[3];
		}
		*aa = 0;
		*aa += (buff[3] << 24);
		*aa += (buff[2] << 16);
		*aa += (buff[1] << 8);
		*aa += buff[0];
		if (minus == 1) {
			*aa = *aa - 1;
		}
#endif
		return (char*)(buff+4);
	}
	else
	{
		cyg_interrupt_disable();
		//diag_printf("Can not goto here\n");
		while(1);
	}

}/* xdrSignedInt */

char* xdrUnsignedInt(int opCode,unsigned char* buff,unsigned int* aa) 
{
	unsigned int a;
	a = *aa; 
	if (opCode == XDR_ENCODE) 
	{
		buff[0] =(unsigned char) (a & 0x0ff);
		buff[1] = (unsigned char)((a >> 8) & 0x0ff);
		buff[2] = (unsigned char)((a >> 16) & 0x0ff);
		buff[3] = (unsigned char)((a >> 24) & 0x0ff);
		return (char*)(buff+4);
	} else if (opCode == XDR_DECODE) 
	{
		*aa = 0;
		*aa |= ((int)(unsigned char)buff[3] << 24);
		*aa |= ((int)(unsigned char)buff[2] << 16);
		*aa |= ((int)(unsigned char)buff[1] << 8);
		*aa |= (int)(unsigned char)buff[0];
		return (char*)(buff+4);
	}
	else
	{
		cyg_interrupt_disable();
		//diag_printf("Can not goto here\n");
		while(1);
	}

}

int xdrIterLenth(struct Iter* iter)
{
	return 4*4;

}
char* xdrIterStru(int opCode,char* buff,struct Iter* iter)
{
	char* ret;
	ret = xdrInt(opCode, buff, &(iter->min));
	ret = xdrInt(opCode, ret, &(iter->max));
	ret = xdrInt(opCode, ret, &(iter->step));
	ret = xdrInt(opCode, ret, &(iter->current));
	
	return ret;


}

/* the str return the buff which exist in Buff */
char* xdrString(int opCode,char* buff,char** str)
{
	char* ret;
	if (opCode == XDR_ENCODE) 
	{
		if (*str == NULL) 
		{
			*(buff++) = '\0';
			return buff;
		} else {
			*buff = 0;	
			strcpy(buff,*str);
			ret = buff + strlen(*str) + 1;
			return ret;
		}
	} else if (opCode == XDR_DECODE) 
	{
		*str = buff;
		ret = buff + strlen(*str) + 1;
		return ret;

	}
	else
	{
		cyg_interrupt_disable();
		//diag_printf("Can not goto here\n");
		while(1);
	}
}

/* caculate pointerArray's lenth */
int xdrpointerArrayLenth(struct pointerArray* array)
{
		int lenth = 0;
		int index = 0;
		lenth += 4; // num
		for (index = 0; index < array->num; index++ ) 
		{
			lenth +=  xdrStringLenth(array->array[index]);
		}
		return lenth;
	
	
}

char* xdrpointerArrayStru(int opCode,char* buff, struct pointerArray* array)
{
	char* ret;
	int i = 0;
	ret = xdrInt(opCode,buff,&(array->num));
	if (array->num < 0 || (array->num > MAX_ELEMENT)) 
		return NULL;
	for (i = 0; i < array->num; i++) 
	{
		ret = xdrString(opCode, ret, &(array->array[i]));

	}
	return ret;	
}



/* caculate UserSetting lenth  */
int xdrUserSettingLenth(struct userSetting* set)
{
	int lenth = 0;
	lenth +=  4; // int
	lenth +=  4; //int
	lenth += 4;
	lenth +=  xdrStringLenth(set->originalName);
	lenth +=  xdrStringLenth(set->originalPasswd);
	lenth +=  xdrStringLenth(set->newName);
	lenth +=  xdrStringLenth(set->newPasswd);
	return lenth;
}

/* encode or decode a struct to/from a string*/
int xdrUserSettingStru(int opCode,struct userSetting* set, char* buff)
{
	/* if error ,ret may be == null */
	char* ret;
	ret = xdrInt(opCode,buff,&(set->checkuser));
	ret = xdrInt(opCode,ret,&(set->type));
	ret = xdrInt(opCode,ret,&(set->nPrivilege));
	ret = xdrString(opCode,ret,&(set->originalName));
	ret = xdrString(opCode,ret,&(set->originalPasswd));
	ret = xdrString(opCode,ret,&(set->newName));
	ret = xdrString(opCode,ret,&(set->newPasswd));
	return 1;

}


/* caculate logonReq lenth  */
int xdrlogonReqLenth(struct logonReq* logon)
{
	int lenth = 0;
	lenth +=  xdrStringLenth(logon->name);
	lenth +=  xdrStringLenth(logon->passwd);
	return lenth;

}

int xdrlogonReqStru(int opCode,struct logonReq* logon, char* buff)
{
	char* ret;
	ret = xdrString(opCode,buff,&(logon->name));
	ret = xdrString(opCode,ret,&(logon->passwd));
	return 1;

}


/* caculate requestStatus lenth  */
int xdrrequestStatusLenth(struct requestStatus* result)
{
	int lenth = 0;
	lenth += 4;
	lenth +=  xdrStringLenth(result->msg);
	lenth +=  xdrStringLenth(WRGetProductVersion());
	return lenth;
}
int xdrrequestStatusStru(int opCode,struct requestStatus* result, char* buff) 
{
	char* ret;
	char *version = WRGetProductVersion();
	ret = xdrInt(opCode,buff,&(result->errNum));
	ret = xdrString(opCode,ret,&(result->msg));
	ret = xdrString(opCode,ret,&version);
	return 1;
}

/* caculate imageSetting lenth */
int xdrimageSettingLenth(struct imageSetting* result) 
{
	return sizeof(struct imageSetting);
}

int xdrimageSettingStru(int opCode,struct imageSetting* result, char* buff) 
{
	char* ret;
	ret = xdrInt(opCode,buff,&(result->motionDetect));
	ret = xdrInt(opCode,ret,&(result->quality));
	ret = xdrInt(opCode,ret,&(result->resolutionX));
	ret = xdrInt(opCode,ret,&(result->resolutionY));
	ret = xdrInt(opCode,ret,&(result->frameRate));	

	return 1;
}

/* caculate emailSetting lenth */
int xdremailSettingLenth(struct emailSetting* result) 
{
	int lenth = 0;
	lenth = lenth + 4;
	lenth = lenth + 4;
	lenth +=  xdrStringLenth(result->userName);
	lenth +=  xdrStringLenth(result->passwd);
	lenth +=  xdrStringLenth(result->sendAddr);
	lenth +=  xdrStringLenth(result->recAddr);
	lenth +=  xdrStringLenth(result->mailSubject);
	return lenth;	
}

int xdremailSettingStru(int opCode,struct emailSetting* result, char* buff) 
{
	char* ret;
	ret = xdrInt(opCode,buff,(int *)&(result->mailServer));
	ret = xdrInt(opCode,ret,&(result->sendMail));
	ret = xdrString(opCode,ret,&(result->userName));
	ret = xdrString(opCode,ret,&(result->passwd));
	ret = xdrString(opCode,ret,&(result->sendAddr));
	ret = xdrString(opCode,ret,&(result->recAddr));
	ret = xdrString(opCode,ret,&(result->mailSubject));
	return 1;
}

/* calculate ftpSetting lenth*/
int xdrftpSettingLenth(struct ftpSetting* result)
{
	int lenth = 0;
	lenth += 4;
	lenth += 4;
	lenth += xdrStringLenth(result->userName);
	lenth += xdrStringLenth(result->passwd);
	lenth += xdrStringLenth(result->account);
	lenth += xdrStringLenth(result->path);
	return lenth;

}
int xdrftpSettingStru(int opCode,struct ftpSetting* result, char* buff) 
{
	char* ret;
	ret = xdrInt(opCode,buff,&(result->sendMail));
	ret = xdrInt(opCode,ret,(int *)&(result->ftpServer));
	ret = xdrString(opCode,ret,&(result->userName));
	ret = xdrString(opCode,ret,&(result->passwd));
	ret = xdrString(opCode,ret,&(result->account));
	ret = xdrString(opCode,ret,&(result->path));
	return 1;
}

/* calculate ipSetting lenth*/

int xdripSettingLenth(struct ipSetting* result)
{
//	int lenth = 0;

	return sizeof(struct ipSetting);

}
int xdripSettingStru(int opCode,struct ipSetting* result, char* buff) 
{
	char* ret;
	ret = xdrInt(opCode,buff,&(result->autoGetIP));
	ret = xdrUnsignedInt(opCode,(unsigned char*)ret,(unsigned int*)&(result->ip));
	ret = xdrUnsignedInt(opCode,(unsigned char*)ret,(unsigned int*)&(result->subnetMask));
	ret = xdrUnsignedInt(opCode,(unsigned char*)ret,(unsigned int*)&(result->gateway));
	ret = xdrUnsignedInt(opCode,(unsigned char*)ret,(unsigned int*)&(result->dns1));
	ret = xdrUnsignedInt(opCode,(unsigned char*)ret,(unsigned int*)&(result->dns2));
	ret = xdrUnsignedInt(opCode,(unsigned char*)ret,(unsigned int*)&(result->dns3));
	return 1;
}

/* calculate extendSetting lenth*/
int xdrextendSettingLenth(struct extendSetting* result)
{
	int lenth = 0;
	lenth = lenth + 4;
	lenth = lenth + 4;
	lenth = lenth + xdrStringLenth(result->date);
	lenth = lenth + xdrStringLenth(result->time);
	lenth = lenth + xdrStringLenth(result->timeZone);
	return lenth;

}
int xdrextendSettingStru(int opCode,struct extendSetting* result, char* buff) 
{
	char* ret;
	ret = xdrInt(opCode,buff,&(result->autoGetTime));
	ret = xdrInt(opCode,ret,(int *)&(result->timeServer));
	ret = xdrString(opCode,ret,&(result->date));
	ret = xdrString(opCode,ret,&(result->time));
	ret = xdrString(opCode,ret,&(result->timeZone));
	return 1;
}

/* calulate systemSetting lenth */
int xdrsystemSettingLenth(struct systemSetting* result)
{
	int lenth = 0;
	lenth += 4; 
	lenth += 4; 
	lenth += 4; 
	lenth += 4; //debug;
	lenth += xdrStringLenth(result->mac_address); 
	return lenth;

}
int xdrsystemSettingStru(int opCode,struct systemSetting* result, char* buff) 
{
	char* ret;
	ret = xdrInt(opCode,buff,&(result->type));
	ret = xdrInt(opCode,ret,&(result->videoFormat));
	ret = xdrInt(opCode,ret,&(result->audioFormat));
	ret = xdrInt(opCode,ret,&(result->debug));
	ret = xdrString(opCode,ret,&(result->mac_address));

	return 1;
}
/* caculate the lanth of wlansetting */
int xdrwlanSettingLenth(struct wlanSetting* wlan)
{
	int lenth = 0;
	lenth = lenth + 4; //ulWlanChannel
	lenth = lenth + xdrStringLenth(wlan->handleMethod);
	lenth = lenth + xdrStringLenth(wlan->pcWepSet);
	lenth = lenth + xdrStringLenth(wlan->pcWepAsc);
	lenth = lenth + xdrStringLenth(wlan->pcWep128);
	lenth = lenth + xdrStringLenth(wlan->pcWep64);

	lenth = lenth + xdrStringLenth(wlan->pcWep64type);
	lenth = lenth + xdrStringLenth(wlan->pcWep128type);

	lenth = lenth + xdrStringLenth(wlan->pcWlanESSID);
	return lenth;

}
int xdrwlanSettingStru(int opCode,struct wlanSetting* wlan, char* buff) 
{
	char* ret;
	ret = xdrUnsignedInt(opCode,(unsigned char*)buff,(unsigned int *)&(wlan->ulWlanChannel));
	ret = xdrString(opCode,ret,&(wlan->handleMethod));
	ret = xdrString(opCode,ret,&(wlan->pcWepSet));
	ret = xdrString(opCode,ret,&(wlan->pcWepAsc));
	ret = xdrString(opCode,ret,&(wlan->pcWep128));
	ret = xdrString(opCode,ret,&(wlan->pcWep64));

	ret = xdrString(opCode,ret,&(wlan->pcWep128type));
	ret = xdrString(opCode,ret,&(wlan->pcWep64type));

	ret = xdrString(opCode,ret,&(wlan->pcWlanESSID));
	return 1;
}






/* calculate initialInfo lenth */
int xdrinitialInfoLenth(struct initialInfo* info)
{
	int lenth = 0;
//	lenth += 4;	//major version
//	lenth += 4;	//minor version

//	lenth += 4; //	int extend_GetFromServer;
//	lenth += 4; //	int extend_GetFromLocal;
//	lenth += 4; //int formate; // system page
	lenth += 4; //	int auth_CheckUser; // user page
	lenth += 4;
	lenth += 4; // motion detect;
	lenth += 4; //vedio format;
	lenth += 4; // audio format;
	lenth += 4;// admin index



	lenth += xdrIterLenth(&(info->quality));
	lenth += xdrIterLenth(&(info->frame));
	lenth += xdrpointerArrayLenth(&(info->resulationList));
	lenth += xdrpointerArrayLenth(&(info->userList));
	lenth += xdripSettingLenth(&(info->ipset)); // ip setting
	lenth += xdrwlanSettingLenth(&(info->wlan)); // waln setting
	lenth += xdrStringLenth(info->mac_address);
	lenth += 4 * info->userList.num;	/* Privilege */

//	lenth += xdremailSettingLenth(&(info->mail)); // email 

//	lenth += xdrftpSettingLenth(&(info->ftp)); //ftp page

	// extend page
//	lenth +=  xdrStringLenth(info->serverVersion);
//	lenth +=  xdrStringLenth(info->serverTime);
//	lenth +=  xdrStringLenth(info->date);
//	lenth +=  xdrStringLenth(info->time);
//	lenth +=  xdrStringLenth(info->timeZoom);
	return lenth;

}

int xdrinitialInfoStru(int opCode,struct initialInfo *info, char* buff) 
{
	int i;
	char* ret = buff;
//	ret = xdrInt(opCode,buff,&(info->extend_GetFromServer));
//	ret = xdrInt(opCode,ret,&(info->extend_GetFromLocal));
//	ret = xdrInt(opCode,ret,&(info->formate));

//	int major_version;
//	int minor_version;

//	WRGetProductVersionNum(&major_version, &minor_version);
//	ret = xdrInt(opCode,ret,&major_version);
//	ret = xdrInt(opCode,ret,&minor_version);

	ret = xdrInt(opCode,ret,&(info->auth_CheckUser));
	ret = xdrInt(opCode,ret,&(info->current_resolation));
	ret = xdrInt(opCode,ret,&(info->motion_detect));
	ret = xdrInt(opCode,ret,&(info->videoFormat));
	ret = xdrInt(opCode,ret,&(info->audioFormat));
	ret = xdrInt(opCode,ret,&(info->adminIndex));

	//	ret = xdrpointerArrayStru(opCode,ret,&(info->resulationList));
	ret = xdrIterStru(opCode,ret,&(info->quality));
	ret = xdrIterStru(opCode,ret,&(info->frame));
	ret = xdrpointerArrayStru(opCode,ret,&(info->userList));
	
	ret = xdrpointerArrayStru(opCode,ret,&(info->resulationList));

	// xdr ip setting
	xdripSettingStru(opCode,&(info->ipset), ret);
	ret = ret + xdripSettingLenth(&(info->ipset));
	xdrwlanSettingStru(opCode,&(info->wlan), ret);
	ret = ret +  xdrwlanSettingLenth(&(info->wlan));
	ret = xdrString(opCode,ret,&(info->mac_address));

	for (i = 0; i < info->userList.num; ++i)
		ret = xdrInt(opCode,ret,&(info->userPrivilege[i]));

	//xdr email setting
//	xdremailSettingStru(opCode,&(info->mail), ret);
//	ret = ret + xdremailSettingLenth(&(info->mail));

	// xdr ftp setting

//	xdrftpSettingStru(opCode,&(info->ftp), ret);
//	ret = ret + xdrftpSettingLenth(&(info->ftp));	

//	//xdr extend
//	ret = xdrString(opCode, ret,&(info->serverVersion));
//	ret = xdrString(opCode, ret, &(info->serverTime));
//	ret = xdrString(opCode, ret,&(info->date));
//	ret = xdrString(opCode, ret,&(info->time));
//	ret = xdrString(opCode, ret,&(info->timeZoom));

	return 1;
}

/////////////////////////////////////////////////
// 
int xdrcustomSettingLenth(struct customSetting* custom) 
{
	int lenth = 0;
	lenth = lenth + xdrStringLenth(custom->value);
	return lenth;
}


int xdrcustomSettingStru(int opCode,struct customSetting* custom, char* buff) 
{
	char* ret;
	ret = xdrString(opCode,buff,&(custom->value));
	return 1;
}

/////////////////////////////////////////////////
//calculate the lenth of firmware
int xdrfirmwareLenth(struct firmware* firm) 
{
	return sizeof(struct firmware);
}

/////////////////////////////////////////////////
//
int xdrfirmwareStru(int opCode,struct firmware* firm, char* buff) 
{
	char* ret;
	ret = xdrUnsignedInt(opCode,(unsigned char* )buff,&(firm->lenth));
	return 1;
}

////////////////////////////////////////////////
//
int xdrwlanAPEntryLenth(struct wlanAPEntry* ap)
{
	int lenth = 0;
	lenth += 4; // mode
	lenth += 4; // signal
	lenth += 4; // noise
	lenth += 4; // encode
	lenth = lenth + xdrStringLenth(ap->essid);
	return lenth;

}

//////////////////////////////////////////////
//
int xdrwlanAPEntryStru(int opCode, struct wlanAPEntry* ap, char* buff)
{
	char* ret;
	ret = xdrUnsignedInt(opCode,(unsigned char*)buff,(unsigned int *)(&(ap->mode)));
	ret = xdrUnsignedInt(opCode,(unsigned char*)ret,(unsigned int *)(&(ap->signal)));
	ret = xdrUnsignedInt(opCode,(unsigned char*)ret,(unsigned int*)(&(ap->noise)));
	ret = xdrUnsignedInt(opCode,(unsigned char*)ret,(unsigned int*)(&(ap->encode)));
	
	//diag_printf("Encode: %d %d %d %d\n", ap->mode, ap->signal, ap->noise, ap->encode);
	ret = xdrString(opCode,ret,&(ap->essid));
	return 1;
}
///////////////////////////////////////////
//
int xdrwlanAPArrayLenth(struct wlanAPArray* ap)
{
	int lenth = 0;
	int num = 0;
	int i;
	struct wlanAPEntry* ent;
	ent = ap->array;
	lenth += 4; // for num;
	num = ap->num;
	for (i = 0; i < num ; i++) {
		lenth = lenth + xdrwlanAPEntryLenth(ent);
		ent ++;
	}
	return lenth;

}
///////////////////////////////////////////
//
int xdrwlanAPArrayStru(int opCode, struct wlanAPArray* ap, char* buff)
{
	char* ret;
	int num = 0;
	int i;
	struct wlanAPEntry* ent;
	ent = ap->array;
	if (opCode == XDR_DECODE) 
	{
		ret = xdrInt(opCode,buff,&(ap->num));
		num = ap->num;
	} else {
		num = ap->num;
		ret = xdrInt(opCode,buff,&(ap->num));
	}
	for ( i = 0; i < ap->num; i++) 
	{
		xdrwlanAPEntryStru(opCode, ent, ret);
		ret = ret + xdrwlanAPEntryLenth(ent);
		ent ++;

	}

	return 1;
}
static void string_mac_to_long(char* src, char* dest)
{
	int i = 0;
	int j = 0;
	for (i = 0; i < 6*3 - 1; i++)
	{
		if((src[i] >= 'A') &&(src[i]) <= 'F')
		{
			dest[j/2] = (dest[j/2] << 4)+ src[i] - 'A' + 10;
		} else if ((src[i] >= 'a') &&(src[i]) <= 'z'){
			dest[j/2] = (dest[j/2] << 4) + (src[i] - 'a') + 10;
		} else if ((src[i] >= '0') &&(src[i]) <= '9'){
			dest[j/2] = (dest[j/2] << 4) + (src[i] - '0');
		} else {
			continue;
		}
		j ++;
	}
	
}


//reset vcom, when client ends abnomore, so we should unplug usb wire, this function will be called at this time
void vcomReset(void)
{
	dumpTail = 0;
	dumpHead = 0;
	//cyg_mutex_init(&mut_vcom_write);
   // cyg_mutex_init(&mut_vcom_console);
    vcom_console_able = 0;
}


void VirtualComInit()
{
    
    //outpw (REG_CLKDIV0, inpw (REG_CLKDIV0) | 0x0020000);
    
    /* inital usb */
    USBInitForVCom();
    //registe a callback for usb unplug
    USBRegiesterException(vcomReset);
   // cyg_semaphore_init(&sem_vcom_event,0);
    cyg_mutex_init(&mut_vcom_write);
    /* create main thread */
    cyg_thread_create(PTD_PRIORITY,vcomThreadEntry,
 		0,     
		"Virtual com  Thread",      
		&vcom_thread_stack,      
		VCOM_THREAD_STACK_SIZE,      
		&vcom_thread_handle,      
		&vcom_thread_obj );       
  cyg_thread_resume(vcom_thread_handle);
  	g_vcom_init = TRUE;
}/* VcomByUSBInit  */




/* this is server thread to handle usb command from client */
void vcomThreadEntry (cyg_addrword_t index)
{
	   
	while (1)   
	{   	    
				
		processClient();	
		//cyg_thread_delay(20); 
	}   
}




/* sent the reply to client */
void statusReply(struct requestStatus* reply)
{
	char replyBuf[MAX_REQUEST_LINE];
	int retlenth;
	
	retlenth = xdrrequestStatusLenth(reply);
	replyBuf[2] = VCOM_TOKEN_REPLY;
	replyBuf[3] = 0;
	replyBuf[0] = retlenth & 0xFF;
	replyBuf[1] = (retlenth >> 8)&0xFF;
	xdrrequestStatusStru(XDR_ENCODE, reply,&replyBuf[4]);
	retlenth = retlenth + 4;
	cyg_mutex_lock(&mut_vcom_write);
	USBWrite_Again(replyBuf,(unsigned int*) &retlenth);	
	cyg_mutex_unlock(&mut_vcom_write);
}

void logonRequest(ICTL_HANDLE_T* ctrlHandle, unsigned int* lenth,char* reqBuf) 
{
	//char reqBuf[MAX_REQUEST_LINE];
	struct requestStatus reply;
	struct logonReq logon;
	int ret;
	
	reply.msg = NULL;
	/* read logon setting */
	if (*lenth > 0) {
		if (USBRead(reqBuf,lenth) == 0)
		{
			USBResetForVCom();
			return;			
		}	
	}
	
	
	xdrlogonReqStru(XDR_DECODE, &logon,reqBuf);
	//Auth the logon request
	if (*logon.name == '\0') 
	{
		// NULL user name
	   	reply.errNum = ERR_INVALID_USER;
	   	reply.msg = NULL;
	   	goto reply;	   		
	   		
	 }
	 if (*logon.passwd == '\0') 
	 {
	 	// NULL user name
	 	reply.errNum = ERR_INVALID_USER;
	 	reply.msg = NULL;
	 	goto reply;	   		
	 		
	 }
	 if (strlen(logon.name) >= 24 || strlen(logon.passwd) >= 24) 
	 {
	   	reply.errNum = ERR_INVALID_USER;
	   	reply.msg = "User name si too long\n";
	   	goto reply;	
	   		
	 }
	   	
	 /* auth the user account */
	ret = ictlAuthUser( ctrlHandle, logon.name, logon.passwd);
	if ( ret != ICTL_OK)
	{
	   	reply.errNum = -1;
	   	if (ret == ICTL_INVALID_PARAMETERS) {
	   		reply.msg = "invalid parameters";	
	   	} else if (ret == ICTL_UNAUTHORIZED) {
	   		reply.msg = "unauthorized users";
	   	}
	   		goto reply;
	} else {
	   		
	   	/* return the privelege level */
	   	reply.errNum = ctrlHandle->Privilege;
	 
	}
reply:
	// At present, we set the powerfullest provilege
	ctrlHandle->Privilege =  AUTH_ADMIN;
	statusReply(&reply);	 
	 
		
	
	
}
void initRequest(ICTL_HANDLE_T* ctrlHandle, unsigned int* lenth, char* reqBuf) 
{
	//char reqBuf[MAX_REQUEST_LINE];
	struct initialInfo info;
	struct requestStatus reply;
	
	/* for auth user */
	char userArray[MAX_ELEMENT][24];
	int privilege[MAX_ELEMENT];
	int actualUserNum;
		
	/* for IP setting */
	unsigned long ipDNS[3];
	char ip_way[20] = {0};
		
	// for wlan
	char pcWlanESSID[sizeof(     ((CAMERA_CONFIG_PARAM_T *)0)->acWlanESSID   )] = {0};
	char handleMethod[10] = {0};
	char acWlanKey[64] = {0};
	char pcWepSet[10] = {0};
	char pcWepAsc[15] = {0};
	char pcWep128[28] = {0};
	char pcWep64[15]={0};
	char pcWep64type[10]= {0};
	char pcWep128type[10] = {0};
	char macAddress[48] = {0};
	
	BOOL bEnable;
	int ret ;
	int tmp;
	int i;
	
	CMD_AUDIO_FORMAT_E audio = CMD_AUDIO_AMR;
	CMD_VIDEO_FORMAT_E vedio = CMD_VIDEO_H263;
	
	/* for reply */
	int retlenth;
	//char replyBuf[MAX_REQUEST_LINE];
	
	
	/*no need to read */
		 
	if ((ret = ictlGetIP( ctrlHandle,ip_way, &(info.ipset.ip),
	   			 &(info.ipset.gateway),
	   			 &(info.ipset.subnetMask), 
	   			 ipDNS,
	   			 3,&tmp)) != ICTL_OK) {
	   		reply.errNum = ERR_NON_PRIVILEGE;
	   		reply.msg = NULL;
	   				 	
	}
	info.ipset.dns1 = ipDNS[0];
	info.ipset.dns2= ipDNS[1];
	info.ipset.dns3 =  ipDNS[2];
	info.ipset.autoGetIP = 0;
	if ((strcmp(ip_way, "manually")) == 0) {
		info.ipset.autoGetIP = 0;
	} else if (strcmp(ip_way, "dchp") == 0) {
		info.ipset.autoGetIP = 1;		
	}	   	
	   	
		
	if ((ictlGetFramerateCapalibity( ctrlHandle, &(info.frame.max), &(info.frame.min) )) != ICTL_OK){
	   		info.frame.step = 1;
	   		reply.errNum = ERR_NO;
	   		reply.msg = NULL;
	   		
	}
	if ((ictlGetFraemrate( ctrlHandle, &(info.frame.current) ) )!= ICTL_OK){
	   		reply.errNum = ERR_NO;
	   		reply.msg = NULL;
	 }
	 
	 // get quality
	if ((ictlGetQualityCapability( ctrlHandle, &(info.quality.min), &(info.quality.max)) )!= ICTL_OK ){
	   		info.quality.step = 2;
	   		reply.errNum = ERR_NO;
	   		reply.msg = NULL;
	   		
	}
	if ((ictlGetQuality( ctrlHandle, &(info.quality.current) ) )!= ICTL_OK){
	   		reply.errNum = ERR_NO;
	   		reply.msg = NULL;
	}
	   	
	// motion detect enble or disable
	if ((ictlIsMotionDetectEnabled( ctrlHandle, &bEnable,NULL,NULL,NULL ) )!= ICTL_OK){
	   		reply.errNum = ERR_NO;
	   		reply.msg = NULL;
	}
	info.motion_detect = bEnable;
	   	
	// get resoluation
	//
	if ((ictlGetResolution( ctrlHandle, &(info.current_resolation) ) )!= ICTL_OK){
	   		reply.errNum = ERR_NO;
	   		reply.msg = NULL;
	}
	   	
	info.resulationList.num = 4;
	info.resulationList.array[0] = "144X176";
	info.resulationList.array[1] = "320X240";
	info.resulationList.array[2] = "352X288";
	info.resulationList.array[3] = "640X480";  	
	   	
	   		
	//int ictlGetResolutionCapability( ICTL_HANDLE_T *pHandle, RES_TYPE **pSize, int *pnSizeTypes )
	// get user
	if ((ictlGetUser( ctrlHandle, (char*)userArray,24, privilege, MAX_ELEMENT,&actualUserNum) )!= ICTL_OK){
	   		reply.errNum = ERR_NO;
	   		reply.msg = NULL;
	}
	// set the reply struct
	info.userList.num = actualUserNum;
	for ( i = 0; i < actualUserNum ;i++){
	   	info.userList.array[i] = userArray[i];
	   	info.userPrivilege[i] = privilege[i];
	   	if (privilege[i] == AUTH_ADMIN) {
	   		info.adminIndex = i;
	   	} 
	   	
	}
	        		
	info.auth_CheckUser = 0;
	if ((ictlGetUserCheck( ctrlHandle, (BOOL *)&(info.auth_CheckUser) ) )){
	   	reply.errNum = ERR_NO;
	   	reply.msg = NULL;
	}
	   	   		   	
	
		
	// get format
	if ((ictlGetMediaFoamat( ctrlHandle,&audio, 
						&vedio) )!= ICTL_OK){
	   		reply.errNum = ERR_NO;
	   		reply.msg = NULL;
	}
	info.audioFormat = audio;
	info.videoFormat = vedio;
	if (ictlGetMac( ctrlHandle,macAddress,48) != ICTL_OK)
	{
	   		reply.errNum = ERR_NO;
	   		reply.msg = NULL;
	} else {
	   		int i;
	   		for (i = 5; i >= 0; --i)
	   		{
	   			httpChar2Hex(macAddress[i], &macAddress[i*3+0]);
	   			if (i != 5)
	   				macAddress[i*3+2] = ':';
	   			else
	   				macAddress[i*3+2] = '\0';
	   		}
			info.mac_address = macAddress;
	}
	// get wlan setting
	info.wlan.handleMethod = handleMethod;
	info.wlan.pcWepSet = pcWepSet;
	//info.wlan.pcWepAsc = pcWepAsc;
	info.wlan.pcWepAsc = acWlanKey;
	info.wlan.pcWep128 = pcWep128;
	info.wlan.pcWep64 = pcWep64;
	info.wlan.pcWlanESSID = pcWlanESSID;
	info.wlan.pcWep64type = pcWep64type;
	info.wlan.pcWep128type = pcWep128type;
	
	if ((ictlGetWlan(ctrlHandle,
				acWlanKey,
				handleMethod,&(info.wlan.ulWlanChannel),
				pcWepSet,pcWepAsc,
				pcWep64type, pcWep128type,
				pcWep64,pcWep128,pcWlanESSID) == ICTL_OK)){
	   		reply.errNum = ERR_NO;
	   		reply.msg = NULL;
	}
	// sent the reply
	
	diag_printf("ulWlanChannel = %d\n"
		"pcWepSet = %s\n"
		"pcWepAsc = %s\n"
		"pcWep64type = %s\n"
		"pcWep128type = %s\n"
		"pcWep64 = %s\n"
		"pcWep128 = %s\n"
		"pcWlanESSID = %s\n",
		info.wlan.ulWlanChannel, pcWepSet,pcWepAsc,
				pcWep64type, pcWep128type,
				pcWep64,pcWep128,pcWlanESSID);
		
	
	retlenth  = xdrinitialInfoLenth(&info);
	xdrinitialInfoStru(XDR_ENCODE, &info,reqBuf + 4);
	reqBuf[2] = VCOM_TOKEN_REPLY;
	reqBuf[3] = 0;
	reqBuf[0] = retlenth & 0xFF;
	reqBuf[1] = (retlenth >> 8 ) & 0xFF;
	retlenth = retlenth + 4;
	cyg_mutex_lock(&mut_vcom_write);
	USBWrite_Again(reqBuf,(unsigned int*) &retlenth);	
	cyg_mutex_unlock(&mut_vcom_write);
	
}


void authRequest(ICTL_HANDLE_T* ctrlHandle, unsigned int* lenth, char* reqBuf) 
{
	struct userSetting user;
	struct requestStatus reply;
	
	reply.msg = NULL;
	reply.errNum = 0;
	
	if (*lenth > 0) {
		if (USBRead(reqBuf,lenth) == 0)
		{
			USBResetForVCom();	
			return;
		}	
	}
	xdrUserSettingStru(XDR_DECODE,&user,reqBuf); 
		
	// type == 1 ,add
	// type ==2 , change
	// type == 3, delete;
	// type ==4 . checkable
   	if (user.type == 1) {
		if((ictlSetUser( ctrlHandle, user.newName, user.newPasswd, user.nPrivilege ) == ICTL_OK)){
	   		reply.errNum = ERR_NO;
	   		reply.msg = NULL;
	   	}
	} else if (user.type == 3) {
	   	if((ictlDelUser( ctrlHandle, user.originalName) == ICTL_OK)){
	   		reply.errNum = ERR_NO;
	   		reply.msg = NULL;
	   		
	   	}
	   	// set check able	
	}else if (user.type == 4){
	   	if(ictlSetUserCheck(ctrlHandle, (BOOL)(user.checkuser)) == ICTL_OK ){
	   		reply.errNum = ERR_NO;
	   		reply.msg = NULL;
	   	}
	   		
	}
	//send the reply
	statusReply(&reply);	   		
}


/* process image setting request */
void imageRequest(ICTL_HANDLE_T* ctrlHandle, unsigned int* lenth, char* reqBuf) 
{
	struct imageSetting image;
	struct requestStatus reply;
	int tmp;
	int ret;
		
	if (*lenth > 0) {
		if (USBRead(reqBuf,lenth) == 0)
		{
			USBResetForVCom();	
			return;
		}	
	}
	reply.msg = NULL;
	reply.errNum = 0;
	
	xdrimageSettingStru(XDR_DECODE,&image, reqBuf) ; 	
	if (image.resolutionX == 176){
		tmp = 0;
	}else if (image.resolutionX == 320) {
	   	tmp = 1;	   	
	}else if(image.resolutionX == 352) {
	   	tmp = 2;
	}else {
	   	tmp = 3;
	}
	if ((ret = ictlSetResolution(ctrlHandle, tmp)) != ICTL_OK) {
	   	reply.errNum = ret;
	   	reply.msg = NULL;		
	}
	if ((ret = ictlSetFramerate(ctrlHandle, image.frameRate)) != ICTL_OK) {
	   	reply.errNum = ret;
	   	reply.msg = NULL;   		
	}
	   	
	if ((ret = ictlSetQuality(ctrlHandle, image.quality)) != ICTL_OK) {
	   	reply.errNum = ret;
	   	reply.msg = NULL;  		
	}
	if ((ret = ictlEnableMotionDetect(ctrlHandle, image.motionDetect,0,0,0)) != ICTL_OK) {
	   	reply.errNum = ret;
	   	reply.msg = NULL; 		
	} 

		   	
	diag_printf("%d %d %d %d %d\n", image.motionDetect,
	   		image.resolutionX,
	   		image.resolutionY,
	   		image.quality,
	   		image.frameRate);
	statusReply(&reply);	   

} /* imageRequest*/

/* process IP setting request */
void usbIPRequest(ICTL_HANDLE_T* ctrlHandle, unsigned int* lenth, char* reqBuf) 
{
 	//char reqBuf[MAX_REQUEST_LINE];
	struct ipSetting ipsetting;
	struct requestStatus reply;
	long ipDNS[3];
	char ip_way[20];
	int ret;
	
	if (*lenth > 0) {
		if (USBRead(reqBuf,lenth) == 0)
		{
			USBResetForVCom();	
			return;
		}	
	}
	
	reply.msg = NULL;
	reply.errNum = 0;
	
 	xdripSettingStru(XDR_DECODE,&ipsetting,reqBuf); 	
   	if (ipsetting.autoGetIP == 1) {
   		strcpy(ip_way,"dhcp");
   	} else {
   		strcpy(ip_way , "manually");	   		
   	}
   	ipDNS[0] = ipsetting.dns1;
   	ipDNS[1] = ipsetting.dns2;
   	ipDNS[2] = ipsetting.dns3;
#if 1	   		   	
	 if ((ret = ictlSetIP(ctrlHandle,ip_way,ipsetting.ip,
	   				ipsetting.gateway, 
	   				ipsetting.subnetMask,
	   				ipDNS,3)) != ICTL_OK) {
	 		reply.errNum = ret;
	   		reply.msg = NULL;  
	   	
	}
	
	// Dynamiclly set IP
	//SetIP();
	wsp_set_network_ip();
	wsp_wait_network_ip(100 * 15);
	
	diag_printf("CA_CONFIG Config_SetIP -- end!\n");
		
#endif	   	
	diag_printf("%ld %ld %ld %ld %ld %ld\n",ipsetting.ip, ipsetting.subnetMask, 
	   			ipsetting.gateway, ipsetting.dns1,
	   			ipsetting.dns2,ipsetting.dns3);
	// send the reply
	
	statusReply(&reply);	   	

}/* usbIPRequest */

void FTPRequest(ICTL_HANDLE_T* ctrlHandle, unsigned int* lenth, char* reqBuf)  
{

   	//char reqBuf[MAX_REQUEST_LINE];
	struct ftpSetting ftp;
	struct requestStatus reply;
	
	if (USBRead(reqBuf,lenth) == 0)
	{
		USBResetForVCom();	
		return;
	}		
	reply.msg = NULL;
	reply.errNum = 0;
   	xdrftpSettingStru(XDR_DECODE , &ftp, reqBuf);
   	diag_printf("%d %ld %s %s %s %s\n", ftp.sendMail,
	   			ftp.ftpServer,
	   			ftp.userName,
	   			ftp.passwd,
	   			ftp.account,
	   			ftp.path);
	//send the reply 
	statusReply(&reply);
}

/* process the mail request */
void mailRequest(ICTL_HANDLE_T* ctrlHandle, unsigned int* lenth,  char* reqBuf)  
{

//   	char reqBuf[MAX_REQUEST_LINE];
	struct emailSetting email;
	struct requestStatus reply;
	if (USBRead(reqBuf,lenth) == 0)
	{
		USBResetForVCom();	
		return;
	}	
	reply.msg = NULL;
	reply.errNum = 0;
   	xdremailSettingStru(XDR_DECODE,&email,reqBuf);
	diag_printf("%d %ld %s %s %s %s %s \n", email.sendMail,
	   		email.mailServer,
	   		email.userName,
	   		email.passwd,
	   		email.sendAddr,
	   		email.recAddr,
	   		email.mailSubject);
	//send the reply 
	statusReply(&reply);
}/* mailRequest */



/* process the wlan request */
void usbWlanRequest(ICTL_HANDLE_T* ctrlHandle, unsigned int* lenth, char* reqBuf)  
{

   //	char reqBuf[MAX_REQUEST_LINE];
	struct wlanSetting wlan;
	struct requestStatus reply;
	int ret;
	
	if (USBRead(reqBuf,lenth) == 0)
	{
		USBResetForVCom();
		return ;
	}	
	reply.msg = NULL;
	reply.errNum = 0;
   	diag_printf("setting wlan request from client\n");
	   	xdrwlanSettingStru(XDR_DECODE,&wlan, reqBuf);
#if 1
	   	if ((ret = ictlSetWlan(ctrlHandle,
	   		NULL,
	   		wlan.handleMethod, 
	   		wlan.ulWlanChannel, wlan.pcWepSet, 
	   		wlan.pcWepAsc,wlan.pcWep64type, 
	   		wlan.pcWep128type,wlan.pcWep64, 
	   		wlan.pcWep128, wlan.pcWlanESSID)) != ICTL_OK) {
	   		reply.errNum = ret;
	   		reply.msg = NULL;
	   	} 
	   	/*set new essid*/
		SetWlan();
#endif
	   	diag_printf("%s %d %s %s %s %s %s %s %s\n", wlan.handleMethod, 
	   		wlan.ulWlanChannel,
	   		wlan.pcWepSet,
	   		wlan.pcWepAsc,
	   		wlan.pcWep64type,
	   		wlan.pcWep128type,
	   		wlan.pcWep64,
	   		wlan.pcWep128,
	   		wlan.pcWlanESSID	   		
	   		);
	   	
	//send the reply 
	statusReply(&reply);
}/* usbWlanRequest */

void netstatRequest(ICTL_HANDLE_T* ctrlHandle, unsigned int* lenth, char* reqBuf)
{
	struct netstatSetting netstat;
	struct requestStatus reply;
	enum SET_WIFI_STATE_E wifiState;
	enum SET_IP_STATE_E ipState;
	unsigned int temp;
	int ret;
	char *buffer;
	
	reply.msg = NULL;
	reply.errNum = 0;

	ret = 8;
	reqBuf[2] = VCOM_TOKEN_REPLY;
	reqBuf[3] = 0;
	reqBuf[0] = ret & 0xFF;
	reqBuf[1] = (ret >> 8 ) & 0xFF;
	
	buffer = reqBuf + 4;

	wsp_get_config_state(&wifiState, &ipState);

	temp = (unsigned int)wifiState;
	buffer = xdrUnsignedInt(XDR_ENCODE,(unsigned char*)buffer,(unsigned int *)&temp);
	
	temp = (unsigned int)ipState;
	buffer = xdrUnsignedInt(XDR_ENCODE,(unsigned char*)buffer,(unsigned int *)&temp);
	
	ret = buffer - reqBuf;
	cyg_mutex_lock(&mut_vcom_write);
	USBWrite_Again(reqBuf,(unsigned int*) &ret);	
	cyg_mutex_unlock(&mut_vcom_write);	
	
}/* netstatRequest */

/* process the custom defined command request */
void usbCustomRequest(ICTL_HANDLE_T* ctrlHandle, unsigned int* lenth, char* reqBuf)  
{

   	//char reqBuf[MAX_REQUEST_LINE];
	struct customSetting custom;
	struct requestStatus reply;
	char singleCmd[MAX_CUSTOM_COMMAND_LINE];
	int ret;
	int index;
	int paraIndex;
	int paraNum;
	char* valueList[MAX_CUSTOM_PARA_PER_CMD];
	char* paraList[MAX_CUSTOM_PARA_PER_CMD];
	if (USBRead(reqBuf,lenth) == 0)
	{
		USBResetForVCom();
		return ;
	}
   	xdrcustomSettingStru(XDR_DECODE,&custom, reqBuf);
	reply.errNum = 0;
	reply.msg = custom.value;
	{
			char* end;
			char* cmd;
			char hold;
			char response[sizeof(MPU_CMD_T) * 2]; 
			if (custom.value == NULL) {
				goto reply;
			}
			
			for (index = 0 ; index < max_custom_command; index++) {
				cmd = strstr(custom.value, cmdHandler[index].cmd);
				if (cmd != NULL) {
					/* separate every cmd */
					end = strstr(cmd + 1, "Cmd");
					if (end != NULL) {
						hold = *end;
						*end = 0;
					}
					strcpy(singleCmd, cmd);					
				//contain no cmd for current index
				} else {
					continue;
				}
				paraNum = cmdHandler[index].paraNum;
				for(paraIndex = 0; paraIndex < paraNum; paraIndex ++) {
					paraList[paraIndex] = cmdHandler[index].paraList[paraIndex];
					
				}
				if (parseCommand(singleCmd, valueList, paraList, paraNum) == -1) {
					//invalid syntax
					continue;
				}
				// call the callbak function
				if ((ret = ((cmdHandler[index]).handler)(ctrlHandle, valueList,paraNum,response, sizeof(response))) == -1) {
					reply.errNum = ret;
					reply.msg = response;
					goto reply;			
				}
				// restore the string
				if (end != NULL) {
					*end = hold;					
				}
	
			
			}
	}
	//send the reply 
reply:
	statusReply(&reply);
}/* usbCustomRequest */

/* process the system request */
void systemRequest(ICTL_HANDLE_T* ctrlHandle, unsigned int* lenth, char* reqBuf)  
{

   	//char reqBuf[MAX_REQUEST_LINE];
	struct systemSetting system;
	struct requestStatus reply;
	char mac[6] = {0};
	reply.msg = NULL;
	if(USBRead(reqBuf,lenth) == 0)
	{
		USBResetForVCom();
		return;
	}	
   	xdrsystemSettingStru(XDR_DECODE,&system, reqBuf);
   	if (system.type == SYSTEM_FORMAT) {
		if ((ictlSetMediaFormat( ctrlHandle,(CMD_AUDIO_FORMAT_E )system.audioFormat, 
						(CMD_VIDEO_FORMAT_E)system.videoFormat) )!= ICTL_OK){
		   	reply.errNum = ERR_NO;
	   		reply.msg = NULL;
		}
		statusReply(&reply);
		cyg_thread_delay(50);
		ictlReboot(ctrlHandle);
		return;
	} else if (system.type == SYSTEM_MAC){
		string_mac_to_long(system.mac_address, mac);
		if (ictlSetMac( ctrlHandle, mac) != ICTL_OK)
		{
	   		reply.errNum = -1;
	   		reply.msg = NULL;
	   		diag_printf("Set mac address %s error\n", system.mac_address);
		} else {
			diag_printf("Set mac address %s\n", system.mac_address);
			reply.errNum = ERR_NO;
	   		reply.msg = NULL;
		}
	} else if (system.type == SYSTEM_DEFAULT_MAC){
		string_mac_to_long(system.mac_address, mac);
		if (ictlSetDefaultMac( ctrlHandle, mac) != ICTL_OK)
		{
	   		reply.errNum = -1;
	   		reply.msg = NULL;
	   		diag_printf("Set default mac address %s error\n", system.mac_address);
		} else {
			diag_printf("Set default mac address %s\n", system.mac_address);
			reply.errNum = ERR_NO;
	   		reply.msg = NULL;
		}		
	} else if(system.type == SYSTEM_DEBUG) {
	 	/* now we can dump the debug info to usb client */
	   	cyg_mutex_lock(&mut_vcom_write);
	   	vcom_console_able = 1;
	   	vcomDump();
	   	cyg_mutex_unlock(&mut_vcom_write);
	   	return;
	} else {
		reply.errNum = -1;
	   	reply.msg = "Invalid request";
		diag_printf("unkown system request from client\n");
	}
	statusReply(&reply);

}/* usbCustomRequest */

// update firmware
void firmwareRequst(ICTL_HANDLE_T* ctrlHandle, unsigned int* lenth, char* reqBuf)
{
	// check whether is admin
	struct requestStatus reply;
	struct firmware firm;
	// read 4 times,and write 1 time to get a good performace
	int filesize;
	int size;
	char header[8]={'W','B',0x5A,0xA5};
	char *tmp;	
	unsigned int pakesize;
	int i = 0;
	int checksum = 0;
	int start = 16;
	char* cur_pos;
	int can_write = 0;
	int size_write;
	
	reply.msg = NULL;
	
	if (USBRead(reqBuf,lenth) == 0)
	{
		USBResetForVCom();
		return ;
	}
	// get the firmware's lenth
	xdrfirmwareStru(XDR_DECODE, &firm, reqBuf);
	filesize = firm.lenth;
	size = filesize + 16;
	//if (0)
	if (!USI_IS_READY)
	{
		diag_printf("No USI Flash!\n");
		reply.msg = "No flash";
		reply.errNum = -1;
		statusReply(&reply);
		return;
	// every thing is ok, tell client to  send data
	} else {
		fmiBuff = (char*)malloc(sizeof(char)*PACKAGE_SIZE * NUM_FOR_WRITE + 1);
		if (fmiBuff == NULL) {
			diag_printf("no enought buffer for programming flash\n");
			reply.errNum = -1;	
			reply.msg = "No enought buff";
			return;			
		}
		diag_printf("begin to write flash!\n");
		reply.errNum = 0;	
		reply.msg = NULL;	
		statusReply(&reply);
	}
	// write the head
	//printf("thanks I will print\n");
	tmp = header;
	memcpy(tmp+4,(UINT8 *)&size,4);
	usiMyWrite(0,8,(UINT8 *)header);
	cur_pos = fmiBuff;
	// after we read NUM_FOR_WRITE packages, then call usiMywrite, since usiMywrite may waste of time 
	while (1) 
	{
		if (filesize > PACKAGE_SIZE)
		{
			pakesize = PACKAGE_SIZE + 1;
			can_write ++;
		} else {
			pakesize = filesize + 1;
			//we set can_write NUM_FOR_WRITE, this told usiMywrite to call, since it is the last package
			can_write = NUM_FOR_WRITE;
		}
		if(USBRead(cur_pos,&pakesize) == 0)
		{
			USBResetForVCom();
			free(fmiBuff);
			return ;
		}
		//check the data's validation
		checksum = 0;
		for ( i = 1; i < pakesize; i++) 
		{
			checksum = checksum ^ (*cur_pos);	
			cur_pos ++;
			
		}
		checksum = checksum ^ (*cur_pos);
		if (checksum != 0) 
		{
			reply.errNum = -1;
			reply.msg = "transfers error";
			diag_printf("transfers error\n");
			goto reply;		
		} else {
			// this is the NUM_FOR_WRITE-th package, or the last package, so write
			if (can_write == NUM_FOR_WRITE){
			
				size_write = cur_pos - fmiBuff;
				//copy it to
				//write it to flash
				//if (1) {
				if(usiMyWrite(start,size_write,(UINT8 *)fmiBuff) == USI_NO_ERR) {
					start += size_write;
					reply.errNum = 0;
					reply.msg = NULL;
					can_write = 0;
					cur_pos = fmiBuff;
					statusReply(&reply);
					// if it is the last pakages
					if (filesize <= PACKAGE_SIZE) 
					{
						free(fmiBuff);
						usiMyFlash();
						return;
					}
					filesize = filesize - (pakesize -1);			
				} else {
					// write error
					reply.errNum = -1;
					reply.msg = "write flash error";
					diag_printf("write to flash error\n");
					goto reply;
				}
			} else {
				//only sent reply to client to tell I only receive data correctly
				reply.errNum = 0;
				reply.msg = NULL;
				//diag_printf("remain size file size %d\n", filesize);
				statusReply(&reply);
				filesize = filesize - (pakesize -1);	
			}			
		}
	}
// not forget to free the buff
reply:
	 free(fmiBuff);
	 statusReply(&reply);
}
// reboot server
void rebootRequst(ICTL_HANDLE_T* ctrlHandle, unsigned int* lenth, char* reqBuf)
{
	struct requestStatus reply;
	reply.msg = NULL;
	// check whether is admin
	// first we should sent the reply. otherwise, client  blocks forever
	statusReply(&reply);
	cyg_thread_delay(50);
	ictlReboot(ctrlHandle);
	
}

void restoreRequst(ICTL_HANDLE_T* ctrlHandle, unsigned int* lenth, char* reqBuf)
{
	// check whether is admin
	// first we should sent the reply. otherwise, client  blocks forever
	struct requestStatus reply;
	reply.msg = NULL;
	reply.errNum = 0;
	//since restore will reboot IPcam, we just send succeful ACK to client 	
	statusReply(&reply);
	// After call this call, the control will not return to this function  
	if (ictlSetFactoryDefault(ctrlHandle) !=  ICTL_OK) 
	{
		diag_printf("set facory default faild\n");	
	} else {
		cyg_thread_delay(50);
		ictlReboot(ctrlHandle);
	}
		
}
//get network information
void getIPInfo(ICTL_HANDLE_T* ctrlHandle, unsigned int* maxsize, char* reqBuf)
{
		/* for IP setting */
	struct ipSetting ipset;
	int lenth = 0;
	unsigned long ipDNS[3];
	char ip_way[20] = {0};
	int ret;
	int tmp;
	
	if ((ret = ictlGetIP( ctrlHandle,ip_way, &(ipset.ip),
	   			 &(ipset.gateway),
	   			 &(ipset.subnetMask), 
	   			 ipDNS,
	   			 3,&tmp)) != ICTL_OK) {	   				 	
	}
	ipset.dns1 = ipDNS[0];
	ipset.dns2= ipDNS[1];
	ipset.dns3 =  ipDNS[2];
	ipset.autoGetIP = 0;
	if ((strcmp(ip_way, "manually")) == 0) {
		ipset.autoGetIP = 0;
	} else if (strcmp(ip_way, "dchp") == 0) {
		ipset.autoGetIP = 1;		
	}
	//send the reply
	lenth = xdripSettingLenth(&ipset);
	xdripSettingStru(XDR_ENCODE, &ipset,reqBuf + 4);
	reqBuf[2] = VCOM_TOKEN_REPLY;
	reqBuf[3] = 0;
	reqBuf[0] = lenth & 0xFF;
	reqBuf[1] = (lenth >> 8 ) & 0xFF;
	lenth = lenth + 4;
	cyg_mutex_lock(&mut_vcom_write);
	USBWrite_Again(reqBuf,(unsigned int*) &lenth);	
	cyg_mutex_unlock(&mut_vcom_write);		
	
}

void freshWlanRequest(ICTL_HANDLE_T* ctrlHandle, unsigned int* reqlenth, char* reqBuf)
{
	ApScanResult ScanAp[32];
	int lenth = 0;
	int num = 0;
    int i = 0;
	struct wlanAPArray apReply;
	struct wlanAPEntry apArray[32];
	apReply.array = apArray;
	// get wireless ap
    if (ictlScanAP(ctrlHandle,ScanAp,&num,32) == ICTL_OK) { // get ap information
      	for(i =0; i<num; i++) {
      		apArray[i].essid = ScanAp[i].ssid;
           	apArray[i].signal = ScanAp[i].signal;
	        apArray[i].noise = ScanAp[i].noise;
            apArray[i].mode = ScanAp[i].mode;
            apArray[i].encode = ScanAp[i].encode;
       	 #if 0
       		diag_printf("ESSID = %s \n", ScanAp[i].ssid);
           	diag_printf("mode = %d \n", ScanAp[i].mode);
           	if(ScanAp[i].mode == 2) {
           		diag_printf("infrastructure  \n");
           	} else if(ScanAp[i].mode == 1) {
               	diag_printf("AD-HOC  \n");
	        }
      		diag_printf("quality level = %d, noise = %d  \n",ScanAp[i].signal, ScanAp[i].noise);
            diag_printf(" \n\n" );
       	#endif
       	}
       	
#if 1
	/* Add a blank element for rovio client bug */
	if (num + 1 < sizeof(apArray) / sizeof(apArray[0]))
	{
		apArray[num].essid = "NULL";
		apArray[num].signal = 1;
		apArray[num].noise = 1;
		apArray[num].mode = 1;
		apArray[num].encode = 0;
		num++;
	}
	
#endif
       	
       	
    apReply.num = num;                     
	
	// get ap error
	} else {
		apReply.num = -1;
	}
	
	//send the reply
	lenth = xdrwlanAPArrayLenth(&apReply);
		
	xdrwlanAPArrayStru(XDR_ENCODE, &apReply,reqBuf + 4);
	reqBuf[2] = VCOM_TOKEN_REPLY;
	reqBuf[3] = 0;
	reqBuf[0] = lenth & 0xFF;
	reqBuf[1] = (lenth >> 8 ) & 0xFF;
	lenth = lenth + 4;
	
	assert(MAX_REQUEST_LINE >= lenth);
	
	cyg_mutex_lock(&mut_vcom_write);
	USBWrite_Again(reqBuf,(unsigned int*) &lenth);	
	cyg_mutex_unlock(&mut_vcom_write);	

}


//get PPP information
void vcom_getPPP(ICTL_HANDLE_T* ctrlHandle, unsigned int* maxsize, char* reqBuf)
{
	char *pBuf;
	int length;
	int nPPPEnable;
	tt_rmutex_lock(&g_rmutex);
	//send the reply
	length = 4	//Enable
		+ sizeof(g_ConfigParam.aacPPPServer)
		+ sizeof(g_ConfigParam.aiPPPPort)
		+ sizeof(g_ConfigParam.acPPPUser)
		+ sizeof(g_ConfigParam.acPPPPass);

	reqBuf[2] = VCOM_TOKEN_REPLY;
	reqBuf[3] = 0;
	reqBuf[0] = length & 0xFF;
	reqBuf[1] = (length >> 8 ) & 0xFF;
	length = length + 4;

	pBuf = reqBuf + 4;
	nPPPEnable = (int)g_ConfigParam.bPPPEnable;
	xdrInt(XDR_ENCODE, pBuf, &nPPPEnable);
	pBuf += 4;
	memcpy(pBuf, &g_ConfigParam.aacPPPServer, sizeof(g_ConfigParam.aacPPPServer));
	pBuf += sizeof(g_ConfigParam.aacPPPServer);
	memcpy(pBuf, &g_ConfigParam.aiPPPPort, sizeof(g_ConfigParam.aiPPPPort));
	pBuf += sizeof(g_ConfigParam.aiPPPPort);
	memcpy(pBuf, &g_ConfigParam.acPPPUser, sizeof(g_ConfigParam.acPPPUser));
	pBuf += sizeof(g_ConfigParam.acPPPUser);
	memcpy(pBuf, &g_ConfigParam.acPPPPass, sizeof(g_ConfigParam.acPPPPass));
	pBuf += sizeof(g_ConfigParam.acPPPPass);
	tt_rmutex_unlock(&g_rmutex);
			
	
	cyg_mutex_lock(&mut_vcom_write);
	USBWrite_Again(reqBuf,(unsigned int*) &length);	
	cyg_mutex_unlock(&mut_vcom_write);
}


void vcom_setPPP(ICTL_HANDLE_T* ctrlHandle, unsigned int* lenth, char* reqBuf)  
{
	int i;
	BOOL bConfigChanged = FALSE;
	int nPPPEnable;
	BOOL bPPPEnable;
	const char *apcPPPServer[ sizeof(g_ConfigParam.aacPPPServer) / sizeof(g_ConfigParam.aacPPPServer[0])];
	int aiPPPPort[ sizeof(g_ConfigParam.aiPPPPort) / sizeof(g_ConfigParam.aiPPPPort[0])];
	const char *pcPPPUser;
	const char *pcPPPPass;
	
	struct requestStatus reply;
	
	if (USBRead(reqBuf,lenth) == 0)
	{
		USBResetForVCom();	
		return;
	}
	
	xdrInt(XDR_DECODE, reqBuf, &nPPPEnable);
	reqBuf += 4;
	bPPPEnable = (BOOL)nPPPEnable;
	if (bPPPEnable != g_ConfigParam.bPPPEnable)
		bConfigChanged = TRUE;
	
	for (i = 0; i < sizeof(g_ConfigParam.aacPPPServer) / sizeof(g_ConfigParam.aacPPPServer[0]); ++i)
	{
		apcPPPServer[i] = reqBuf;
		reqBuf += sizeof(g_ConfigParam.aacPPPServer[0]);
		
		if (strcmp(apcPPPServer[i], g_ConfigParam.aacPPPServer[i]) != 0)
			bConfigChanged = TRUE;
	}
	for (i = 0; i < sizeof(g_ConfigParam.aiPPPPort) / sizeof(g_ConfigParam.aiPPPPort[0]); ++i)
	{
		memcpy(&aiPPPPort[i], reqBuf, sizeof(g_ConfigParam.aiPPPPort[0]));
		reqBuf += sizeof(g_ConfigParam.aiPPPPort[0]);
		
		if (aiPPPPort[i] != g_ConfigParam.aiPPPPort[i])
			bConfigChanged = TRUE;
	}

	/* User */
	pcPPPUser = reqBuf;
	reqBuf += sizeof(g_ConfigParam.acPPPUser);
	if (strcmp(pcPPPUser, g_ConfigParam.acPPPUser) != 0)
		bConfigChanged = TRUE;
	
	/* Pass */
	pcPPPPass = reqBuf;
	reqBuf += sizeof(g_ConfigParam.acPPPPass);
	if (strcmp(pcPPPPass, g_ConfigParam.acPPPPass) != 0)
		bConfigChanged = TRUE;	

	if (bConfigChanged)
	{
		tt_rmutex_lock(&g_rmutex);
		g_ConfigParam.bPPPEnable = bPPPEnable;
			
		for (i = 0; i < sizeof(g_ConfigParam.aacPPPServer) / sizeof(g_ConfigParam.aacPPPServer[0]); ++i)
			httpMyStrncpy(g_ConfigParam.aacPPPServer[i], apcPPPServer[i], sizeof(g_ConfigParam.aacPPPServer[i]));
		for (i = 0; i < sizeof(g_ConfigParam.aiPPPPort) / sizeof(g_ConfigParam.aiPPPPort[0]); ++i)
			g_ConfigParam.aiPPPPort[i] = aiPPPPort[i];
		httpMyStrncpy(g_ConfigParam.acPPPUser, pcPPPUser, sizeof(g_ConfigParam.acPPPUser));
		httpMyStrncpy(g_ConfigParam.acPPPPass, pcPPPPass, sizeof(g_ConfigParam.acPPPPass));
		tt_rmutex_unlock(&g_rmutex);
		
		WriteFlashMemory(&g_ConfigParam);
	}
		
	if (!bPPPEnable)
	{
		/* Disconnect if possible */
		//if (ppot_is_connecting())
		ppot_disconnect();
	}
		
	if(bPPPEnable && bConfigChanged) //&& !ppot_is_connecting())
	{
		ppot_connect(apcPPPServer, aiPPPPort,
			sizeof(apcPPPServer) / sizeof(apcPPPServer[0]),
			pcPPPUser, pcPPPPass);
	}

	
	reply.msg = NULL;
	reply.errNum = 0;
	statusReply(&reply);
}



/* decode the req string and call system call to handle the request and sent the result */

void processClient(void )
{
	// record user control's context
	
	static ICTL_HANDLE_T ctrlHandle;
	/* buff for store request stucture */
	char reqBuf[MAX_REQUEST_LINE];
	//char replyBuf[MAX_REQUEST_LINE];
	//char reqBuf[4];
	//cyg_priority_t old;
	

	UINT32  headerLen = 4;
	UINT32 lenth;
	int opcode;
	
	struct requestStatus reply;
	/* read request from client */
	if (USBRead(reqBuf, &headerLen) == 0)
	{
		diag_printf("a error ocurred for USBRead() \n");
		//empty current buff
		USBResetForVCom();	
		return;
	
	}
	opcode = ((int)reqBuf[3] << 8) + (int)reqBuf[2];
	lenth = ((int)reqBuf[1] << 8) + (int)reqBuf[0];
	
	g_bIsMac = ((opcode & (int)0x0100) == 0 ? FALSE : TRUE);
	opcode &= ~(int)0x0100;
	
	if (opcode < LOGON && opcode > USB_CLOSE) 
	{
		diag_printf("Unkowned request from client \n");
		//empty current buff
		USBResetForVCom();	
		return;
	}
	// for other request, we have limitation for request lenth
	if (opcode != FIRMWARE) {			
		if (/*lenth < 0 || */lenth > MAX_REQUEST_LINE)
			return ;
	}
	// read the request struct
	//USBRead(reqBuf,&lenth);	
	switch ((enum CTRL_TYPE)opcode) 
	{
	   case LOGON:
	   	/* handle user logon request and initial the user hander */
	   	logonRequest(&ctrlHandle,&lenth, reqBuf);
	   	diag_printf("LOGON request from client\n");
	   	break;
	   	
	   case INIT:  
 	   	diag_printf("Init request from client\n");
 	   	initRequest(&ctrlHandle,&lenth, reqBuf);
	   	break;
	   
	   case AUTH:
	   	diag_printf("Authority request from client\n");
	   	authRequest(&ctrlHandle, &lenth, reqBuf);
	   	break;
	   case IMAGE:
	   	diag_printf("Image request from client\n");
		imageRequest(&ctrlHandle, &lenth, reqBuf);
	   	break;
	   
	   case USB_IP:
	   	diag_printf("IP request from client\n");
		usbIPRequest(&ctrlHandle, &lenth, reqBuf);	   	
	   	break;
	   case FTP:
	   	diag_printf("FTP request from client\n");
	   	FTPRequest(&ctrlHandle, &lenth, reqBuf);  	
	   	break;
	   case MAIL:
	   	diag_printf("MAIL request from client\n");
		mailRequest(&ctrlHandle, &lenth, reqBuf);
	   	break;
	   case FIRMWARE:
	   	diag_printf("FIRMWARE request from client\n");
	   	// promote the priority
	   //	old = cyg_thread_get_current_priority(vcom_thread_handle);
	   // 	cyg_thread_set_priority(vcom_thread_handle, old - 4);
	   	firmwareRequst(&ctrlHandle, &lenth, reqBuf);
	   	//after finish update firmware, low it
	   //	cyg_thread_set_priority(vcom_thread_handle, old);
	   	break;
	   case EXTEND:
	   	diag_printf("EXTEND request from client\n");
	   	break;
	   case REBOOT:
	   	diag_printf("REBOOT request from client\n");
	   	rebootRequst(&ctrlHandle, &lenth, reqBuf);
	   	break;
	   case SYSTEM:
	   	//diag_printf("SYSTEM request from client\n");
	   	systemRequest(&ctrlHandle, &lenth, reqBuf);
	   	break;
	   case RESTORE:
	   	diag_printf("RESTORE request from client\n");
	   	restoreRequst(&ctrlHandle, &lenth, reqBuf);
	   	break;
	   case GET_PPPOE_STATUS:
	   	diag_printf("GET_PPPOE_STATUS request from client\n");
	   	break;
	   case USB_WLAN:
	   	diag_printf("setting wlan request from client\n");
		usbWlanRequest(&ctrlHandle, &lenth, reqBuf);
	   	break;
	   	
	   case USB_CUSTOM:
		diag_printf("custom setting request from client\n");
		usbCustomRequest(&ctrlHandle, &lenth, reqBuf);
	   	break;
	   case FRESH_WLAN:
	    	diag_printf("scan wireless network request from clinet");
	    	freshWlanRequest(&ctrlHandle, &lenth, reqBuf);
	    	break;
	   case USB_GET_IP:
	   		diag_printf("get network information from client\n");
	   		getIPInfo(&ctrlHandle, &lenth, reqBuf);
	   		break;
	   case USB_CLOSE:
	   {
			reply.errNum = 0;
	   		reply.msg = NULL;
			lenth = xdrrequestStatusLenth(&reply);
			reqBuf[2] = VCOM_TOKEN_REPLY;
			reqBuf[3] = 0;
			reqBuf[0] = lenth & 0xFF;
			reqBuf[1] = (lenth >> 8)&0xFF;
			xdrrequestStatusStru(XDR_ENCODE, &reply,&reqBuf[4]);
			lenth = lenth + 4;
			cyg_mutex_lock(&mut_vcom_write);
			USBWrite_Again(reqBuf,(unsigned int*) &lenth);
			vcom_console_able = 0;
			cyg_mutex_unlock(&mut_vcom_write);
	   	
	   }	   	
	   	diag_printf("Close request from client\n");	 
	   	break;
	   case NETWORK_STATE:
		diag_printf("custom setting request from client\n");
		netstatRequest(&ctrlHandle, &lenth, reqBuf);
	   	break;
	   case USB_GET_PPP:
		diag_printf("custom setting request from client\n");
		vcom_getPPP(&ctrlHandle, &lenth, reqBuf);
	   	break;
	   case USB_SET_PPP:
		diag_printf("custom setting request from client\n");
		vcom_setPPP(&ctrlHandle, &lenth, reqBuf);
	   	break;
	   

	}
 	
} /* usbProcessClient*/



/* parse the command line to a string array, 
 * cmdLine [in]: the cmdline to be parseed
 * args[out]: return the parametres value array indicated by cmdLine
 * token[in]: parametres name
 * return: -1: error, otherwise successully  
*/
int parseCommand(char* cmdLine, char** args, char** token, int num) 
{
	char tmp[100];
	char* p;
	char* value;
	char* end;
	char hold;
	int i = 0;
	char *head;
	//int tmpIndex;
	strcpy(tmp, cmdLine);
	for (i = 0; i < num ; i++) {
		if ((p = strstr(tmp, token[i])) == NULL) {
			args[i] = NULL;
			
		} else {
			//determin whether tmp is only a head of token[i]
			// such as tmp = "headbody", token[i] = "head"
			// 
			head = p;
			head = head + strlen(token[i]);
			while(*head != 0 && *head != '='&& *head == ' ') {
				head ++;
			}
			if (*head != '=') {
				args[i] = 0;
				continue;
			}			
		// fetch the value, which stored in the cmdline
			//example "cmd=LED&which=10&color=yellow"
			// if token[i] = "which" 
			//p ->which
			//value -> 10;
			
			end = strchr(p, '&');
			//terminal current key-value pair
			if (end != NULL) {
				hold = *end;
				*end = '\0';
				//replace '&' with '\0', and to terminate the value;
				*(cmdLine + (end - tmp)) = 0;

			}
			value = strchr(p,'=');
			if(value == NULL) {
				//error format
				return -1;
			}
			//ignore the '='
			value ++;
			args[i] = value - tmp + cmdLine;
			// restore the 
			if ( end != NULL) {
				*end = hold;
			}
						
		}
	}
	return 0;	

}

/* dump string to vcom buff, the string will be send when client ready to read */
// buf[0-1]: lenth
// buf[2-3]: type
void vcom_write(char *buf, size_t size )
{
	
	unsigned int lenth = 4;
	unsigned int header;
	int i = 0;
	
	if(!g_vcom_init)
		return;

	// if this is called by write_char(), buff the data, until encounter a '\n' or '\r'
	cyg_mutex_lock(&mut_vcom_write);
#if 1
	/*write to buff */
	for (i = 0; i < size; i++)
	{
		int dumpTail_Next = NEXTONE(dumpTail);
		if(dumpTail_Next != dumpHead)
		{
			dumpBuff[dumpTail] = buf[i];
			dumpTail = dumpTail_Next;
		}
	}
#else	
	if (size == 1) {
		
		if(dumpTail_Next != dumpHead)
		{
			dumpBuff[dumpTail] = *buf;
			dumpTail = dumpTail_Next;
			if (*buf == '\n') {
				if (vcom_console_able == 1) {	
					vcomDump();	
				}
			}	
		}
	} else {
		//encode the header
		header = VCOM_TOKEN_MSG;
		header = header << 16;
		header = header + size;	
		if (vcom_console_able == 1) {
			if (USBWrite((char *)&header, &lenth) < 0)
			{
				USBResetForVCom();
				goto out;
			}
			lenth = size;
			if (USBWrite(buf, &lenth) < 0)
			{
				USBResetForVCom();
				goto out;
			}
		} else {
		/*write to buff */
			for (i = 0; i < size; i++) {
				dumpBuff[dumpTail] = buf[i];
				dumpTail = NEXTONE(dumpTail);
			}
			
		}
	}

out:
#endif
	cyg_mutex_unlock(&mut_vcom_write);
}/* vcom_write */



/* return the total size of usb printf */
int vcomDumpSize()
{
	int size = dumpTail - dumpHead;
	if (size < 0) 
		return (size + MAX_DEBUG_SIZE);
	else 
		return size;	
}
/* since client is ready for receive msg, dump it , before call this function, we should 
 * cyg_mutex_lock(&mut_vcom_write);*/
void vcomDump(void ) 
{
	unsigned int size;
	unsigned int headlen = 4;
	unsigned int header;
	size = vcomDumpSize();
	header = VCOM_TOKEN_MSG;
	header = header << 16;
	header = header + size;	
	if (USBWrite((char *)&header,&headlen) < 0)
	{
		USBResetForVCom();
		return;
	}

	if (dumpTail == dumpHead) {
			return;
	} else if (dumpTail > dumpHead) {
			size = dumpTail - dumpHead;
			if (USBWrite(&dumpBuff[dumpHead], &size) < 0)
			{
				USBResetForVCom();
				return;
			}
	} else {
			size = MAX_DEBUG_SIZE - dumpHead;
			USBWrite(&dumpBuff[dumpHead], &size);
			size = dumpTail;
			if (USBWrite(&dumpBuff[0], &size) < 0)
			{
				USBResetForVCom();
				return;
			}
	}
	dumpHead = dumpTail;
}

/* this function register a cmd formate, with the formate we can parese the cmd's parameters
 * the parameter name list'order decide the value list's order  
 * parameter:
 * 		cmdName[in]: command's name
 * 		paralist[in]: the parameter name list
 *		paraNum[in]: the element number in list
 *      pf[in]: the callback function
 * return:
 *      if -1, indicate a error such as too many parameters
 *	
 */
int registerCustomCmd(char* cmdName, char** paralist, int paraNum, customHandler pf) 
{
	int i = 0, index;
	if (paraNum > MAX_CUSTOM_PARA_PER_CMD) {
		printf("register a command with too mang parameters\n");
		return -1;
	}
	for (i = 0; i < max_custom_command && i < MAX_CUSTOM_COMMAND; i++ ) {
		if (strncmp(cmdHandler[i].cmd, cmdName, MAX_CUSTOM_COMMAND_LEN) == 0) {
			//over write provios's handle
			for(index = 0; index < paraNum; index ++) {
				strncpy(cmdHandler[i].paraList[index],paralist[index], MAX_CUSTOM_PARA_NAME_LEN);
				cmdHandler[i].paraList[index][MAX_CUSTOM_PARA_NAME_LEN] = 0; 
				
			}
			cmdHandler[i].handler = pf; 
			cmdHandler[i].paraNum = paraNum;
			return 0;
			
			
		 
		}
	}
	/* too many command */
	if (i == MAX_CUSTOM_COMMAND) {
		diag_printf("Too many USB vcom custom command\n");
		cyg_interrupt_disable(); while(1);	//Block the system
		return -1;
	}
	/* add the cmd handle in the end */
	strncpy(cmdHandler[i].cmd, cmdName, MAX_CUSTOM_COMMAND_LEN);
	/* termintal the cmd string */
	cmdHandler[i].cmd[MAX_CUSTOM_COMMAND_LEN] = 0;
	
	for(index = 0; index < paraNum; index ++) {
		strncpy(cmdHandler[i].paraList[index],paralist[index], MAX_CUSTOM_PARA_NAME_LEN);
		cmdHandler[i].paraList[index][MAX_CUSTOM_PARA_NAME_LEN] = 0; 	
	}
	cmdHandler[i].handler = pf; 
	cmdHandler[i].paraNum = paraNum;
	max_custom_command ++ ;
	return 0;
	
}
