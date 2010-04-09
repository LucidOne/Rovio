#include "../../Inc/CommonDef.h"
#ifndef WLAN
__align (32)
CHAR g_Mctest_Buf[MCTEST_BUFFER_LEN];
#endif
volatile BOOL g_endflag;
struct dev_addr ipmc[MAXINTERFACES];

void CreateAndSendPackage(int i1, unsigned long addr, char *portstr)
{
    char packbuf[100];
    char CameraName[NAMELEN]={'D','S','C','1'};
    memset(packbuf, 0, 100);

    memcpy(packbuf, CAMFLAG, CAMFLAGLEN);
    packbuf[CAMFLAGLEN] = 0;
    memcpy(&packbuf[CAMFLAGLEN+1], ipmc[i1].name, 4);
    memcpy(&packbuf[CAMFLAGLEN+5], ipmc[i1].ipaddr, 4);
    memcpy(&packbuf[CAMFLAGLEN+9], ipmc[i1].hwaddr, 6);
    memcpy(&packbuf[CAMFLAGLEN+15], ipmc[i1].netmask, 4);
    packbuf[CAMFLAGLEN+RECORDLEN] = 0;
    memcpy(&packbuf[CAMFLAGLEN+RECORDLEN+1], "gw", 2);
    memcpy(&packbuf[CAMFLAGLEN+RECORDLEN+3], ipmc[i1].gwaddr, 4);
    packbuf[CAMFLAGLEN+RECORDLEN+7] = 0;
    memcpy(&packbuf[CAMFLAGLEN+RECORDLEN+8], CameraName, NAMELEN);
    memcpy(&packbuf[CAMFLAGLEN+RECORDLEN+8+NAMELEN], portstr, 10);


    MCTxTest(packbuf, PACKAGELEN, addr);
}
#if 0
int GetDevInterface(void)
{
    int dev_num = 0;
    struct ifreq ifbuf[MAXINTERFACES/2]={0};
    struct ifconf ifc;
    int fd, ifnum=0;
    int iInterface;
    
	if (iInterface < sizeof(g_apcNetworkInterface) / sizeof(const char *))
			if(g_ConfigParam.abAsNetEnable[iInterface])
				dev_num = iInterface;
	
    memset(&ipmc, 0, sizeof(ipmc));
    memset(&ifc, 0, sizeof(ifc));
#ifndef WLAN
    fd = socket(AF_INET, SOCK_DGRAM, 0, g_Mctest_Buf, MCTEST_BUFFER_LEN);

#else
    fd = socket(AF_INET, SOCK_DGRAM, 0);

#endif
    if (fd < 0) 
    {
        perror("socket");
        return fd;
    }

    ifc.ifc_len = sizeof(ifbuf);
    ifc.ifc_buf = (caddr_t)ifbuf;
#ifndef WLAN
    if(netioctl_withbuf(fd, SIOCGIFCONF, &ifc,sizeof(ifc), g_Mctest_Buf, MCTEST_BUFFER_LEN) < 0)
    {
        netclose(fd, g_Mctest_Buf, MCTEST_BUFFER_LEN);
        return -1;
    }
    
    ifnum = ifc.ifc_len/sizeof(struct ifreq);
#else
      /*Get interface name*/
	{
    int i;
    struct ifreq *ifrp;
    struct sockaddr *sa;
    if(ioctl(fd, SIOCGIFCONF, &ifc) != 0)
    {
        close(fd);
        return -1;
    }
    ifnum = 0;
    ifrp = ifc.ifc_req;
  	 while (ifc.ifc_len && ifrp->ifr_name[0])
  	 {
         for(i=0;i<ifnum;i++)
         	if(strcmp(ifbuf[i].ifr_name,ifrp->ifr_name) == 0) break;
         	
         if(i == ifnum)
         {	
         	 strcpy(ifbuf[ifnum].ifr_name , ifrp->ifr_name);
        	 ifnum++;
         }
          /* Scan the addresses, even if we are not interested
                 * in this interface, since we must skip to the next.
                 */
         do {
    			 sa = &ifrp->ifr_addr;
      	   	 	 if (sa->sa_len <= sizeof(*sa))
       				ifrp++;
  				else
 				{
     				 ifrp=(struct ifreq *)(sa->sa_len + (char *)sa);
     				 ifc.ifc_len -= sa->sa_len - sizeof(*sa);
   			 	}
    			ifc.ifc_len -= sizeof(*ifrp);
 		} while (!ifrp->ifr_name[0] && ifc.ifc_len);
 	}
	}
#endif

    while(ifnum-- > 0)
    {
#ifndef WLAN
        if( strlen(ifbuf[ifnum].ifr_name) > 4 ||ifbuf[ifnum].ifr_name == NULL||
        	strcmp(ifbuf[ifnum].ifr_name,"lo")==0||
            netioctl(fd, SIOCGIFFLAGS, &ifbuf[ifnum],sizeof(ifbuf[ifnum]), g_Mctest_Buf, MCTEST_BUFFER_LEN) < 0 ||
            !(ifbuf[ifnum].ifr_flags & IFF_UP) ||
            netioctl(fd, SIOCGIFADDR, &ifbuf[ifnum],sizeof(ifbuf[ifnum]), g_Mctest_Buf, MCTEST_BUFFER_LEN) < 0)
#else
        if( strlen(ifbuf[ifnum].ifr_name) < 4 ||ifbuf[ifnum].ifr_name == NULL||
            ioctl(fd, SIOCGIFFLAGS, &ifbuf[ifnum]) != 0 ||
            !(ifbuf[ifnum].ifr_flags & IFF_UP) ||
            ioctl(fd, SIOCGIFADDR, &ifbuf[ifnum]) != 0)
#endif   
		     continue; 		
        
        diag_printf("interface name is %s\n",ifbuf[ifnum].ifr_name);
        memcpy(ipmc[dev_num].name, ifbuf[ifnum].ifr_name, strlen(ifbuf[ifnum].ifr_name));
        
  		/*Get the IP Address*/ 
#ifndef WLAN  	   
        if (netioctl(fd, SIOCGIFADDR, &ifbuf[dev_num], sizeof(ifbuf[dev_num]),g_Mctest_Buf, MCTEST_BUFFER_LEN) >= 0)
#else
        if (ioctl(fd, SIOCGIFADDR, &ifbuf[dev_num]) >= 0)
#endif         
            memcpy(ipmc[dev_num].ipaddr, (char *)&(((struct sockaddr_in *)&ifbuf[ifnum].ifr_addr)->sin_addr.s_addr), 4);
         else
         	memset(ipmc[dev_num].ipaddr, 0, 4) ;
#ifndef WLAN  	   
        diag_printf("ipaddr Address====%s\n",inet_ntoa(((struct sockaddr_in *)&ifbuf[ifnum].ifr_addr)->sin_addr, g_Mctest_Buf, MCTEST_BUFFER_LEN));  
#else
        diag_printf("ipaddr Address====%s\n",inet_ntoa(((struct sockaddr_in *)&ifbuf[ifnum].ifr_addr)->sin_addr));  
#endif         
         	
		/*Get the Netmask*/
#ifndef WLAN  	   
        if (netioctl(fd, SIOCGIFNETMASK, &ifbuf[ifnum],sizeof(ifbuf[ifnum]), g_Mctest_Buf, MCTEST_BUFFER_LEN) >= 0)
#else
        if (ioctl(fd, SIOCGIFNETMASK, &ifbuf[ifnum]) == 0)
#endif         
            memcpy(ipmc[dev_num].netmask, (char *)&(((struct sockaddr_in *)&ifbuf[ifnum].ifr_netmask)->sin_addr.s_addr), 4); 
        else
            memset(ipmc[dev_num].netmask, 0, 4);
#ifndef WLAN  	   
        diag_printf("Netmask====%s\n",inet_ntoa(((struct sockaddr_in *)&ifbuf[ifnum].ifr_netmask)->sin_addr, g_Mctest_Buf, MCTEST_BUFFER_LEN));    
#else
        diag_printf("Netmask====%s\n",inet_ntoa(((struct sockaddr_in *)&ifbuf[ifnum].ifr_netmask)->sin_addr));    
#endif         
            
		/*Get the MAC Address*/
#ifndef WLAN  	   
        if (netioctl(fd, SIOCGIFHWADDR, &ifbuf[ifnum],sizeof(ifbuf[ifnum]), g_Mctest_Buf, MCTEST_BUFFER_LEN) >= 0)
#else
        if (ioctl(fd, SIOCGIFHWADDR, &ifbuf[ifnum]) == 0)
#endif         
            memcpy(ipmc[dev_num].hwaddr, (char *)ifbuf[ifnum].ifr_hwaddr.sa_data, 6);
        else
            memset(ipmc[dev_num].hwaddr, 0, 6);            
       // diag_printf("MAC Address====%d\n",ifbuf[ifnum].ifr_hwaddr.sa_data);
       
#ifndef WLAN         
        /*Get the Gateway*/
        {
        	unsigned long Gateway;
        	char tmpgw[16];
        	char *c,*p;
        	int i = 0;
        	Gateway = wb740getgateway((char*)(ifbuf[ifnum].ifr_name),g_Mctest_Buf, MCTEST_BUFFER_LEN);
        	diag_printf("Gateway====%s\n",inet_ntoa(*(struct in_addr*)&Gateway, g_Mctest_Buf, MCTEST_BUFFER_LEN));
        	if(Gateway != 0L)
        	{
        		strcpy(tmpgw,inet_ntoa(*(struct in_addr*)&Gateway, g_Mctest_Buf, MCTEST_BUFFER_LEN));
        		p = tmpgw;
        		while(*p != '\0')
        		{
        			c = p;
        			while((*p != '.') && (*p != '\0'))
        				p++;
        			*p = '\0';
        			ipmc[dev_num].gwaddr[i] = (unsigned char)atoi(c);          			
        			if(i < 3)
        			{p++; i++;  }  		
        		}	
				
			} 
			else 
				memset(ipmc[dev_num].gwaddr,0 ,4);
			
		}
#else
		//memset(ipmc[dev_num].gwaddr,0 ,4);
		memcpy(ipmc[dev_num].gwaddr, g_ConfigParam.ulAsGateway[1],4);
#endif
        dev_num++;
    }
#ifndef WLAN
    netclose(fd, g_Mctest_Buf, MCTEST_BUFFER_LEN);
#else
    close(fd);
#endif
    return dev_num;
}
#else
int GetDevInterface(void)
{
	BOOL bEnable[2];
	int dev_num = 0;
	int fd,i = 0;
	struct ifreq ifr;
	bEnable[0] = g_ConfigParam.abAsNetEnable[0];
	bEnable[1] = g_ConfigParam.abAsNetEnable[1];
	
	
#ifndef WLAN
	if (!bEnable[0] && !bEnable[1]) bEnable[0] = TRUE;
   	fd = socket(AF_INET, SOCK_DGRAM, 0, g_Mctest_Buf, MCTEST_BUFFER_LEN);
#else
	if (!bEnable[0] && !bEnable[1]) bEnable[1] = TRUE;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
#endif
    if (fd < 0) 
    {
        perror("socket");
        return fd;
    }

	for (i = sizeof(g_apcNetworkInterface) / sizeof(const char *) - 1; i >= 0; i--)
	{
		if (!bEnable[i]) continue;
		strcpy(ifr.ifr_name, g_apcNetworkInterface[i]);
		
#ifndef WLAN
        if( netioctl(fd, SIOCGIFFLAGS, &ifr,sizeof(ifr), g_Mctest_Buf, MCTEST_BUFFER_LEN) < 0 ||
            !(ifr.ifr_flags & IFF_UP) ||
            netioctl(fd, SIOCGIFADDR, &ifr,sizeof(ifr), g_Mctest_Buf, MCTEST_BUFFER_LEN) < 0)
#else
		 if( ioctl(fd, SIOCGIFFLAGS, &ifr) != 0 ||
            !(ifr.ifr_flags & IFF_UP) ||
            ioctl(fd, SIOCGIFADDR, &ifr) != 0)
#endif   
		     continue; 		
        
        diag_printf("interface name is %s\n",ifr.ifr_name);
        memcpy(ipmc[dev_num].name, ifr.ifr_name, strlen(ifr.ifr_name));
        
  		/*Get the IP Address*/ 
#ifndef WLAN  	   
        if (netioctl(fd, SIOCGIFADDR, &ifr, sizeof(ifr),g_Mctest_Buf, MCTEST_BUFFER_LEN) >= 0)
#else
        if (ioctl(fd, SIOCGIFADDR, &ifr) >= 0)
#endif         
            memcpy(ipmc[dev_num].ipaddr, (char *)&(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr), 4);
         else
         	memset(ipmc[dev_num].ipaddr, 0, 4) ;
#ifndef WLAN  	   
        diag_printf("ipaddr Address====%s\n",inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr, g_Mctest_Buf, MCTEST_BUFFER_LEN));  
#else
        diag_printf("ipaddr Address====%s\n",inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));  
#endif         
		if (((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr == 0)
			continue;
         	
		/*Get the Netmask*/
#ifndef WLAN  	   
        if (netioctl(fd, SIOCGIFNETMASK, &ifr,sizeof(ifr), g_Mctest_Buf, MCTEST_BUFFER_LEN) >= 0)
#else
        if (ioctl(fd, SIOCGIFNETMASK, &ifr) == 0)
#endif         
            memcpy(ipmc[dev_num].netmask, (char *)&(((struct sockaddr_in *)&ifr.ifr_netmask)->sin_addr.s_addr), 4); 
        else
            memset(ipmc[dev_num].netmask, 0, 4);
#ifndef WLAN  	   
        diag_printf("Netmask====%s\n",inet_ntoa(((struct sockaddr_in *)&ifr.ifr_netmask)->sin_addr, g_Mctest_Buf, MCTEST_BUFFER_LEN));    
#else
        diag_printf("Netmask====%s\n",inet_ntoa(((struct sockaddr_in *)&ifr.ifr_netmask)->sin_addr));    
#endif   
		/*Get the MAC Address*/
#ifndef WLAN  	   
        if (netioctl(fd, SIOCGIFHWADDR, &ifr,sizeof(ifr), g_Mctest_Buf, MCTEST_BUFFER_LEN) >= 0)
#else
        if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0)
#endif         
            memcpy(ipmc[dev_num].hwaddr, (char *)ifr.ifr_hwaddr.sa_data, 6);
        else
            memset(ipmc[dev_num].hwaddr, 0, 6);            
      // diag_printf("MAC Address====%d\n",ifbuf[ifnum].ifr_hwaddr.sa_data);

#ifndef WLAN         
        /*Get the Gateway*/
        {
        	unsigned long Gateway;
        	char tmpgw[16];
        	char *c,*p;
        	int i = 0;
        	Gateway = wb740getgateway((char*)(ifr.ifr_name),g_Mctest_Buf, MCTEST_BUFFER_LEN);
        	diag_printf("Gateway====%s\n",inet_ntoa(*(struct in_addr*)&Gateway, g_Mctest_Buf, MCTEST_BUFFER_LEN));
        	if(Gateway != 0L)
        	{
        		strcpy(tmpgw,inet_ntoa(*(struct in_addr*)&Gateway, g_Mctest_Buf, MCTEST_BUFFER_LEN));
        		p = tmpgw;
        		while(*p != '\0')
        		{
        			c = p;
        			while((*p != '.') && (*p != '\0'))
        				p++;
        			*p = '\0';
        			ipmc[dev_num].gwaddr[i] = (unsigned char)atoi(c);          			
        			if(i < 3)
        			{p++; i++;  }  		
        		}	
				
			} 
			else 
				memset(ipmc[dev_num].gwaddr,0 ,4);
			
		}
#else
		memset(ipmc[dev_num].gwaddr,0 ,4);
#endif
        dev_num++;
    }      
#ifndef WLAN
    netclose(fd, g_Mctest_Buf, MCTEST_BUFFER_LEN);
#else
    close(fd);
#endif
    return dev_num;		
}
#endif

int mc_rx_socket(char *portstr)
{
    int dev_num;
    int st, sd, i1;
    struct sockaddr_in sa;
    fd_set readfds;
    char testbuf[100];
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(MCTESTPORT);
    sa.sin_addr.s_addr = htons(INADDR_ANY);
#ifndef WLAN
    sd = socket(AF_INET, SOCK_DGRAM, 0, g_Mctest_Buf, MCTEST_BUFFER_LEN);
#else
    sd = socket(AF_INET, SOCK_DGRAM, 0);
#endif
    if (sd < 0)
    {
        perror("socket");
        return sd;
    }

    i1 = 1;
#ifndef WLAN
    st = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&i1, sizeof(i1), g_Mctest_Buf, MCTEST_BUFFER_LEN);
#else
    st = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&i1, sizeof(i1));
#endif
    if (st < 0) 
    {
        perror("setsockopt");
        goto unsock;
    }
#ifndef WLAN

    st = bind(sd, (struct sockaddr *)&sa, sizeof(sa), g_Mctest_Buf, MCTEST_BUFFER_LEN);
#else
    st = bind(sd, (struct sockaddr *)&sa, sizeof(sa));
#endif
    if (st < 0)
    {
        perror("bind");
        goto unsock;
    }

    while(g_endflag)
    {
        FD_ZERO(&readfds);
        FD_SET(sd, &readfds);
 #ifndef WLAN
       if((netselect(sd + 1, &readfds, 0, 0, &timeout, g_Mctest_Buf, MCTEST_BUFFER_LEN))<0)
#else
       if((select(sd + 1, &readfds, 0, 0, &timeout)) <= 0)
#endif
        {
         	diag_printf("mctest timeout\n");

			dev_num = GetDevInterface();
			if(  dev_num > 0 )
			{
	            unsigned long ip, mask;
	            for(i1=0; i1<dev_num; i1++)
    	        {
    	        	if(memcmp(ipmc[i1].name, "wlan", 4) == 0)
    	        	{
    	        		CreateAndSendPackage(i1,INADDR_BROADCAST,portstr);
	    	        	//diag_printf("name:[%c%c%c%c%d,%d,]\n", ipmc[i1].name[0], ipmc[i1].name[1],
    		        	//	ipmc[i1].name[2], ipmc[i1].name[3],
    		        	//	(int)ipmc[i1].name[4], (int)ipmc[i1].name[5]);
    		        }
            	}
	        }
         	
         	cyg_thread_delay(10);
         	continue;
		}
        memset(testbuf, 0, sizeof(testbuf));
        i1 = sizeof(sa);
  #ifndef WLAN
       st = recvfrom(sd, testbuf, sizeof(testbuf), 0, (struct sockaddr *)&sa, &i1, g_Mctest_Buf, MCTEST_BUFFER_LEN);
#else        
		st = recvfrom(sd, testbuf, sizeof(testbuf), 0, (struct sockaddr *)&sa, (unsigned int *)&i1);
#endif
        if(st<IPFLAGLEN || memcmp(testbuf, IPEDITFLAG, IPFLAGLEN) != 0)
            continue;

        dev_num = GetDevInterface();
        if(  dev_num <= 0 )
            continue;

        if(testbuf[IPFLAGLEN] == 2)
        {
            unsigned long ip, mask;

            for(i1=0; i1<dev_num; i1++)
            {
                memcpy(&ip, ipmc[i1].ipaddr, 4);
                memcpy(&mask, ipmc[i1].netmask, 4);

                if((ip&mask) == (sa.sin_addr.s_addr&mask))
                    break;
            }
            if(i1 == dev_num)
                i1=0;

            CreateAndSendPackage(i1,sa.sin_addr.s_addr,portstr);
        }
    }

unsock:
  #ifndef WLAN
    netclose(sd, g_Mctest_Buf, MCTEST_BUFFER_LEN);
#else        
    close(sd);
#endif
    return st;
}

void MctestThread(cyg_addrword_t pParam)
{
    int status;
    char portstr[11];
    int *port = (int*)pParam;
    g_endflag = TRUE;
    memset(portstr,0,sizeof(portstr));
    sprintf(portstr, "%5d%5d", *port, *(port+1));
    status = mc_rx_socket(portstr);
    if (status < 0)
        diag_printf("ERROR:  SOCKET test fails or Timeout %d\n", status);
}

void MctestThread_release(void)
{
	cyg_thread_info MctestInfo;   
    g_endflag = FALSE;
  
	if (ptdMctest_handle != NULL)
	{
		thread_join(&ptdMctest_handle,&g_ptdMctest,&MctestInfo);
	}

    
}
