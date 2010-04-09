/***************************************************************************
 *                                                                         *
 * Copyright (c) 2008 Nuvoton Technology Corp. All rights reserved.        *
 *                                                                         *
 ***************************************************************************/
// This file contains UPnP related thread and functions 

#ifndef UPNP_H
#define UPNP_H

// Find the parameter name to open port
static int find_port_param(char *instr, int intf);
static char *find_uuid(char *input);
static char *find_descurl(char *input);
static int find_scpdurl(char *c, int intf);
static int find_control_url(char *c, int intf);
static int find_event_url(char *c, int intf);
static int find_max_age(char *c);

static void flush_upnp_struct(void);

// restart CP job, including find igd, open port, and find external IP
void upnp_restart(void);



// request decription xml of root device
static void req_desc(char *url);


static int ssdp_recv_response(void);

static void gen_ssdp_pkt(const char * ntst,char *usn, int type);

static int ssdp_send_advertisement(void);


// process advertisement and search packets
static void ssdp_adv_srch(void);

int gena_send_subscribe(int action, int intf); // action 0 is using NT, 1 is using SID


int gena_recv_response(int intf);



int gena_recv_notify(int index);


void gena_connect_notify(void);

static int soap_invoke(int action, int method, int intf);

static int soap_recv_response(int intf, int action);

static void req_serv_xml(char *c);

// action x: mappin port [x]
// method 0: POST, 1: M-POST
// intf 0: IP, 1: PPP
static int soap_invoke(int action, int method, int intf);

static void process_xml(int intf);

static void recv_xml(int *sock, int intf);


static int gen_uuid(void);



static void upnp_thread(cyg_addrword_t pParam);


int upnp_get_extip(int intf, char *c);


int upnp_get_port(int intf, unsigned short *port);


void init_upnp(unsigned short base_port, unsigned short *port, unsigned char *protocol);
void upnp_refresh(void);
int upnp_is_halt(void);


static int XML_Analysis(char *req,char *B_msg,char *E_msg,char *arg, int size);


// web call back functions

// send root descriptiton file
int Http_SendDesc(HTTPCONNECTION hConnection, void *pParam);


// send 500 error for SOAP INVOKE and GENA SUBSCRIBE
int Http_Send500(HTTPCONNECTION hConnection, void *pParam);

#define DEFAULT_UPNP_FIRST_EXT_PORT 8168

#endif 

