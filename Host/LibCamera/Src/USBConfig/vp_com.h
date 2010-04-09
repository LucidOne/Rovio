#if !defined VCOM_H_
#define VCOM_H_
#include "../Inc/CommonDef.h"

#define MAX_REQUEST_LINE 4096
#define MAX_FIELD  256

#define	XDR_ENCODE  0
#define XDR_DECODE  1
#define MAX_ELEMENT 30

#define ERR_INVALID_USER 	-1
#define ERR_NON_PRIVILEGE	-2

#define ERR_NO		  0

#define CST_TYPE_INT 0
#define CST_TYPE_STRING 1

//indicate whether client is cmd or gui
#define CLIENT_MODE_CUI		1
#define CLIENT_MODE_GUI		2

typedef int (* customHandler)(ICTL_HANDLE_T* ctrlHandle, char** paraList, int paraNum, char *pcResponse,unsigned int szMaxResponse);    
enum CTRL_TYPE {
	   LOGON =1,
	   INIT, // get all the information to initialize the Dialog;	
	   AUTH,
	   IMAGE,
	   USB_IP,
	   FTP,
	   MAIL,
	   FIRMWARE,
	   EXTEND,
	   REBOOT,
	   SYSTEM,
	   RESTORE,
	   GET_PPPOE_STATUS, // this need modify
	   USB_WLAN,
	   USB_GET_IP,
	   USB_CUSTOM,
	   DATA,
	   FRESH_WLAN,
	   USB_CLOSE,
	   VERSION,
	   NETWORK_STATE,
	   USB_GET_PPP,
	   USB_SET_PPP
};

enum SYSTEM_SET_TYPE
{
	SYSTEM_FORMAT = 1,
	SYSTEM_MAC,
	SYSTEM_DEBUG,
	SYSTEM_DEFAULT_MAC
};


struct logonReq {
	char* name;
	char* passwd;
};

struct requestStatus {
	int errNum;
	char* msg;
};

struct imageSetting{
	int motionDetect;
	int resolutionX;
	int resolutionY;
	int quality;
	int frameRate;

};

struct userSetting{
	int checkuser;
	int type;//add / change/ delete
	int nPrivilege;
	char* originalName;
	char* originalPasswd;
	char* newName;
	char* newPasswd;
};

//get a user list;


struct emailSetting{
	int sendMail; //indicate if send a email; 
	unsigned long  mailServer; // the ip of mail server;
	char* userName;
	char* passwd;
	char* sendAddr; // the sender's mail address;
	char* recAddr;
	char* mailSubject;

};
struct ftpSetting{
	int sendMail; //indicate if send a file to ftp server; 
	unsigned long ftpServer;
	char* userName;
	char* passwd;
	char* account;
	char* path; // the path wherre file be upload
};

struct ipSetting {
	int autoGetIP;
	unsigned long ip;
	unsigned long subnetMask;
	unsigned long gateway;
	unsigned long dns1;
	unsigned long dns2;
	unsigned long dns3;
	// this is for PPPOE, if set autoGetIP true;
#if 0
	char* userName;
	char* passwd;
	int dialOnBoot;
	int emailAfterDial;
	char* mailServer; //the ip for mail server;
	char* userOnMailServer;//the account to logon mail server
	bool needPasswd;
	char* mail_passwd;
	char* senderAddr;
	char* revAddr;
	char* subject;
#endif
} ;
// this command will return a struct to client;



struct extendSetting {
	int autoGetTime;
	unsigned long timeServer; // the time server's ip
	char* date;
	char* time;
	char* timeZone;
};


struct systemSetting {
	// 0:mpeg4 / 1: h263
	int type;
	int videoFormat; 
	int audioFormat;
	int debug;
	char* mac_address;
};

struct pointerArray {
	int num;
	char* array[MAX_ELEMENT];
};
struct Iter{
	int min;
	int max;
	int step;
	int current;
};


struct wlanSetting {
	char* handleMethod;
	unsigned long ulWlanChannel;
	char *pcWepSet; // disable .k64, k128
	char *pcWepAsc; 
	char *pcWep128; // 128 asicii, 128 hex value
	char *pcWep64; //64 asicii 64 hex vlaue
	char *pcWep64type;
	char *pcWep128type;
	char *pcWlanESSID;
	
};


struct netstatSetting {
	int wifiState;
	int ipState;
};


struct initialInfo {
	struct pointerArray resulationList; //image
	struct pointerArray userList; //user page
	int userPrivilege[MAX_ELEMENT];
	int adminIndex;
	struct Iter frame;
	struct Iter quality;
	
	
	struct ipSetting ipset; // ip setting
	struct wlanSetting wlan;

	//struct emailSetting mail; // email 

	//struct ftpSetting ftp; //ftp page

	int auth_CheckUser; // user page
	int current_resolation;//image
	int motion_detect;

	int audioFormat;
	int videoFormat;
	char* mac_address;
	
	// extend page
	//char* serverVersion;
	//char* serverTime;
	//int extend_GetFromServer;
	//int extend_GetFromLocal;
	//char* date;
	//char* time;
	//char* timeZoom;

	//int formate; // system page
};
struct customSetting {
	char* value;
};

struct firmware {
	unsigned int lenth;
};

struct wlanAPEntry 
{
	char* essid;
	int mode;
	int signal;
	int noise;
	int encode;
};

struct wlanAPArray 
{
	int num;
	struct wlanAPEntry* array;
};

extern void VirtualComInit(void);
extern int registerCustomCmd(char* cmdName, char** paralist, int paraNum, customHandler pf);

#endif
