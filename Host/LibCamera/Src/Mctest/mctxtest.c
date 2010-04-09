#include "Commondef.h"


int mc_tx_socket(char *p, int plen, unsigned long addr)
{
    int st, sd;
    struct sockaddr_in sap;
#ifndef WLAN
    sd = socket(AF_INET, SOCK_DGRAM, 0, g_Mctest_Buf, MCTEST_BUFFER_LEN);   /* UDO/IP */
#else
    sd = socket(AF_INET, SOCK_DGRAM, 0);   /* UDO/IP */
#endif
    if (sd < 0) 
        return sd;

	if(addr==INADDR_BROADCAST)
	{
		int i1 = 1;
		setsockopt(sd, SOL_SOCKET, SO_BROADCAST, (char *)&i1, sizeof(i1));
	}

    memset(&sap, 0, sizeof(sap));
    sap.sin_family = AF_INET;
    sap.sin_port = htons(MCTESTPORT);
    sap.sin_addr.s_addr = addr;

    /* Transmit packet */
#ifndef WLAN
    st = sendto(sd, p, plen, 0, (struct sockaddr *)&sap, sizeof(sap), g_Mctest_Buf, MCTEST_BUFFER_LEN);
    netclose(sd, g_Mctest_Buf, MCTEST_BUFFER_LEN);
#else
    st = sendto(sd, p, plen, 0, (struct sockaddr *)&sap, sizeof(sap));
    close(sd);
#endif
    return st;
}

void  MCTxTest(char *p, int plen, unsigned long addr)
{
    int s = mc_tx_socket(p, plen, addr);
    if (s < 0)
        diag_printf("ERROR:  SOCKET transmit test fails %d\n", s);
}