/*
 *  W9986 USB Control
 *
 *
 *  Winbond Electronics Corp. 2000
 */

#include "string.h"
#include "wblib.h"
#include "w99702_reg.h"
#include "vp_usb.h"
#ifdef ECOS
#include "cyg/kernel/kapi.h"
#include "cyg/infra/diag.h"
#endif


/* To output debug message by USB, must disable the printf.. funcitons in USB */
#define printf(...)	do{;}while(0)
#define diag_printf(...)	do{;}while(0)


extern int sysInit(void);
UINT8 Read_GPIO7(void);

/* Define Endpoint feature */
#define Ep_In        0x01
#define Ep_Out       0x00
#define Ep_Bulk      0x01
#define Ep_Int       0x02
#define EP_A         0x01
#define EP_B         0x02
#define EP_C         0x03

#define VCOM_CMD_READY 1
#define VCOM_CMD_NO 0



/* for usb cach */
/* vitual com data address */
#define VIRTUAL_IN_SIZE 1024
#define VIRTUAL_OUT_SIZE 1024*2+10
__align(32) UINT8 Virtual_ComIn_Block[VIRTUAL_IN_SIZE]; //for board to host;
__align(32) UINT8 Virtual_ComOut_Block[VIRTUAL_OUT_SIZE]; //for board to host;
#define Virtual_Com_Addr_In1  (UINT32 )(0x10000000 | (UINT32)&Virtual_ComIn_Block[0])
#define Virtual_Com_Addr_Out1  (UINT32 )(0x10000000| (UINT32)&Virtual_ComOut_Block[0])




static int SDRAM_USB_Transfer(UINT8 epname,UINT32 DRAM_Addr ,UINT32 Tran_Size);


/* these two variable has no use only for support function call */
#ifdef ECOS
static cyg_handle_t		int_handle_usb;   
static cyg_interrupt	int_older_handle_usb;
#endif



#define usbvaStart(list, param) list = (INT8*)((INT)&param + sizeof(param))
#define usbvaArg(list, type) ((type *)(list += sizeof(type)))[-1]


#define usbBulkInNextOne()	(((_usb_uBulkInTail+1)==VIRTUAL_IN_SIZE)? NULL: _usb_uBulkInTail+1) 
int volatile _usb_uBulkInHead=0, _usb_uBulkInTail=0;
BOOL volatile _usb_bIsUSBInit=FALSE;

/* this varibles used for thread-safe and avoid busy-waiting */
#ifdef ECOS
/* this mutex ensure usbread and USBPrintf function exclusively excess host */
cyg_mutex_t mut_read_usb;
cyg_mutex_t mut_write_usb;
/* this semphore used to wakeup usbread when data from host availible */
cyg_sem_t sem_get_data;  // comsumer

UINT32 volatile usb_inform_read = 0;

#endif

#define DISABLE_USB_INTURRUPT cyg_interrupt_mask( IRQ_USB );
#define ENABLE_USB_INTURRUPT cyg_interrupt_unmask( IRQ_USB );
//THIS SHOULD EQUEUE VIRTUAL_OUT_SIZE
#define CMD_BUF_SIZE VIRTUAL_OUT_SIZE
#define INIT_USB_DEV(p) p.head = p.tail = p.buff; \
						p.end = p.buff +  CMD_BUF_SIZE; \
						p.num = 0; \
						p.block = 0; \
						p.need_rest_dsr = 0; \
						p.rest_start = NULL; \
						p.rest_size = 0; \
						p.usb_error = 0;
typedef struct
{
	char buff[CMD_BUF_SIZE];
	char *head;
	char *tail;
	char *end;
	int num; 
	int block; //indicate whether any read has block
	int need_rest_dsr;
	char* rest_start;
	int rest_size;
	int usb_error; 
} USB_DEV;
static USB_DEV usb_dev;
usbException usbfExceptionHandle = NULL;


typedef struct 
{
    UINT32 dwAddr;
    UINT32 dwValue;
}USB_INIT_T;

typedef struct 
{
 UINT8 Req_Type;
 UINT8 Req;
 UINT16 Value;
 UINT16 Index;
 UINT16 Length;
}USB_Vender_Cmd_Format_T;   // each field of vendor command

typedef struct {
    UINT8 EP_Num;
    UINT8 EP_Dir;
    UINT8 EP_Type;
} USB_EP_Inf_T;

UINT32 volatile Bulk_Out_Transfer_Size=1;
UINT8 volatile USBModeFlag=0;
UINT8 volatile ResetFlag=0;
UINT8 volatile Bulk_First_Flag=0;
UINT8 volatile DMA_CInt_A_Flag=0;
UINT8 volatile DMA_CInt_B_Flag=0;
UINT8 volatile DMA_CInt_C_Flag=0;
UINT8 volatile Token_Input_A_Flag=0;
UINT8 volatile Token_Input_B_Flag=0;
UINT8 volatile Token_Input_C_Flag=0;

UINT8 volatile DEV_RES_Flag;
UINT8 volatile GET_DEV_Flag;
UINT8 volatile GET_CFG_Flag;
UINT8 volatile GET_STR_Flag;
UINT8 volatile CLA_CMD_Oflag;
UINT8 volatile VEN_CMD_Oflag;
UINT8 volatile VEN_CMD_Iflag;
UINT8 volatile CLA_CMD_Iflag;
UINT8 volatile CTL_BKI_Eflag;
extern UINT8 volatile USB_Power_Flag; // 0: bus power de-attached; 1: attached

//Mem start sector
UINT32 volatile USB_MEM_START;

//bulk set
UINT32 volatile Bulk_set_length=0;
UINT8 volatile bulksetflag=0;
UINT32 volatile Bulkout_set_length=0;
UINT8 volatile bulkoutsetflag=0;

/////////way
static UINT8 USB_Setup_Endpoint(UINT8 epname,UINT8 epnum,UINT8 epdir,UINT8 eptype,
                         UINT8 epcon,UINT8 epinf,UINT16 epmaxps );

//static void USB2SDRAM_Bulk(UINT32 DRAM_Addr ,UINT32 Tran_Size);
//static void SDRAM2USB_Bulk(UINT32 DRAM_Addr ,UINT32 Tran_Size);
static void SDRAM2USB_Int(UINT32 DRAM_Addr ,UINT32 Tran_Size);
static void Outp_Byte(UINT32 addr,UINT8 value);

static void USB_All_Flag_Clear(void);
static void USB_Cmd_Parsing(void);
static void Vender_Data_Out(void);
static void Vender_Data_In(void);
static void Class_Data_Out(void);
static void Class_Data_In(void);
static void Get_Dev_Dpr_In(void);
/*
UINT8 readgpio10(void);//usb-detect
*/
////////
static void USB_ISR_Reset_Start(void);
static void USB_ISR_Reset_End(void);
static void USB_ISR_Suspend(void);
static void USB_ISR_Resume(void);
static void USB_ISR_Error(void);
static void USB_ISR_Dev_Des(void);
static void USB_ISR_Conf_Des(void);
static void USB_ISR_Str_Des(void);
static void USB_ISR_Class_Cmd(void);
static void USB_ISR_Vendor_Cmd(void);
static void USB_ISR_CtlOut(void);
static void USB_ISR_CtlIn(void);
/*
void USB_ISR_Power(void);
*/
static void USB_ISR_EpA_Stall(void);
static void USB_ISR_EpA_Token_Input(void);
static void USB_ISR_EpA_DMA_Complete(void);
static void USB_ISR_EpA_Bus_Err(void);
static void USB_ISR_EpB_Stall(void);
static void USB_ISR_EpB_Token_Input(void);
static void USB_ISR_EpB_DMA_Complete(void);
static void USB_ISR_EpB_Bus_Err(void);
static void USB_ISR_EpC_Stall(void);
static void USB_ISR_EpC_Token_Input(void);
static void USB_ISR_EpC_DMA_Complete(void);
static void USB_ISR_EpC_Bus_Err(void);

USB_Vender_Cmd_Format_T volatile USB_CtlOut_Format;

USB_EP_Inf_T volatile USB_EP_Inf_Array[4];

static USB_Device[5] = {
0x01100112, 0x10000000, 0x70210416, 0x02010100, 0x00000100 };

static USB_Config[8] = {
0x00200209, 0xC0000101, 0x00040932, 0x00000200 , 0x05070000, 0x00400281,
0x02050701, 0x01004002 };

static USB_Lang_Id_String[1] = {
0x04090304 };

static USB_Vender_Id_String[6] = {
0x00550316, 0x00420053, 0x00440020, 0x00760065, 0x00630069 ,0x00000065};

#if 0	/* W99702 USB Virtual COM */
static USB_Device_Id_String[12] = {
0x0057032E, 0x00390039, 0x00300037, 0x00200032, 0x00530055, 0x00200042, 0x00690056,
0x00740072, 0x00610075, 0x0020006C, 0x004F0043, 0x0000004D};
#else	/* Rovio 1.0 USB */
static USB_Device_Id_String[12] = {
0x0052032E, 0x0076006f, 0x006f0069, 0x00310020, 0x0030002E, 0x00550020, 0x00420053,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000};
#endif

static USB_Version[1] = { 0x00000001 };

static USB_INIT_T USBInit[6] = 
{
    0x00,0x000000e4,
    0x08,0x000047f7,
    0x14,0x00000030,
    0x38,0x00000000,
    0x44,0x00000001,
    0x48,0x00000000,
};
static USB_INIT_T USBRetInit[25] = 
{
    0x00,0x00000000,
    0x04,0x00000000,
    0x08,0x00000000,
    0x10,0x0000ffff,
    0x14,0x00000000,
    0x38,0x00000000,
    0x44,0x00000000,
    0x48,0x00000000,
    0x4C,0x00000000,
    0x50,0x00000000,
    0x54,0x0000003f,
    0x5C,0x00000000,
    0x60,0x00000000,
    0x64,0x00000000,
    0x68,0x00000000,
    0x6C,0x00000000,
    0x70,0x0000003f,
    0x78,0x00000000,
    0x7C,0x00000000,
    0x80,0x00000000,
    0x84,0x00000000,
    0x88,0x00000000,
    0x8C,0x0000003f,
    0x94,0x00000000,
    0x98,0x00000000,
};
static UINT8 USB_Setup_Endpoint(UINT8 epname,UINT8 epnum,UINT8 epdir,UINT8 eptype,
                         UINT8 epcon,UINT8 epinf,UINT16 epmaxps )
{
    UINT32 tmp;
    
    if (epname == EP_A)
    {
    	if ((epdir==Ep_In)&&(eptype==Ep_Bulk)&&(epcon==1))
    	    outpw(REG_USB_EPA_INFO,0x30000010);
    	else if ((epdir==Ep_In)&&(eptype==Ep_Int)&&(epcon==1))   
    	    outpw(REG_USB_EPA_INFO,0x50000010);   
    	else if ((epdir==Ep_Out)&&(eptype==Ep_Bulk)&&(epcon==1))   
    	    outpw(REG_USB_EPA_INFO,0x20000010);   
    	else   
    	     return 0;   
    	tmp = epinf;   
    	tmp = tmp << 8;   
    	outpw(REG_USB_EPA_INFO,(inpw(REG_USB_EPA_INFO)|tmp));	// interface = 0   
    	tmp = epmaxps;   
    	tmp = tmp << 16;   
    	outpw(REG_USB_EPA_INFO,(inpw(REG_USB_EPA_INFO)|tmp));	// max packet size = 64     
    	outpw(REG_USB_EPA_INFO,(inpw(REG_USB_EPA_INFO)|(epnum&0x0f)));   
        USB_EP_Inf_Array[EP_A].EP_Num = epnum;
        USB_EP_Inf_Array[EP_A].EP_Dir = epdir;
        USB_EP_Inf_Array[EP_A].EP_Type = eptype;
    	outpw(REG_USB_EPA_CTL,0x00000001);   
    }
    else if (epname == EP_B)
    {
    	if ((epdir==Ep_In)&&(eptype==Ep_Bulk)&&(epcon==1))   
    	    outpw(REG_USB_EPB_INFO,0x30000010);   
    	else if ((epdir==Ep_In)&&(eptype==Ep_Int)&&(epcon==1))   
    	    outpw(REG_USB_EPB_INFO,0x50000010);   
    	else if ((epdir==Ep_Out)&&(eptype==Ep_Bulk)&&(epcon==1))   
    	    outpw(REG_USB_EPB_INFO,0x20000010);   
    	else   
    	     return 0;   
    	tmp = epinf;   
    	tmp = tmp << 8;   
    	outpw(REG_USB_EPB_INFO,(inpw(REG_USB_EPB_INFO)|tmp));   
    	tmp = epmaxps;   
    	tmp = tmp << 16;   
    	outpw(REG_USB_EPB_INFO,(inpw(REG_USB_EPB_INFO)|tmp));   
    	outpw(REG_USB_EPB_INFO,(inpw(REG_USB_EPB_INFO)|(epnum&0x0f)));   
        USB_EP_Inf_Array[EP_B].EP_Num = epnum;
        USB_EP_Inf_Array[EP_B].EP_Dir = epdir;
        USB_EP_Inf_Array[EP_B].EP_Type = eptype;
    	outpw(REG_USB_EPB_CTL,0x00000001);   
    }
    else if (epname == EP_C)
    {
    	if ((epdir==Ep_In)&&(eptype==Ep_Bulk)&&(epcon==1))   
    	    outpw(REG_USB_EPC_INFO,0x30000010);   
    	else if ((epdir==Ep_In)&&(eptype==Ep_Int)&&(epcon==1))   
    	    outpw(REG_USB_EPC_INFO,0x50000010);   
    	else if ((epdir==Ep_Out)&&(eptype==Ep_Bulk)&&(epcon==1))   
    	    outpw(REG_USB_EPC_INFO,0x20000010);   
    	else   
    	     return 0;   
    	tmp = epinf;   
    	tmp = tmp << 8;   
    	outpw(REG_USB_EPC_INFO,(inpw(REG_USB_EPC_INFO)|tmp));   
    	tmp = epmaxps;   
    	tmp = tmp << 16;   
    	outpw(REG_USB_EPC_INFO,(inpw(REG_USB_EPC_INFO)|tmp));   
    	outpw(REG_USB_EPC_INFO,(inpw(REG_USB_EPC_INFO)|(epnum&0x0f)));   
        USB_EP_Inf_Array[EP_C].EP_Num = epnum;
        USB_EP_Inf_Array[EP_C].EP_Dir = epdir;
        USB_EP_Inf_Array[EP_C].EP_Type = eptype;
    	outpw(REG_USB_EPC_CTL,0x00000001);   
    }
    else
        return 0;
    return 1;
}

void USB_Init(void)
{
    int j;

    //enable GPIO7 interrupt;
    outpw(REG_GPIO_IE, (inpw(REG_GPIO_IE)|0x00000080));
    
    for (j=0 ; j<6 ; j++)
	outpw((USB_BA+USBInit[j].dwAddr),USBInit[j].dwValue);   

    for (j=0 ; j<4 ; j++)
    {
        USB_EP_Inf_Array[j].EP_Num = 0xff;
        USB_EP_Inf_Array[j].EP_Dir = 0xff;
        USB_EP_Inf_Array[j].EP_Type = 0xff;
    }
    USB_Setup_Endpoint(EP_A,1,Ep_In,Ep_Bulk,1,0,64);
    USB_Setup_Endpoint(EP_B,2,Ep_Out,Ep_Bulk,1,0,64);
//#ifndef _NO_USBDetect
    USB_Power_Flag = Read_GPIO7();
    if (USB_Power_Flag==1)
    {
        outpw(REG_USB_CTL,(inpw(REG_USB_CTL)|0x00000001));//D+ high 
    }
    else
    {
        USBModeFlag=0;
        outpw(REG_USB_CTL,(inpw(REG_USB_CTL)&0xfffffffe));//D+ low
    }
//#else
//    outpw(REG_USB_CTL,(inpw(REG_USB_CTL)|0x00000001));//D+ high 
//#endif
    outpw(REG_USB_EPA_ADDR,Virtual_Com_Addr_In1);
#ifdef ECOS
 	/* inital a mutex, make ensure mult-thread safe */  
 	cyg_mutex_init(&mut_read_usb);  
	cyg_mutex_init(&mut_write_usb);
 	cyg_semaphore_init(&sem_get_data,0);  
#endif
}


/* this is real function to handle the interupt, it first get the detail inturupt register */
#ifdef ECOS
cyg_uint32 USBHandler(cyg_vector_t vector, cyg_addrword_t data)
#else
static void USBHandler(void)
#endif
{
    UINT32 id;
#ifdef ECOS
    // mask the interrupt until DSR finished
    cyg_interrupt_mask( vector );
#endif
    id = inpw(REG_USB_IS);
#ifdef _INTISR_DEBUG
    printf("======USB_Int_ISR======\n");
    printf("REG_USB_IS=0x%x\n", id);
#endif
    if (id&0x00000001) 
    {
#ifdef _INTISR_DEBUG
    	printf("USB_ISR_Reset_Start()\n");   
#endif
        USB_ISR_Reset_Start();
    }
    if (id&0x00000002) 
    {
#ifdef _INTISR_DEBUG
    	printf("USB_ISR_Suspend()\n");   
#endif
        USB_ISR_Suspend();
    }
    if (id&0x00000004) 
    {
#ifdef _INTISR_DEBUG
    	printf("USB_ISR_Resume()\n");   
#endif
        USB_ISR_Resume();
    }
    if (id&0x00000008) 
    {
#ifdef _INTISR_DEBUG
    	printf("USB_ISR_Error()\n");   
#endif
        USB_ISR_Error();
    }
    if (id&0x00000010)
    {
#ifdef _INTISR_DEBUG
    	printf("USB_ISR_Dev_Des()\n");   
#endif
        USB_ISR_Dev_Des();
    }
    if (id&0x00000020)
    {
#ifdef _INTISR_DEBUG
    	printf("USB_ISR_Conf_Des()\n");   
#endif
        USB_ISR_Conf_Des();
    }
    if (id&0x00000040) 
    {
#ifdef _INTISR_DEBUG
    	printf("USB_ISR_Str_Des()\n");   
#endif
        USB_ISR_Str_Des();
    }
    if (id&0x00000080)  
    {
#ifdef _INTISR_DEBUG
    	printf("USB_ISR_Class_Cmd()\n");   
#endif
        USB_ISR_Class_Cmd();
    }

    if (id&0x00000100) 
    {
#ifdef _INTISR_DEBUG
    	printf("USB_ISR_Vendor_Cmd()\n");   
#endif
        USB_ISR_Vendor_Cmd();
    }

    if (id&0x00000200) 
    {
#ifdef _INTISR_DEBUG
    	printf("USB_ISR_CtlOut()\n");   
#endif
        USB_ISR_CtlOut();
    }
    if (id&0x00000400)
    {
#ifdef _INTISR_DEBUG
    	printf("USB_ISR_CtlIn()\n");   
#endif
        USB_ISR_CtlIn();
    }
   
    if (id&0x00004000)
    {
#ifdef _INTISR_DEBUG
    	printf("USB_ISR_Reset_End()\n");   
#endif
        USB_ISR_Reset_End();
    }
    
    /* only clear all bits , avoid dead loop, add by yjyan */
    outpw(REG_USB_IC,inpw(REG_USB_IS));

    id = inpw(REG_USB_EPA_IS);
    //printf("REG_USB_EPA_IS=0x%x\n", id);
    if (id&0x00000001) 
        USB_ISR_EpA_Stall();
    if (id&0x00000002)
        USB_ISR_EpA_Token_Input();
    if (id&0x00000008)
        USB_ISR_EpA_DMA_Complete();
    if (id&0x00000010)
        USB_ISR_EpA_Bus_Err();

    id = inpw(REG_USB_EPB_IS);
    //printf("REG_USB_EPB_IS=0x%x\n", id);
    if (id&0x00000001) 
        USB_ISR_EpB_Stall();
    if (id&0x00000002) 
        USB_ISR_EpB_Token_Input();
    if (id&0x00000008)
        USB_ISR_EpB_DMA_Complete();
    if (id&0x00000010) 
        USB_ISR_EpB_Bus_Err();

    id = inpw(REG_USB_EPC_IS);
    //printf("REG_USB_EPC_IS=0x%x\n", id);
    if (id&0x00000001) 
        USB_ISR_EpC_Stall();
    if (id&0x00000002) 
        USB_ISR_EpC_Token_Input();
    if (id&0x00000008) 
        USB_ISR_EpC_DMA_Complete();
    if (id&0x00000010) 
        USB_ISR_EpC_Bus_Err();
#ifdef ECOS
	cyg_interrupt_acknowledge( vector );   
	return( CYG_ISR_HANDLED | CYG_ISR_CALL_DSR );   
#endif
} /* USBHandler */

//copy data to usb
static void copy_mem_to_usb(USB_DEV* dev, char* source, int size)
{
	int init_size = size;
	char* tmp = source;
	if(dev->tail + size > dev->end)
	{
		init_size = dev->end - dev->tail;
		memcpy(dev->tail, tmp, init_size);
		tmp = tmp + init_size;
		//wrapper to start
		dev->tail = dev->buff;
		init_size = size - init_size;
	}
	memcpy(dev->tail, tmp, init_size);
	dev->tail = dev->tail + init_size;
	if (dev->tail == dev->end)
	{
		dev->tail = dev->buff;
	}
	dev->num = dev->num + size;	
}

// how to make sure this function not be interrupted by USB DSR?
static void copy_mem_from_usb(USB_DEV* dev, char* dest, int size)
{
	int init_size = size;
	char* tmp = dest;
	if (dev->head + size > dev->end)
	{
		init_size = dev->end - dev->head;
		memcpy(tmp, dev->head, init_size);
		tmp = tmp + init_size;
		//wrapper to start
		dev->head = dev->buff;
		init_size = size - init_size;
	}
	memcpy(tmp, dev->head,init_size);
	dev->head = dev->head + init_size;
	if (dev->head == dev->end)
	{
		dev->head = dev->buff;
	}
	dev->num =  dev->num - size;	
	
}

//

/* this function used to unblock the thread which waiting for read or write
 * at present, it only handle usb_read, first, it fetch a char from usb,  
 * later, it decide whether the char is a '/r' or '/n ', if so,wake up the usb_read thread 
 */
void usbHandlerDSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
//	char* tmp;   
//	int i =0;   
	int remain = 0;
	int size = 0;
	
	// always copy data to the output queue, untill the buff is full, and wait for usb read;   
	if (usb_inform_read == 1)
	{   
		usb_inform_read = 0;  
		Bulk_First_Flag = 0;             
		//read data from hardware
		SDRAM_USB_Transfer(EP_B,Virtual_Com_Addr_Out1,Bulk_Out_Transfer_Size);    
		remain = CMD_BUF_SIZE - usb_dev.num -1;
		// if current buff can not hold all the data, we should prevent furthuer input
		if (Bulk_Out_Transfer_Size > remain)
		{
			size = remain;
			usb_dev.need_rest_dsr = 1;
			usb_dev.rest_start = (char*)Virtual_Com_Addr_Out1 + size;
			usb_dev.rest_size = Bulk_Out_Transfer_Size - size;
		} else {
			size = Bulk_Out_Transfer_Size;
		}
		//copy data to usb_dev buff
		copy_mem_to_usb(&usb_dev,(char*)Virtual_Com_Addr_Out1, size);
		//wakeup read
		if (usb_dev.block == 1)
		{
			cyg_semaphore_post( &sem_get_data ); 
			//avoid wakeup more than one time
			usb_dev.block = 0;
		}
		if (usb_dev.need_rest_dsr == 1)
		{ 
			return; 	              
		}
		outpw(REG_USB_IE,inpw(REG_USB_IE)|0x100);//enable vendor cmd again(this is disabled in USB_ISR_Vendor_Cmd)  
	}    
	cyg_interrupt_unmask( vector );  

}
// when usb buff is full, we should call this function to turn on vender-define interrupt after calling read
// note: this function will not be interrupted by usb DSR
static void callRestDSR()
{
	int remain;
	int size;
	remain = CMD_BUF_SIZE - usb_dev.num -1;
	// if current buff can not hold all the data, we should prevent furthuer input
	if (usb_dev.rest_size > remain)
	{
		size = remain;
		//copy data to usb_dev buff
		copy_mem_to_usb(&usb_dev,usb_dev.rest_start, size);
		usb_dev.need_rest_dsr = 1;
		usb_dev.rest_start = usb_dev.rest_start + size;
		usb_dev.rest_size = usb_dev.rest_size - size;
	} else {
		size = usb_dev.rest_size;
		//copy data to usb_dev buff
		copy_mem_to_usb(&usb_dev,usb_dev.rest_start, size);
		usb_dev.need_rest_dsr = 0;
		usb_dev.rest_start = NULL;
		usb_dev.rest_size = 0;
	}
	//if all the hardware data has remove to software buff, turn on usb interrupt 
	if (usb_dev.need_rest_dsr == 0)
	{
		outpw(REG_USB_IE,inpw(REG_USB_IE)|0x100);//enable vendor cmd again(this is disabled in USB_ISR_Vendor_Cmd)  
		usb_dev.need_rest_dsr = 0;  
		cyg_interrupt_unmask( IRQ_USB );  
	}
	
}

 

/* instasll the interrut handle into system, support either ecos or 
 * other system
 *
 */

void USB_Int_Init(void)
{
  
#ifdef ECOS
    /* modify by Qzhang disable and enable interrupt in pairs */
    cyg_interrupt_disable();
    cyg_interrupt_create(IRQ_USB, 1, 0, &USBHandler,    
						 &usbHandlerDSR, &int_handle_usb,                  
						 &int_older_handle_usb);                  
    cyg_interrupt_attach(int_handle_usb);   
    cyg_interrupt_unmask(IRQ_USB);
    cyg_interrupt_enable();

#else
    sysInstallISR(IRQ_LEVEL_1, IRQ_USB, (PVOID)USBHandler);
    /* enable USB interrupt */
    sysEnableInterrupt(IRQ_USB);
#endif
    
    outpw(REG_USB_EPA_IE,(inpw(REG_USB_EPA_IE)|0x00000008));//enable DMA interrupt
    outpw(REG_USB_EPB_IE,(inpw(REG_USB_EPB_IE)|0x00000008));//enable DMA interrupt
 }/*USB_Int_Init*/


static void USB_ISR_Reset_Start(void)
{
    outpw(REG_USB_EPA_CTL,inpw(REG_USB_EPA_CTL)|0x00000002);
    outpw(REG_USB_EPB_CTL,inpw(REG_USB_EPB_CTL)|0x00000002);
    outpw(REG_USB_EPC_CTL,inpw(REG_USB_EPC_CTL)|0x00000002);
    outpw(REG_USB_EPA_CTL,inpw(REG_USB_EPA_CTL)&0xfffffffd);
    outpw(REG_USB_EPB_CTL,inpw(REG_USB_EPB_CTL)&0xfffffffd);
    outpw(REG_USB_EPC_CTL,inpw(REG_USB_EPC_CTL)&0xfffffffd);
    outpw(REG_USB_IC,0x00000001);
}

static void USB_ISR_Reset_End(void)
{
#if 0
    outpw(REG_USB_EPA_CTL,inpw(REG_USB_EPA_CTL)&0xfffffffb);
    outpw(REG_USB_EPB_CTL,inpw(REG_USB_EPB_CTL)&0xfffffffb);
    outpw(REG_USB_EPC_CTL,inpw(REG_USB_EPC_CTL)&0xfffffffb);
#else
    if ((inpw(REG_USB_EPA_CTL)&0x00000004)==0x00000004)
    {
        outpw(REG_USB_EPA_CTL,inpw(REG_USB_EPA_CTL)&0xfffffffb);
        outpw(REG_USB_EPA_CTL,inpw(REG_USB_EPA_CTL)|0x00000004);
    }
    if ((inpw(REG_USB_EPB_CTL)&0x00000004)==0x00000004)
    {
        outpw(REG_USB_EPB_CTL,inpw(REG_USB_EPB_CTL)&0xfffffffb);
        outpw(REG_USB_EPB_CTL,inpw(REG_USB_EPB_CTL)|0x00000004);
    }
    if ((inpw(REG_USB_EPC_CTL)&0x00000004)==0x00000004)
    {
        outpw(REG_USB_EPC_CTL,inpw(REG_USB_EPC_CTL)&0xfffffffb);
        outpw(REG_USB_EPC_CTL,inpw(REG_USB_EPC_CTL)|0x00000004);
    }
#endif
    ResetFlag=1;
    outpw(REG_USB_IC,0x00004000);
}
static void USB_ISR_Suspend(void)
{
    if (DEV_RES_Flag == 0)
    {
    	DEV_RES_Flag = ~DEV_RES_Flag;   
    }else
    {
#if 0
        regUPLLCtlCR = regUPLLCtlCR & 0x7f;
#endif        
    }
    outpw(REG_USB_IC,0x00000002);
}

static void USB_ISR_Resume(void)
{
#if 0
    regUPLLCtlCR = regUPLLCtlCR | 0x80;
#endif
    outpw(REG_USB_IC,0x00000004);
}

static void USB_ISR_Error(void)
{
    outpw(REG_USB_IC,0x00000008);
}

static void USB_ISR_Dev_Des(void)
{
     USB_All_Flag_Clear();     	
     GET_DEV_Flag = 1;
     USBModeFlag=1;
     if ((inpw(REG_USB_CTLS)&0x0000001f) == 0x00000008)
     {
         USB_Cmd_Parsing();
         if (USB_CtlOut_Format.Length <=(USB_Device[0]&0x000000ff))
             outpw(REG_USB_ENG,0x00000008);
         else
         {
             USB_CtlOut_Format.Length = (USB_Device[0]&0x000000ff);
             outpw(REG_USB_ENG,0x00000008);
         }
     }    
     else {
        outpw(REG_USB_ENG,0x00000002);
     }
     outpw(REG_USB_IC,0x00000010);
}

static void USB_ISR_Conf_Des(void)
{
     USB_All_Flag_Clear();     		   
     GET_CFG_Flag = 1;
     if ((inpw(REG_USB_CTLS)&0x0000001f) == 0x00000008)
     {
         USB_Cmd_Parsing();
         if (USB_CtlOut_Format.Length <= ((USB_Config[0]&0x00ff0000)>>16))
             outpw(REG_USB_ENG,0x00000008);
         else
         {
             USB_CtlOut_Format.Length = ((USB_Config[0]&0x00ff0000)>>16);
             outpw(REG_USB_ENG,0x00000008);
         }
     }    
     else {
        outpw(REG_USB_ENG,0x00000002);
     }
     outpw(REG_USB_IC,0x00000020);
}

static void USB_ISR_Str_Des(void)
{
     USB_All_Flag_Clear();     		   
     GET_STR_Flag = 1;
     if ((inpw(REG_USB_CTLS)&0x0000001f) == 0x00000008)
     {
         USB_Cmd_Parsing();
         if ((inpw(REG_USB_ODATA0)&0x00ff0000) == 0x00000000)
         {
             if (USB_CtlOut_Format.Length <= (USB_Lang_Id_String[0]&0x000000ff))
                 outpw(REG_USB_ENG,0x00000008);
             else
             {
                 USB_CtlOut_Format.Length = (USB_Lang_Id_String[0]&0x000000ff);
                 outpw(REG_USB_ENG,0x00000008);
             }    
         }        	 
         if ((inpw(REG_USB_ODATA0)&0x00ff0000) == 0x00010000)
         {
             if (USB_CtlOut_Format.Length <= (USB_Vender_Id_String[0]&0x000000ff))
                 outpw(REG_USB_ENG,0x00000008);
             else
             {
                 USB_CtlOut_Format.Length = (USB_Vender_Id_String[0]&0x000000ff);
                 outpw(REG_USB_ENG,0x00000008);
             }    
         }
         if ((inpw(REG_USB_ODATA0)&0x00ff0000) == 0x00020000)
         {
             if (USB_CtlOut_Format.Length <= (USB_Device_Id_String[0]&0x000000ff))
                 outpw(REG_USB_ENG,0x00000008);
             else
             {
                 USB_CtlOut_Format.Length = (USB_Device_Id_String[0]&0x000000ff);
                 outpw(REG_USB_ENG,0x00000008);
             }   
         }
     }    
     else {
        outpw(REG_USB_ENG,0x00000002);
     }
     outpw(REG_USB_IC,0x00000040);
}

static void USB_ISR_Class_Cmd(void)
{
     USB_All_Flag_Clear();
     if ((inpb(USB_BA+0x1b) == 0xa1)&&(inpb(USB_BA+0x1a)==0xfe))
         CLA_CMD_Iflag = 1;
     outpw(REG_USB_ENG,0x00000008);
     outpw(REG_USB_IC,0x00000080);
}

/* this function should wakeup others */
static void USB_ISR_Vendor_Cmd(void)
{
     USB_All_Flag_Clear();
     if ((inpw(REG_USB_CTLS)&0x0000001f) == 0x00000008)
     {
         USB_Cmd_Parsing();
         if (USB_CtlOut_Format.Req_Type == 0x40)
         {
             VEN_CMD_Oflag = 1;
             if (USB_CtlOut_Format.Req == 0xA0)
             {
                 if (USB_CtlOut_Format.Value == 0x12)
                 {
                    Bulk_Out_Transfer_Size=USB_CtlOut_Format.Index;
                    Bulk_First_Flag=1;
                    outpw(REG_USB_IE,inpw(REG_USB_IE)&0xfffffeff);//disable vender interrupt enable
                    if (USBModeFlag == 1) {
                    	// set the flag to inform the read thread;   
                    	usb_inform_read = 1;   
                    }
                   
                 }
                 else if (USB_CtlOut_Format.Value == 0x13)
                 {
                     if ((inpw(REG_USB_EPB_CTL)&0x00000004)==0x00000004)
                    {
                        outpw(REG_USB_EPB_CTL,inpw(REG_USB_EPB_CTL)|0x00000002);//reset
                        outpw(REG_USB_EPB_CTL,inpw(REG_USB_EPB_CTL)&0xfffffffd);
                    }
                    if ((inpw(REG_USB_EPB_CTL)&0x00000004)==0x00000004)
                        outpw(REG_USB_EPB_CTL,inpw(REG_USB_EPB_CTL)&0xfffffffb);//clear ready
                 }
             }
         }
         else if (USB_CtlOut_Format.Req_Type == 0xc0)
             VEN_CMD_Iflag = 1;
         else
             outpw(REG_USB_ENG,0x00000002);
     }    
     else {
        outpw(REG_USB_ENG,0x00000002);
     }
     outpw(REG_USB_ENG,0x00000008);
     outpw(REG_USB_IC,0x00000100);
}

static void USB_ISR_CtlOut(void)
{
     if (CLA_CMD_Oflag == 1)
         Class_Data_Out();
     else if (VEN_CMD_Oflag == 1)
         Vender_Data_Out();
     else
         outpw(REG_USB_ENG,0x00000002);
     outpw(REG_USB_ENG,0x00000008);
     outpw(REG_USB_IC,0x00000200);
}

static void USB_ISR_CtlIn(void)
{
     if (CLA_CMD_Iflag == 1)
         Class_Data_In();
     else if (VEN_CMD_Iflag == 1)
         Vender_Data_In();
     else if (GET_DEV_Flag == 1)
         Get_Dev_Dpr_In();      
     else if (GET_CFG_Flag == 1)
         Get_Dev_Dpr_In();              
     else if (GET_STR_Flag == 1)
         Get_Dev_Dpr_In();     
     else
         outpw(REG_USB_ENG,0x00000002);

     if (CTL_BKI_Eflag == 1)
//         outpw(REG_USB_ENG,0x00000004);
         outpw(REG_USB_ENG,0x00000001);
     else
         outpw(REG_USB_ENG,0x00000001);
     outpw(REG_USB_IC,0x00000400);
}

static void USB_ISR_EpA_Stall(void)
{
     outpw(REG_USB_EPA_IC,0x00000001);
}

static void USB_ISR_EpA_Token_Input(void)
{
     Token_Input_A_Flag=1;
     outpw(REG_USB_EPA_IC,0x00000002);
}

static void USB_ISR_EpA_DMA_Complete(void)
{
     DMA_CInt_A_Flag=1;
     outpw(REG_USB_EPA_IC,0x00000008);
}

static void USB_ISR_EpA_Bus_Err(void)
{
     outpw(REG_USB_EPA_IC,0x00000010);
}

static void USB_ISR_EpB_Stall(void)
{
     outpw(REG_USB_EPB_IC,0x00000001);
}

static void USB_ISR_EpB_Token_Input(void)
{
     outpw(REG_USB_EPB_IC,0x00000002);
}

static void USB_ISR_EpB_DMA_Complete(void)
{
     outpw(REG_USB_EPB_IC,0x00000008);
}

static void USB_ISR_EpB_Bus_Err(void)
{
     outpw(REG_USB_EPB_IC,0x00000010);
}

static void USB_ISR_EpC_Stall(void)
{
     outpw(REG_USB_EPC_IC,0x00000001);
}

static void USB_ISR_EpC_Token_Input(void)
{
     Token_Input_C_Flag=1;
     outpw(REG_USB_EPC_IC,0x00000002);
}

static void USB_ISR_EpC_DMA_Complete(void)
{
     DMA_CInt_C_Flag=1;
     outpw(REG_USB_EPC_IC,0x00000008);
}

static void USB_ISR_EpC_Bus_Err(void)
{
     outpw(REG_USB_EPC_IC,0x00000010);
}

static void USB_Cmd_Parsing(void)
{
     USB_CtlOut_Format.Req_Type = inpb(USB_BA+0x18);//A_U_CTOD0;
     USB_CtlOut_Format.Req = inpb(USB_BA+0x19);//A_U_CTOD1;     

     USB_CtlOut_Format.Value = inphw(USB_BA+0x1a);//2,3

     USB_CtlOut_Format.Index = inphw(USB_BA+0x1c);//4,5

     USB_CtlOut_Format.Length = inphw(USB_BA+0x1e);//6,7

     if ((USB_CtlOut_Format.Req == 0x04)||(USB_CtlOut_Format.Req == 0x05))
         USB_MEM_START = (USB_CtlOut_Format.Value<<16)+USB_CtlOut_Format.Index;

     if (GET_STR_Flag == 1)
         USB_CtlOut_Format.Index = 0;  
}

static void Class_Data_Out(void)
{
}

static void Vender_Data_Out(void)
{
     UINT16 i;
     UINT32 tmp;

     Bulk_set_length=0;
     Bulkout_set_length=0;     

     if ((inpw(REG_USB_CTLS)&0x0000001f) != 0)
     {
         if (USB_CtlOut_Format.Req == 0x01)     	    
         {
             tmp=REG_USB_ODATA0;
             for (i = 0 ; i < USB_CtlOut_Format.Length ; i++)
             {
                    Outp_Byte(USB_CtlOut_Format.Index++,inpb(tmp++));
             }
         }else if (USB_CtlOut_Format.Req == 0x13)     	  
         {  	   
             USB_MEM_START = (UINT32)USB_CtlOut_Format.Index + (UINT32)0x10000 * USB_CtlOut_Format.Value ;
             tmp=REG_USB_ODATA0;
             for (i = USB_CtlOut_Format.Length-1 ; i > 0 ; i--)
                  Bulk_set_length = Bulk_set_length + (UINT32)(inpb(tmp+i)<<(i*8));
             Bulk_set_length = Bulk_set_length + (UINT32)(inpb(tmp));
             bulksetflag = 1;                 
         }else if (USB_CtlOut_Format.Req == 0x15)     	  
         {  	   
             USB_MEM_START = (UINT32)USB_CtlOut_Format.Index + (UINT32)0x10000 * USB_CtlOut_Format.Value ;
             tmp=REG_USB_ODATA0;
             for (i = USB_CtlOut_Format.Length-1 ; i > 0 ; i--)
                  Bulkout_set_length = Bulkout_set_length + (UINT32)(inpb(tmp+i)<<(i*8));
             Bulkout_set_length = Bulkout_set_length + (UINT32)(inpb(tmp));
             bulkoutsetflag = 1;                 
         }
         else if (USB_CtlOut_Format.Req == 0x05)     	   
         { 
             USB_MEM_START = (UINT32)USB_CtlOut_Format.Index + (UINT32)0x10000 * USB_CtlOut_Format.Value ;
             tmp=REG_USB_ODATA0;
             for (i = 0 ; i < USB_CtlOut_Format.Length ; i++)
                  writeb(USB_MEM_START++,inpb(tmp++));
         }     
         USB_CtlOut_Format.Index = USB_CtlOut_Format.Index + (inpw(REG_USB_CTLS)&0x0000001f);
         USB_CtlOut_Format.Length = USB_CtlOut_Format.Length - (inpw(REG_USB_CTLS)&0x0000001f);; 
     }
}

static void Class_Data_In(void)
{
     outpw(REG_USB_CVCMD,0x01);
     CTL_BKI_Eflag = 1;
     Outp_Byte(REG_USB_IDATA0,0x00);
}

static void Vender_Data_In(void)
{
     UINT8 i;
     UINT32 tmp;

     if (USB_CtlOut_Format.Length > 0)
     {
     	 if (USB_CtlOut_Format.Length <= 16)  
     	 {  
     	     CTL_BKI_Eflag = 1;  
     	     outpw(REG_USB_CVCMD,USB_CtlOut_Format.Length);     	          
     	 }  
     	 else  
     	 {  
     	     USB_CtlOut_Format.Length = USB_CtlOut_Format.Length - 16;  
     	     outpw(REG_USB_CVCMD,0x10);  
     	 }  
         if (USB_CtlOut_Format.Req == 0x00)     	    
         {
             tmp=REG_USB_IDATA0;
     	     for (i = 0 ; i < inpw(REG_USB_CVCMD) ; i++)  
     	     {  
                  Outp_Byte(tmp,inpb(USB_CtlOut_Format.Index));
                  tmp=tmp+1;
     	          USB_CtlOut_Format.Index = USB_CtlOut_Format.Index + 1;  
     	     }  
     	       
         }else if (USB_CtlOut_Format.Req == 0x10)
         {
             tmp=REG_USB_IDATA0;
     	     for (i = 0 ; i < inpw(REG_USB_CVCMD) ; i=i+2)  
     	     {  
     	          outphw(tmp,USB_Version[USB_CtlOut_Format.Index]);  
     	     	  tmp=tmp+2;    
     	          USB_CtlOut_Format.Index = USB_CtlOut_Format.Index + 1;       	        
     	     }               
         }else if (USB_CtlOut_Format.Req == 0x04)
         {
             tmp=REG_USB_IDATA0;
     	     for (i = 0 ; i < inpw(REG_USB_CVCMD) ; i++)  
     	     {  
                  Outp_Byte(tmp,readb(USB_MEM_START++));
     	     	  tmp=tmp+1;    
             }
         }
     }
     else
         outpw(REG_USB_CVCMD,0x00); 
}

static void Get_Dev_Dpr_In(void)
{
     UINT8 i;
     
     if (USB_CtlOut_Format.Length > 0)
     {
     	 if (USB_CtlOut_Format.Length <= 16)  
     	 {  
     	     CTL_BKI_Eflag = 1;  
     	     outpw(REG_USB_CVCMD,USB_CtlOut_Format.Length);     	          
     	 }  
     	 else  
     	 {  
     	     USB_CtlOut_Format.Length = USB_CtlOut_Format.Length - 16;  
     	     outpw(REG_USB_CVCMD,0x10);  
     	 }  
     	 for (i = 0 ; i < inpw(REG_USB_CVCMD) ; i=i+4)  
     	 {  
     	      if (GET_DEV_Flag == 1)  
     	          outpw(REG_USB_IDATA0+i,USB_Device[USB_CtlOut_Format.Index]);  
              else if (GET_CFG_Flag == 1)
     	          outpw(REG_USB_IDATA0+i,USB_Config[USB_CtlOut_Format.Index]);  
              else if (GET_STR_Flag == 1)
              {
                  if ((inpw(REG_USB_ODATA0)&0x00ff0000) == 0x00000000)
     	              outpw(REG_USB_IDATA0+i,USB_Lang_Id_String[USB_CtlOut_Format.Index]);  
     	          if ((inpw(REG_USB_ODATA0)&0x00ff0000) == 0x00010000)  
     	              outpw(REG_USB_IDATA0+i,USB_Vender_Id_String[USB_CtlOut_Format.Index]);  
     	          if ((inpw(REG_USB_ODATA0)&0x00ff0000) == 0x00020000)  
     	              outpw(REG_USB_IDATA0+i,USB_Device_Id_String[USB_CtlOut_Format.Index]);  
     	      }  
    	      USB_CtlOut_Format.Index = USB_CtlOut_Format.Index + 1;     
     	 }  
     }
     else
         outpw(REG_USB_CVCMD,0x00); 
}

static void USB_All_Flag_Clear(void)
{
    DEV_RES_Flag = 0;
    VEN_CMD_Oflag = 0;
    VEN_CMD_Iflag = 0;
    CLA_CMD_Oflag = 0;
    CLA_CMD_Iflag = 0;
    GET_DEV_Flag  = 0;
    GET_CFG_Flag  = 0;
    GET_STR_Flag  = 0;
    CTL_BKI_Eflag = 0;
}

static int SDRAM_USB_Transfer(UINT8 epname,UINT32 DRAM_Addr ,UINT32 Tran_Size)
{
	if (!Read_GPIO7())	/* Plug out! */
		return -1;
	
    if (epname == EP_A)
    {
        if(Tran_Size==0)
        {
            DMA_CInt_A_Flag=0;
            outpw(REG_USB_EPA_CTL,inpw(REG_USB_EPA_CTL)|0x00000040);
            while((USBModeFlag)&&((inpw(REG_USB_EPA_CTL)&0x00000040)==0x00000040))
            {
            	if(usb_dev.usb_error || !Read_GPIO7()) return -1;
            }
        }
        else
        {
            outpw(REG_USB_EPA_ADDR,DRAM_Addr);
            outpw(REG_USB_EPA_LENTH,Tran_Size);
            DMA_CInt_A_Flag=0;
            outpw(REG_USB_EPA_CTL,inpw(REG_USB_EPA_CTL)|0x00000004);
            /* why used a busy-waiting ? */
            while((USBModeFlag)&&((inpw(REG_USB_EPA_CTL)&0x00000004)==0x00000004))
            {
            	if(usb_dev.usb_error || !Read_GPIO7()) return -1;
            }
        }

    }
    else if (epname == EP_B)
    {
        if(Tran_Size==0)
        {
            DMA_CInt_B_Flag=0;
            outpw(REG_USB_EPB_CTL,inpw(REG_USB_EPB_CTL)|0x00000040);
            while((USBModeFlag)&&((inpw(REG_USB_EPB_CTL)&0x00000040)==0x00000040))
            {
            	if(usb_dev.usb_error || !Read_GPIO7()) return -1;
            }
        }
        else
        {
            outpw(REG_USB_EPB_ADDR,DRAM_Addr);
            outpw(REG_USB_EPB_LENTH,Tran_Size);
            DMA_CInt_B_Flag=0;
            outpw(REG_USB_EPB_CTL,inpw(REG_USB_EPB_CTL)|0x00000004);
            while((USBModeFlag)&&((inpw(REG_USB_EPB_CTL)&0x00000004)==0x00000004))
            {
            	if(usb_dev.usb_error || !Read_GPIO7()) return -1;
            }
        }
    }
    else if (epname == EP_C)
    {
        if(Tran_Size==0)
        {
            DMA_CInt_C_Flag=0;
            outpw(REG_USB_EPC_CTL,inpw(REG_USB_EPC_CTL)|0x00000040);
            while((USBModeFlag)&&((inpw(REG_USB_EPC_CTL)&0x00000040)==0x00000040))
            {
            	if(usb_dev.usb_error || !Read_GPIO7()) return -1;
            }
        }
        else
        {
            outpw(REG_USB_EPC_ADDR,DRAM_Addr);
            outpw(REG_USB_EPC_LENTH,Tran_Size);
            DMA_CInt_C_Flag=0;
            outpw(REG_USB_EPC_CTL,inpw(REG_USB_EPC_CTL)|0x00000004);
            while((USBModeFlag)&&((inpw(REG_USB_EPC_CTL)&0x00000004)==0x00000004))
            {
            	if(usb_dev.usb_error || !Read_GPIO7()) return -1;
            }
        }
    }
    
    return 0;
}

static void Outp_Byte(UINT32 addr,UINT8 value)
{
    UINT32 tmp1,tmp2;
    
    tmp1=inpw(addr&0xfffffffc);
    tmp2=(UINT32)(value);
    if (addr%4==0)
    {
        tmp1=tmp1&0xffffff00;
        tmp1=tmp1|tmp2;
    }
    else if (addr%4==1)
    {
        tmp1=tmp1&0xffff00ff;
        tmp1=tmp1|(tmp2<<8);
    }
    else if (addr%4==2)
    {
        tmp1=tmp1&0xff00ffff;
        tmp1=tmp1|(tmp2<<16);
    }
    else if (addr%4==3)
    {
        tmp1=tmp1&0x00ffffff;
        tmp1=tmp1|(tmp2<<24);
    }
    outpw((addr&0xfffffffc),tmp1);
}

static void SDRAM2USB_Bulk(UINT32 DRAM_Addr ,UINT32 Tran_Size)
{
    if ((USB_EP_Inf_Array[EP_A].EP_Dir==Ep_In)&&(USB_EP_Inf_Array[EP_A].EP_Type==Ep_Bulk))
        SDRAM_USB_Transfer(EP_A,DRAM_Addr,Tran_Size);
    else if ((USB_EP_Inf_Array[EP_B].EP_Dir==Ep_In)&&(USB_EP_Inf_Array[EP_B].EP_Type==Ep_Bulk))
    	SDRAM_USB_Transfer(EP_B,DRAM_Addr,Tran_Size);   
    else if ((USB_EP_Inf_Array[EP_C].EP_Dir==Ep_In)&&(USB_EP_Inf_Array[EP_C].EP_Type==Ep_Bulk))
    	SDRAM_USB_Transfer(EP_C,DRAM_Addr,Tran_Size);   
}
static void USB2SDRAM_Bulk(UINT32 DRAM_Addr ,UINT32 Tran_Size)
{
    if ((USB_EP_Inf_Array[EP_A].EP_Dir==Ep_Out)&&(USB_EP_Inf_Array[EP_A].EP_Type==Ep_Bulk))
        SDRAM_USB_Transfer(EP_A,DRAM_Addr,Tran_Size);
    else if ((USB_EP_Inf_Array[EP_B].EP_Dir==Ep_Out)&&(USB_EP_Inf_Array[EP_B].EP_Type==Ep_Bulk))
        SDRAM_USB_Transfer(EP_B,DRAM_Addr,Tran_Size);
    else if ((USB_EP_Inf_Array[EP_C].EP_Dir==Ep_Out)&&(USB_EP_Inf_Array[EP_C].EP_Type==Ep_Bulk))
        SDRAM_USB_Transfer(EP_C,DRAM_Addr,Tran_Size);
}

static void SDRAM2USB_Int(UINT32 DRAM_Addr ,UINT32 Tran_Size)
{
    if ((USB_EP_Inf_Array[EP_A].EP_Dir==Ep_In)&&(USB_EP_Inf_Array[EP_A].EP_Type==Ep_Int))
        SDRAM_USB_Transfer(EP_A,DRAM_Addr,Tran_Size);
    else if ((USB_EP_Inf_Array[EP_B].EP_Dir==Ep_In)&&(USB_EP_Inf_Array[EP_B].EP_Type==Ep_Int))
    	SDRAM_USB_Transfer(EP_B,DRAM_Addr,Tran_Size);   
    else if ((USB_EP_Inf_Array[EP_C].EP_Dir==Ep_In)&&(USB_EP_Inf_Array[EP_C].EP_Type==Ep_Int))
    	SDRAM_USB_Transfer(EP_C,DRAM_Addr,Tran_Size);   
}


static VOID usbPutChar(UINT8 ucCh)
{
	((UINT8*)Virtual_Com_Addr_In1)[_usb_uBulkInTail] = ucCh;   
	_usb_uBulkInTail = usbBulkInNextOne();   

    if (ucCh == '\n')
    {
		((UINT8*)Virtual_Com_Addr_In1)[_usb_uBulkInTail] = '\r';      
		_usb_uBulkInTail = usbBulkInNextOne();      
    }
}


static VOID usbPutString(INT8 *string)
{
    while (*string != '\0')
    {
        usbPutChar(*string);
        string++;
    }
}


static VOID usbPutRepChar(INT8 c, INT count)
{
    while (count--)
        usbPutChar(c);
}


static VOID usbPutStringReverse(INT8 *s, INT index)
{
    while ((index--) > 0)
        usbPutChar(s[index]);
}


static VOID usbPutNumber(INT value, INT radix, INT width, INT8 fill)
{
    INT8    buffer[40];
    INT     bi = 0;
    UINT32  uvalue;
    UINT16  digit;
    UINT16  left = FALSE;
    UINT16  negative = FALSE;

    if (fill == 0)
        fill = ' ';

    if (width < 0)
    {
        width = -width;
        left = TRUE;
    }

    if (width < 0 || width > 80)
        width = 0;

    if (radix < 0)
    {
        radix = -radix;
        if (value < 0)
        {
            negative = TRUE;
            value = -value;
        }
    }

    uvalue = value;

    do
    {
        if (radix != 16)
        {
            digit = uvalue % radix;
            uvalue = uvalue / radix;
        }
        else
        {
            digit = uvalue & 0xf;
            uvalue = uvalue >> 4;
        }
        buffer[bi] = digit + ((digit <= 9) ? '0' : ('A' - 10));
        bi++;

        if (uvalue != 0)
        {
            if ((radix == 10)
                && ((bi == 3) || (bi == 7) || (bi == 11) | (bi == 15)))
            {
                buffer[bi++] = ',';
            }
        }
    }
    while (uvalue != 0);

    if (negative)
    {
        buffer[bi] = '-';
        bi += 1;
    }

    if (width <= bi)
        usbPutStringReverse(buffer, bi);
    else
    {
        width -= bi;
        if (!left)
            usbPutRepChar(fill, width);
        usbPutStringReverse(buffer, bi);
        if (left)
            usbPutRepChar(fill, width);
    }
}


static INT8 *usbFormatItem(INT8 *f, INT a)
{
    INT8   c;
    INT    fieldwidth = 0;
    INT    leftjust = FALSE;
    INT    radix = 0;
    INT8   fill = ' ';

    if (*f == '0')
        fill = '0';

    while ((c = *f++) != 0)
    {
        if (c >= '0' && c <= '9')
        {
            fieldwidth = (fieldwidth * 10) + (c - '0');
        }
        else
            switch (c)
            {
                case '\000':
                    return (--f);
                case '%':
                    usbPutChar('%');
                    return (f);
                case '-':
                    leftjust = TRUE;
                    break;
                case 'c':
                    {
                        if (leftjust)
                            usbPutChar(a & 0x7f);

                        if (fieldwidth > 0)
                            usbPutRepChar(fill, fieldwidth - 1);

                        if (!leftjust)
                            usbPutChar(a & 0x7f);
                        return (f);
                    }
                case 's':
                    {
                        if (leftjust)
                            usbPutString((PINT8)a);

                        if (fieldwidth > strlen((PINT8)a))
                            usbPutRepChar(fill, fieldwidth - strlen((PINT8)a));

                        if (!leftjust)
                            usbPutString((PINT8)a);
                        return (f);
                    }
                case 'd':
                case 'i':
                    radix = -10;
                    break;
                case 'u':
                    radix = 10;
                    break;
                case 'x':
                    radix = 16;
                    break;
                case 'X':
                    radix = 16;
                    break;
                case 'o':
                    radix = 8;
                    break;
                default:
                    radix = 3;
                    break;      /* unknown switch! */
            }
        if (radix)
            break;
    }

    if (leftjust)
        fieldwidth = -fieldwidth;

    usbPutNumber(a, radix, fieldwidth, fill);

    return (f);
}


void USBPrintf(PINT8 pcStr,...)
{
	INT8 *argP;   

	cyg_mutex_lock(&mut_write_usb);

	usbvaStart(argP, pcStr);       /* point at the end of the format string */   
	while (*pcStr)   
	{                       /* this works because args are all ints */   
		if (*pcStr == '%')      
			pcStr = usbFormatItem(pcStr + 1, usbvaArg(argP, INT));         
		else      
			usbPutChar(*pcStr++);         
	}   

    if (USBModeFlag)
    {
		while(1)      
		{      
	       	SDRAM_USB_Transfer(EP_A, Virtual_Com_Addr_In1, (_usb_uBulkInTail-_usb_uBulkInHead));   
	       	_usb_uBulkInTail = 0;   
	       	_usb_uBulkInHead = 0;   
	       	break;   
	    }   
   }
   cyg_mutex_unlock(&mut_write_usb);

}/* USBPrintf */


/* read data from usb buffer, if no data available, will not return until any charcters 
 * send to it.
 * size[in]: how many characters will be read from usb
 * return: size if usb contain enough data, otherwise return the actual numbers when read calling
 * be wakeup by usb DSR
 */
static int usbread(PCHAR buff, int size)
{
	int usb_num;
	int inturrupt_on = 0;
	//if there is no data in buff, block until any data comes
	DISABLE_USB_INTURRUPT
	//usb is off
	if (usb_dev.usb_error != 0)
	{
		diag_printf("usb is not ok\n");
		return -1;
	}
	if (usb_dev.num <= 0)
	{
		//tell DSR to wakeup
		usb_dev.block = 1;
		ENABLE_USB_INTURRUPT
		cyg_semaphore_wait( &sem_get_data );
		DISABLE_USB_INTURRUPT
	}
	if (usb_dev.usb_error != 0)
	{
		return -1;
	}
	//copy usb data to user buff
	usb_num = usb_dev.num;
	// if usb_dev buff is full, after copy out the data, we should turn on interrupt
	if(usb_num == CMD_BUF_SIZE - 1)
	{
		inturrupt_on =1;
	}
	if (usb_num < size)
	{
		size = usb_num ;
	}
	copy_mem_from_usb(&usb_dev,buff,size);
	if(inturrupt_on == 1) 
	{
		callRestDSR();
	} else {
	ENABLE_USB_INTURRUPT;
	}
	return size;
}

static int usbwrite(PCHAR buff, int size)
{
	if (USBModeFlag)   
    {   
	   	memcpy((PCHAR)Virtual_Com_Addr_In1,buff,size);     
    	if (SDRAM_USB_Transfer(EP_A, Virtual_Com_Addr_In1, size) < 0)
    		return 0;

	    if ( size % 64 == 0 )
	    {
	    	if (SDRAM_USB_Transfer(EP_A, Virtual_Com_Addr_In1, 0) < 0)
	    		return 0;
	    }
	    return 1;      
	} else {
	    return 0 ; 
	}

}


UINT32 USBRead(PCHAR pbuf, PUINT32 rt_len)
{
	int remain = *rt_len;
	char* tmp = (PCHAR)pbuf;
	int size;
	int ret = 0;
	cyg_mutex_lock(&mut_read_usb);
    while(remain > 0)
    {
    	if (remain > CMD_BUF_SIZE - 1) 
    	{
    		size = CMD_BUF_SIZE - 1;	
    	} else {
    		size = remain;
    	}
    	ret = usbread(tmp, size);
    	// a serious error
    	if (ret == -1) 
    	{
    		*rt_len = 0;
    		break;
    	
    	}
    	tmp = tmp + ret;
    	remain = remain - ret;
    	
    }
    cyg_mutex_unlock(&mut_read_usb);
    return *rt_len;
}
//#define WRITE_PACKAGE 1024
/* write a string to usb host, this string is terminate with '\n' */
UINT32 USBWrite(PCHAR pbuf, PUINT32 rt_len)
{
	PCHAR tmp = pbuf;
	UINT32 remain = *rt_len;
	UINT32 package_size = VIRTUAL_IN_SIZE;
	int try = 0;
	cyg_mutex_lock(&mut_write_usb);
	while ( remain > 0 )
	{
		if ( remain < VIRTUAL_IN_SIZE)
		{
			package_size = remain;
		}
		
		//if (package_size % 64 == 0 && package_size != 0)
		//	package_size--;
#define USB_VCOM_MAX_WRITE_SIZE 63
		if (package_size >= USB_VCOM_MAX_WRITE_SIZE + 1)
			package_size = USB_VCOM_MAX_WRITE_SIZE;
	
		diag_printf("usbwrite: %d\n", package_size);
		if (usbwrite(tmp, package_size))
		{
			diag_printf("usbwrite: ok\n", package_size);
			tmp = tmp + package_size;
			remain = remain - package_size;
		}
		else
		{
			diag_printf("usbwrite: failed\n", package_size);
			if (++try == 10)
				break;
		}
	}
	cyg_mutex_unlock(&mut_write_usb);		      
	return (remain > 0 ? -1 : *rt_len);      
}/* usbWrite */

/* reset the VCOM_CmdBuf buff, caused by some wild or ghost characters */
void USBResetForVCom() 
{
	//usb_dev.head = usb_dev.tail = usb_dev.buff;
	DISABLE_USB_INTURRUPT;
	INIT_USB_DEV(usb_dev);
	ENABLE_USB_INTURRUPT;
}



/* initial usb communication including install interupt */

void USBInitForVCom(void ) {
	/* initial the usb clock and */
	//sysInit();
	/* install interrupt */
	USB_Int_Init();
	/* initial usb regestor */
	USB_Init(); 
	INIT_USB_DEV(usb_dev);
}

UINT8 Read_GPIO7(void)
{
    outpw(REG_GPIO_OE,inpw(REG_GPIO_OE)|0x00000080);
    if ((inpw(REG_GPIO_STS)&0x00000080)==0x00000080)
        return 1;
    else 
        return 0;
}

void USBExceptionHandle()
{
	if (usbfExceptionHandle != NULL)
	{
		usbfExceptionHandle();
	}
	if (usb_dev.block == 1)
	{
		if (!Read_GPIO7())	/* Plug out! */
		{
			usb_dev.usb_error = 1;
			cyg_semaphore_post( &sem_get_data );
		}
	}
	// this case never come ture, since client guarrettte not to overflow usb buff. 
	if (usb_dev.need_rest_dsr == 1)
	{
		outpw(REG_USB_IE,inpw(REG_USB_IE)|0x100);//enable vendor cmd again(this is disabled in USB_ISR_Vendor_Cmd)  
		//cyg_interrupt_unmask( IRQ_USB );  
	}
	//when usbread try to access usb_dev, it first unmask interrupt, so after user unplug the usb, we should turn it on
	cyg_interrupt_unmask( IRQ_USB );
}

usbException USBRegiesterException(usbException pfn)
{
	usbException old = usbfExceptionHandle;
	usbfExceptionHandle = pfn;
	return old;

}




