

/*---------- Tuple ID tables ----------*/


/*----- Layer 1 : Basic Compatibility Tuples -----*/

			/* Control Tuples */
#define		CISTPL_CHECKSUM				0x10
#define		CISTPL_END					0xFF
#define		CISTPL_INDIRECT				0x03
#define		CISTPL_LINKTARGET			0x13
#define		CISTPL_LONGLINK_A			0x11
#define		CISTPL_LONGLINK_C			0x12
#define		CISTPL_LONGLINK_CB			0x02
#define		CISTPL_LOINGLINK_MFC		0x06
#define		CISTPL_NO_LINK				0x14
#define		CISTPL_NULL					0x00

			/* Basic Compability Tuples */
#define		CISTPL_ALTSTR				0x16
#define		CISTPL_DEVICE				0x01
#define		CISTPL_DEVICE_A				0x17
#define		CISTPL_DEVICE_OA			0x1D
#define		CISTPL_DEVICE_OC			0x1C
#define		CISTPL_DEVICEGEO			0x1E
#define		CISTPL_DEVICEGEO_A			0x1F
#define		CISTPL_EXTDEVICE			0x09
#define		CISTPL_FUNCE				0x22
#define		CISTPL_FUNCID				0x21
#define		CISTPL_JEDEC_A				0x19
#define		CISTPL_JEDEC_C				0x18
#define		CISTPL_MANFID				0x20
#define		CISTPL_VERS_1				0x15

			/* Configuration Tuples */
#define		CISTPL_BAR					0x07
#define		CISTPL_CFTABLE_ENTRY		0x1B
#define		CISTPL_CFTABLE_ENTRY_CB		0x05
#define		CISTPL_CONFIG				0x1A
#define		CISTPL_CONFIG_CB			0x04
#define		CISTPL_PWR_MGMNT			0x08

/*----- Layer 2: Data Recording Format Tuples -----*/
			
			/* Card Information Tuples */
#define		CISTPL_BATTERY				0x45
#define		CISTPL_DATE					0x44
#define		CISTPL_VERS_2				0x40

			/* Data Recording Format Tuples */
#define		CISTPL_BYTEORDER			0x43
#define		CISTPL_FORMAT				0x41
#define		CISTPL_FORMAT_A				0x47
#define		CISTPL_GEOMETRY				0x42
#define		CISTPL_SWIL					0x23

/*----- Layer 3: Data Organization Tuples -----*/
#define		CISTPL_ORG					0x46

/*----- Layer 4: System-Specific Standard Tuples -----*/
#define		CISTPL_SPCL					0x90


/*----- Tuple node & chain -----*/
typedef struct cistpl_recd {
	unsigned char	cistpl_code;
	unsigned char	cistpl_link;
	unsigned int	*cisptl_body;
} CISTPL_NODE; 

typedef struct cistpl_chain {
	unsigned char	id;
	unsigned int	base;
	CISTPL_NODE     *first_tpl;
	unsigned int	*next;
} CISTPL_CHAIN;




