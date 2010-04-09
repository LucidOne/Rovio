#ifndef __ASF_OBJECT__
#define __ASF_OBJECT__

#include "wbtypes.h"
#include "asf_types.h"


#define GUID_OCCUPY_SIZE	16
#define QWORD_OCCUPY_SIZE	8
#define DWORD_OCCUPY_SIZE	4
#define WORD_OCCUPY_SIZE	2
#define CHAR_OCCUPY_SIZE	1

struct Content_Descriptor;
struct Single_Sub_Payload_Data;
struct ASF_Data_Packet;
//typedef struct Content_Descriptor;


typedef struct {
	GUID 		ObjectID;
	ASF_QWORD 	ObjectSize;
	GUID		ResField1;
	ASF_WORD	ResField2;
	ASF_DWORD	HEDSize;
	ASF_BYTE	HED[196];
}Header_Ext_Object;//read write finish

typedef struct {
	GUID 		ObjectID;
	ASF_QWORD 	ObjectSize;
	GUID 		FileID;
	ASF_QWORD 	FileSize;
	ASF_QWORD	CreateData;
	ASF_QWORD	PacketNum;
	ASF_QWORD	PlayDuration;
	ASF_QWORD	SendDuration;
	ASF_QWORD	Preroll;
	ASF_DWORD	Flags;
	ASF_DWORD	MinPacketSize;
	ASF_DWORD	MaxPacketSize;
	ASF_DWORD	MaxBitRate;
}File_Proper_Object;

typedef struct {
	ASF_WORD	Flags;
	ASF_DWORD	Ave_bitrate;
}Bitrate_record;

typedef struct {
	GUID 		ObjectID;
	ASF_QWORD 	ObjectSize;
	ASF_WORD	Bitrate_RC;
	Bitrate_record	BitrateR;
}Str_Bit_Pro_Object;//write finish

typedef struct tagSPO *SPO_point;
typedef struct tagSPO{
	GUID 		ObjectID;
	ASF_QWORD	ObjectSize;
	GUID		Streamtype;
	GUID		ErrorCorType;
	ASF_QWORD	TimeOffset;
	ASF_DWORD	TSD_Length;
	ASF_DWORD	ECD_length;
	ASF_WORD	Flags;
	ASF_DWORD	Res;
	ASF_BYTE	TSD[128];
	ASF_BYTE	ECD[64];
}Stream_Proper_Object;//read write finish

typedef struct {
	ASF_WORD	Type;
	ASF_WORD	Cnamelong;
	ASF_WCHAR	Cname[128];
	ASF_WORD	Cdeslong;
	ASF_WCHAR	Ceds[32];
	ASF_WORD	Cinflong;
	ASF_BYTE	Cinf[32];
}Codec_Entry;

typedef struct {
	GUID		ObjectID;
	ASF_QWORD 	ObjectSize;
	GUID		Res;
	ASF_DWORD	CEC;
	Codec_Entry	CE[4];
}Codec_List_Object;//read write finish

typedef struct {
	GUID 		ObjectID;
	ASF_QWORD 	ObjectSize;
	ASF_WORD	Title_length;
	ASF_WORD	Author_length;
	ASF_WORD	Copyright_length;
	ASF_WORD	Des_length;
	ASF_WORD	Rating_length;
	ASF_WCHAR	*Title;
	ASF_WCHAR	*Author;
	ASF_WCHAR	*Copyright;
	ASF_WCHAR	*Des;
	ASF_WCHAR	*Rating;
}Content_Des_Object;//read finish

typedef union {
	ASF_WORD	*Unicode_string;
	ASF_BYTE	*Byte_array;
	ASF_DWORD	Bool32;
	ASF_DWORD	Dword;
	ASF_QWORD	Qword;
	ASF_WORD	Word;
}Des_Vaule;

typedef struct {
	ASF_WORD	Desnamlength;
	ASF_WCHAR	*Desname;
	ASF_WORD	DVDT;
	ASF_WORD	DVlength;
	Des_Vaule	DV;
}Content_Descriptor; 

typedef struct {
	GUID 		ObjectID;
	ASF_QWORD 	ObjectSize;
	ASF_WORD	CDCount;
	Content_Descriptor*	CDes;
}Ex_Content_Des_Object;

typedef struct {
	ASF_WORD	CT_Name_Length;
	ASF_WCHAR*	CT_Name;
}Command_Type;

typedef struct {
	ASF_DWORD	Present_Time;
	ASF_WORD	Type_Index;
	ASF_WORD	Command_Name_Length;
	ASF_WCHAR*	Comand_Name;
}Command;

typedef struct {
	GUID 	ObjectID;
	ASF_QWORD 	ObjectSize;
	GUID 	Res;
	ASF_WORD	Command_Count;
	ASF_WORD	Command_Type_Count;
	Command_Type*	This_CT;
	Command*	This_Command;	
}Script_Command_Object;

typedef struct {
	ASF_QWORD	Offset;
	ASF_QWORD	Present_Time;
	ASF_WORD	Entry_Length;
	ASF_DWORD	Send_Time;
	ASF_DWORD	Flags;
	ASF_DWORD	Marker_Des_Len;
	ASF_WCHAR	*Marker_Des;
}Markers;

typedef struct {
	GUID 	ObjectID;
	ASF_QWORD 	ObjectSize;
	GUID	Res1;
	ASF_DWORD	Marker_Count;
	ASF_WORD	Res2;
	ASF_WORD	Name_Length;
	ASF_WCHAR*	Name;
	Markers*	This_Marker;
}Marker_Object;

typedef struct {
	GUID 	ObjectID;
	ASF_QWORD 	ObjectSize;
	GUID	Exclusion_Type;
	ASF_WORD	Stream_Num_Count;
	ASF_WORD*	Stream_Num;
}Bit_Mut_Exc_Object;

typedef struct {
	GUID 	ObjectID;
	ASF_QWORD 	ObjectSize;
	GUID	Err_Cor_Type;
	ASF_DWORD	Err_Cor_Data_Len;
	ASF_BYTE	*Err_Cor_Data;
}Error_Cor_Object;

typedef struct {
	GUID 	ObjectID;
	ASF_QWORD 	ObjectSize;
	ASF_DWORD	Ban_Img_Type;
	ASF_DWORD	Ban_Img_Data_Size;
	ASF_BYTE*	Ban_Img_Data;
	ASF_DWORD	Ban_Img_URL_Len;
	ASF_CHAR*	Ban_Img_URL;
	ASF_DWORD	Copyright_URL_Len;
	ASF_CHAR*	Copyright_URL;
}Content_Brand_Object;

typedef struct {
	GUID 	ObjectID;
	ASF_QWORD 	ObjectSize;
	ASF_DWORD	Secret_Data_Len;
	ASF_BYTE	*Secret_Data;
	ASF_DWORD	Pro_Type_Len;
	ASF_CHAR	*Pro_Type;
	ASF_DWORD	Key_ID_Len;
	ASF_CHAR	*Key_ID;
	ASF_DWORD	License_URL_Len;
	ASF_CHAR	*License_URL;
}Content_Encryption_Object;

typedef struct {
	GUID 	ObjectID;
	ASF_QWORD 	ObjectSize;
	ASF_DWORD	Data_Size;
	ASF_BYTE	*Data;
}Ext_Con_Enc_Object;

typedef struct {
	GUID 	ObjectID;
	ASF_QWORD 	ObjectSize;
	ASF_DWORD	Signature_Type;
	ASF_DWORD	Signature_Data_Len;
	ASF_BYTE	*Signature_Data;
}Digital_Signature_Object;

typedef struct tagAECEO *AECEO_point;
typedef struct tagAECEO{
	GUID 	ObjectID;
	ASF_QWORD 	ObjectSize;
	ASF_DWORD	Crypto_Version;
	ASF_DWORD	Data_Size;
	ASF_BYTE	*Data;
	AECEO_point	pnext;
}Alt_Ext_Con_Enc_Object;

typedef struct tagPadObject *PO_point;
typedef struct tagPadObject{
	GUID 	ObjectID;
	ASF_QWORD 	ObjectSize;
	ASF_BYTE	*Pad_Data;
	PO_point	pnext;	
}Padding_Object;

typedef struct {
	GUID 	ObjectID;
	ASF_QWORD 	ObjectSize;
	ASF_DWORD	NumOfHO;
	ASF_BYTE 	Res1;
	ASF_BYTE 	Res2;
	File_Proper_Object 	FPO;
	Header_Ext_Object	HEO;
	ASF_DWORD	SPO_Count;
	Stream_Proper_Object SPO[3];
	Codec_List_Object	CLO;
}Head_Object;

typedef struct {
	GUID 	ObjectID;
	ASF_QWORD 	ObjectSize;
	ASF_DWORD	NumOfHO;
	ASF_BYTE 	Res1;
	ASF_BYTE 	Res2;
	File_Proper_Object 	FPO;
	Header_Ext_Object	HEO;
	Stream_Proper_Object AudioSPO;
	Stream_Proper_Object VideoSPO;
	Content_Des_Object	CDO;
	Codec_List_Object	CLO;
}Head_Object_For_Record;

typedef struct {
	ASF_BYTE	Err_Cor_Flag;
	ASF_BYTE*	Err_Cor_Data;
}Error_Correction_Data;

typedef union {
	ASF_BYTE	Byte_len;
	ASF_WORD	Word_len;
	ASF_DWORD	Dword_len;
}Choose_Type;

typedef struct {
	ASF_BYTE	Len_Type_Flag;
	ASF_BYTE	Pro_Flag;
	ASF_DWORD	Packet_Length;
	ASF_DWORD	Sequence;
	ASF_DWORD	Padding_Length;
	ASF_DWORD	Send_Time;
	ASF_WORD	Duration;
}Payload_Parsing_Inf;

typedef struct {
	ASF_BYTE 	Stream_number;
	ASF_DWORD	Media_obj_number;
	ASF_DWORD	Offset_media_obj;
	ASF_DWORD	Rep_data_length;
	ASF_BYTE*	Rep_data;
	ASF_DWORD	Payload_Length;
	ASF_BYTE*	Data;
}Multiple_Payload_NC;

typedef struct tagMP Sub_Multi_Payload;

typedef struct {
	ASF_BYTE	Sub_PD_Length;
	Sub_Multi_Payload*	Sub_PD;
}Multi_Sub_Payload_Data;

typedef struct {
	ASF_BYTE 	Stream_number;
	ASF_DWORD	Media_obj_number;
	ASF_DWORD	Offset_media_obj;
	ASF_DWORD	Rep_data_length;
	ASF_BYTE	Presentation_Time_Delta;
	ASF_DWORD	Payload_Length;
	Multi_Sub_Payload_Data*	Sub_M_P_D;
}Multiple_Payload_C;

typedef union {
	Multiple_Payload_NC*	now_Mul_P_N;
	Multiple_Payload_C*		now_Mul_P_C;
}Multi_Payloads;

typedef struct tagMP{
	ASF_BYTE	Payload_Flags;
	ASF_BYTE	Multi_Payload_Type;
	Multi_Payloads*	 M_P_D;
}Multiple_Payload;

typedef struct tagSPN{
	ASF_BYTE 	Stream_number;
	ASF_DWORD	Media_obj_number;
	ASF_DWORD	Offset_media_obj;
	ASF_DWORD	Rep_data_length;
	ASF_BYTE*	Rep_data;
	ASF_DWORD	Data_length;
	ASF_BYTE*	Data;
}Single_Payload_NoCom;

typedef struct tagSPC compress_S_P_C;

typedef union {
	Single_Payload_NoCom*	this_S_P_N;
	compress_S_P_C*	this_S_P_C;
}Sub_Single_Payload;

typedef struct {
	ASF_BYTE	Sub_PD_Length;
	Sub_Single_Payload*	Sub_PD;
}Single_Sub_Payload_Data;

typedef struct tagSPC{
	ASF_BYTE 	Stream_number;
	ASF_DWORD	Media_obj_number;
	ASF_DWORD	Offset_media_obj;
	ASF_DWORD	Rep_data_length;
	ASF_BYTE	Pre_Time_Delta;
	Single_Sub_Payload_Data* Sub_Sin_Pay_Data;
}Single_Payload_Com;

typedef union {
	Single_Payload_NoCom*	Single_Data_NC;
	Single_Payload_Com*		Single_Data_C;
	Multiple_Payload*	Multi_Data;
}Payload_Data;

typedef ASF_BYTE Padding_Data;

typedef struct {
	Error_Correction_Data*	Err_C_D;
	Payload_Parsing_Inf		PP_Inf;
	ASF_BYTE	Payload_Data_Type;
	Payload_Data*	Fulldata;
	Padding_Data*	Pad_Data;
}ASF_Data_Packet;

typedef struct {
	GUID 	ObjectID;
	ASF_QWORD 	ObjectSize;
	GUID 	FileID;
	ASF_QWORD 	TotalPacket;
	ASF_WORD	Res;
	ASF_Data_Packet*	pData_packet;
}Data_Object;


typedef struct {
	ASF_WORD	Format_Tag;
	ASF_WORD	Channel_Number;
	ASF_DWORD	Sample_Per_Second;
	ASF_DWORD	Avg_Byte_Per_Sec;
	ASF_WORD	Block_Align;
	ASF_WORD	Bits_Per_Sample;
	ASF_WORD	Codec_Specific_Data_Size;
	ASF_BYTE*	Codec_Specific_Data;
}Audio_Media_Type;

typedef struct {
	ASF_DWORD	Format_Data_Size;
	ASF_LONG	Image_Width;
	ASF_LONG	Image_Height;
	ASF_WORD	Reserved;
	ASF_WORD	Bits_Per_Pixel_Count;
	ASF_DWORD	Compression_ID;
	ASF_DWORD	Image_Size;
	ASF_LONG	Horizontal_Pixels_Per_Meter;
	ASF_LONG	Vertical_Pixels_Per_Meter;
	ASF_DWORD	Colors_Used_Count;
	ASF_DWORD	Important_Colors_Count;
	ASF_BYTE*	Codec_Specific_Data;
}Image_Data_Format;

typedef struct {
	ASF_DWORD	Encoded_Image_Width;
	ASF_DWORD	Encoded_Image_Height;
	ASF_BYTE	Reserved_Flags;
	ASF_WORD	Format_Data_Size;
	Image_Data_Format	Format_Data;	
}Video_Media_Type;



static const GUID asf_header_object = 
{
    {0x30, 0x26, 0xB2, 0x75, 0x8E, 0x66, 0xCF, 0x11, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C },
};

/*The objects from file_properties_object to padding_object are all included in header object */

static const GUID asf_file_properties_object = 
{
    {0xA1, 0xDC, 0xAB, 0x8C, 0x47, 0xA9, 0xCF, 0x11, 0x8E, 0xE4, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 },
};

static const GUID asf_stream_properties_object = 
{
	{0x91, 0x07, 0xDC, 0xB7, 0xB7, 0xA9, 0xCF, 0x11, 0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 },
};

static const GUID asf_header_extension_object =
{
	{0xB5, 0x03, 0xBF, 0x5F, 0x2E, 0xA9, 0xCF, 0x11, 0x8E, 0xE3, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 },
};

static const GUID asf_codec_list_object = 
{
	{0x40, 0x52, 0xD1, 0x86, 0x1D, 0x31, 0xD0, 0x11, 0xA3, 0xA4, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6 },
};

static const GUID asf_codec_list1_object = 
{
	{0x41, 0x52, 0xd1, 0x86, 0x1d, 0x31, 0xd0, 0x11, 0xa3, 0xa4, 0x00, 0xa0, 0xc9, 0x03, 0x48, 0xf6 },
};

static const GUID asf_script_command_object =
{
	{0x30, 0x1A, 0xFB, 0x1E, 0x62, 0x0B, 0xD0, 0x11, 0xA3, 0x9B, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6 },
};

static const GUID asf_stream_bitrate_properties_object =
{
	{0xCE, 0x75, 0xF8, 0x7B, 0x8D, 0x46, 0xD1, 0x11, 0x8D, 0x82, 0x00, 0x60, 0x97, 0xC9, 0xA2, 0xB2 },
};

static const GUID asf_extended_content_description_object = 
{
	{0x40, 0xA4, 0xD0, 0xD2, 0x07, 0xE3, 0xD2, 0x11, 0x97, 0xF0, 0x00, 0xA0, 0xC9, 0x5E, 0xA8, 0x50 },
};

static const GUID asf_marker_object =
{
	{0x01, 0xCD, 0x87, 0xF4, 0x51, 0xA9, 0xCF, 0x11, 0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 },
};

static const GUID asf_bitrate_mutual_exclusion_object =
{
	{0xDC, 0x29, 0xE2, 0xD6, 0xDA, 0x35, 0xD1, 0x11, 0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE },
};

static const GUID asf_error_correction_object =
{
	{0x35, 0x26, 0xB2, 0x75, 0x8E, 0x66, 0xCF, 0x11, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C },
};

static const GUID asf_content_branding_object =
{
	{0xFA, 0xB3, 0x11, 0x22, 0x23, 0xBD, 0xD2, 0x11, 0xB4, 0xB7, 0x00, 0xA0, 0xC9, 0x55, 0xFC, 0x6E },
};

static const GUID asf_content_description_object = 
{
	{0x33, 0x26, 0xB2, 0x75, 0x8E, 0x66, 0xCF, 0x11, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C },
};

static const GUID asf_content_encryption_object ={
	{0xFB, 0xB3, 0x11, 0x22, 0x23, 0xBD, 0xD2, 0x11, 0xB4, 0xB7, 0x00, 0xA0, 0xC9, 0x55, 0xFC, 0x6E },
};

static const GUID asf_extended_content_encryption_object ={
	{0x14, 0xE6, 0x8A, 0x29, 0x22, 0x26, 0x17, 0x4C, 0xB9, 0x35, 0xDA, 0xE0, 0x7E, 0xE9, 0x28, 0x9C },
};

static const GUID asf_alt_extended_content_encryption_obj ={
	{0xF1, 0x9E, 0x88, 0xFF, 0xEE, 0xAD, 0xDA, 0x40, 0x9E, 0x71, 0x98, 0x70, 0x4B, 0xB9, 0x28, 0xCE },
};

static const GUID asf_digital_signature_object ={
	{0xFC, 0xB3, 0x11, 0x22, 0x23, 0xBD, 0xD2, 0x11, 0xB4, 0xB7, 0x00, 0xA0, 0xC9, 0x55, 0xFC, 0x6E },
};

static const GUID asf_padding_object ={
	{0x74, 0xD4, 0x06, 0x18, 0xDF, 0xCA, 0x09, 0x45, 0xA4, 0xBA, 0x9A, 0xAB, 0xCB, 0x96, 0xAA, 0xE8 },
};

static const GUID asf_data_header = {
	{0x36, 0x26, 0xb2, 0x75, 0x8e, 0x66, 0xcf, 0x11, 0xa6, 0xd9, 0x00, 0xaa, 0x00, 0x62, 0xce, 0x6c },
};

static const GUID asf_audio_stream = {
	{0x40, 0x9E, 0x69, 0xF8, 0x4D, 0x5B, 0xCF, 0x11, 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B },
};

static const GUID asf_video_stream = {
	{0xC0, 0xEF, 0x19, 0xBC, 0x4D, 0x5B, 0xCF, 0x11, 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B },
};

static const GUID asf_reverse_1 = {
	{0x11, 0xD2, 0xD3, 0xAB, 0xBA, 0xA9, 0xCF, 0x11, 0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 },
};

static const GUID asf_reserve_2 = {
	{0x41, 0x52, 0xD1, 0x86, 0x1D, 0x31, 0xD0, 0x11, 0xA3, 0xA4 ,0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6 },
};

static const GUID asf_audio_spread = {
	{0x50, 0xCD, 0xC3, 0xBF, 0x8F, 0x61, 0xCF, 0x11, 0x8B, 0xB2, 0x00, 0xAA, 0x00, 0xB4, 0xE2, 0x20 },
};

static const GUID asf_no_error_correction = {
	{0x00, 0x57, 0xFB, 0x20, 0x55, 0x5B, 0xCF, 0x11, 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B },
};


#define AP do {int n; printf ("f: %s l: %d (s: %d)\n", __FILE__, __LINE__, &n);} while (0)
#endif

