#ifndef WEBCAMERA_LOG_H
#define WEBCAMERA_LOG_H

#define CL_PRIVILEGE_ADMIN 0	//超级用户才能看到的信息
#define CL_PRIVILEGE_COMMON 1	//任何用户能看到的信息


#define CL_INFO 0
#define CL_ERR 1

#define CL_SET_USER 11
#define CL_DEL_USER 12
#define CL_SET_USER_CHECK 13

#define CL_OPEN_CAMERA 14
#define CL_CLOSE_CAMERA 15

#define CL_CHANGE_RESOLUTION 16
#define CL_CHANGE_QUALITY 17
#define CL_CHANGE_BRIGHTNESS 18
#define CL_CHANGE_CONTRAST 19
#define CL_CHANGE_SATURATION 20
#define CL_CHANGE_HUE 21
#define CL_CHANGE_SHARPNESS 22


#define CL_SET_MAIL 23
#define CL_SET_FTP 24

#define CL_PPPOE_DIAL 25
#define CL_MODEM_DIAL 26

#define CL_NEW_CLIENT 27

#define CL_SET_MOTION_DETECT 28
#define CL_SET_MONITE_RECT 29

#define CL_SET_TIME 30

#define CL_SET_IP 31
#define CL_SET_HTTPPORT 32

#define CL_SET_NAME 33

#define CL_SET_WLAN 34

#define CL_CHANGE_FRAMERATE 35
#define CL_CHANGE_DIRECTION 36

#define CL_CHANGE_SPEAKERVOLUME 37
#define CL_CHANGE_MICVOLUME 38
#define CL_CHANGE_SHOWTIME 39
#define CL_CHANGE_LOGO 40

#define CL_CHANGE_CAMERA 41

#define CL_SET_VNET 42

/* do with the http request "/GetLog.cgi" */
int Config_GetLog(HTTPCONNECTION hConnection, LIST *pParamList, int iAction, XML *pReturnXML);

/* add log */
void WebCameraLog(int iViewerPrivilege,
					int iLogType,
					const char *pcLogValue,
					HTTPCONNECTION hConnection);


int OnRequestBegin(HTTPCONNECTION hConnection, void *pParam);
#endif
