#ifndef MCTEST_H
#define MCTEST_H

#define MCTESTPORT    4000

#define CAMFLAG		"IPCNIS"
#define CAMFLAGLEN	6

#define IPEDITFLAG	"IPENIS"
#define IPFLAGLEN	6

#define RECORDLEN	19
#define NAMELEN		20

#define PACKAGELEN	(CAMFLAGLEN+RECORDLEN+8+NAMELEN+10)

#define MAXINTERFACES 10

struct dev_addr{
    unsigned char name[4];
    unsigned char ipaddr[4];
    unsigned char hwaddr[6];
    unsigned char netmask[4];
    unsigned char gwaddr[4];
};

void  MCTxTest(char *p, int plen, unsigned long addr);
void MctestThread_release(void);
void MctestThread(cyg_addrword_t pParam);
#endif
