#ifndef CONVERT_H
#define CONVERT_H


/* Constant definition. */

#ifdef CONVERT_C
#define G_CONST(varName, varValue) const char *varName = varValue
#else
#define G_CONST(varName, varValue) extern const char *varName
#endif


G_CONST(G_PC_NO_CACHE_HEADER, "Pragma: no-cache\r\nCache-Control: no-cache\r\nExpires: 01 Jan 1970 00:00:00 GMT\r\n");
G_CONST(G_PC_SEC1970, "Sec1970");
G_CONST(G_PC_TIMEZONE, "TimeZone");
G_CONST(G_PC_NTPSERVER, "NtpServer");
G_CONST(G_PC_USENTP, "UseNtp");
G_CONST(G_PC_SHOWTIME, "ShowTime");
G_CONST(G_PC_SHOWSTRING,"ShowString");
G_CONST(G_PC_SHOWPOS,"ShowPos");
G_CONST(G_PC_MAC,"MAC");

G_CONST(G_PC_ESSID, "ESSID");
G_CONST(G_PC_CHANNEL, "Channel");
G_CONST(G_PC_MODE, "Mode");
G_CONST(G_PC_WEPSET, "WepSet");
G_CONST(G_PC_WEPASC, "WepAsc");
G_CONST(G_PC_WEPGROUP, "WepGroup");
G_CONST(G_PC_WEP64, "Wep64");
G_CONST(G_PC_WEP64_TYPE, "Wep64type");
G_CONST(G_PC_WEP128, "Wep128");
G_CONST(G_PC_WEP128_TYPE, "Wep128type");
G_CONST(G_PC_KEY, "Key");

G_CONST(G_PC_CAMERANAME, "CameraName");
G_CONST(G_PC_DNS_N, "DNS%d");
G_CONST(G_PC_INTERFACE, "Interface");
G_CONST(G_PC_ENABLE, "Enable");
G_CONST(G_PC_IPWAY, "IPWay");
G_CONST(G_PC_IP, "IP");
G_CONST(G_PC_NETMASK, "Netmask");
G_CONST(G_PC_GATEWAY, "Gateway");

G_CONST(G_PC_PORT_N, "Port%d");

G_CONST(G_PC_TOTAL, "Total");
G_CONST(G_PC_FINISH, "Finish");

G_CONST(G_PC_SENSITIVITY, "Sensitivity");

G_CONST(G_PC_FTPSERVER, "FtpServer");
G_CONST(G_PC_USER, "User");
G_CONST(G_PC_PASS, "Pass");
G_CONST(G_PC_ACCOUNT, "Account");
G_CONST(G_PC_UPLOADPATH, "UploadPath");

G_CONST(G_PC_DOMAINNAME, "DomainName");
G_CONST(G_PC_PROXY, "Proxy");
G_CONST(G_PC_PROXYUSER, "ProxyUser");
G_CONST(G_PC_PROXYPASS, "ProxyPass");
G_CONST(G_PC_INFO, "Info");

G_CONST(G_PC_MAIL, "Mail");
G_CONST(G_PC_MAILSERVER, "MailServer");
G_CONST(G_PC_SENDER, "Sender");
G_CONST(G_PC_RECEIVER, "Receiver");
G_CONST(G_PC_SUBJECT, "Subject");
G_CONST(G_PC_CHECKFLAG, "CheckFlag");
G_CONST(G_PC_MAILUSER, "MailUser");
G_CONST(G_PC_MAILPASSWORD, "MailPassWord");

G_CONST(G_PC_ACTION, "Action");
G_CONST(G_PC_PHONE, "Phone");
G_CONST(G_PC_CONNECT_ON_BOOT, "ConnectOnBoot");
G_CONST(G_PC_SHOWPRIVILEGE, "ShowPrivilege");

G_CONST(G_PC_AUDIO, "Audio");
G_CONST(G_PC_VIDEO, "Video");


G_CONST(G_PC_RT, "Return");

#endif
