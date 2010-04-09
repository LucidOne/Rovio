#ifndef __ASF_DEC__
#define __ASF_DEC__

#include "asf_object.h"
#include "asf_buffer_stream.h"

#define FAST_DIV	20
#define FAST_MULTI	10

#define FPO_FOUND	353
#define SPO_FOUND	354
#define HEO_FOUND	355
#define CLO_FOUND	356
#define CL1O_FOUND	357
#define SCO_FOUND	358
#define MO_FOUND	359
#define BMEO_FOUND	360
#define ECO_FOUND	361
#define CDO_FOUND	362
#define ECDO_FOUND	363
#define SBPO_FOUND	364
#define CBO_FOUND	365
#define CEO_FOUND	366
#define ECEO_FOUND	367
#define AECEO_FOUND	368
#define DSO_FOUND	369
#define PO_FOUND	370


//int asf_header_object_init(Head_Object* h_o);

int asf_read_h_o(Head_Object* h_o,Asf_Stream *pStream);

int asf_read_f_p_o(File_Proper_Object* f_p_o, Asf_Stream *pStream);
int asf_read_h_e_o(Header_Ext_Object* h_e_o, Asf_Stream *pStream);
int asf_read_s_p_o(ASF_DWORD SPO_Num,Stream_Proper_Object* s_p_o, Asf_Stream *pStream);
int asf_read_c_l_o(Codec_List_Object* c_l_o, Asf_Stream *pStream);

BOOL checkoutguid(GUID sourceguid, GUID sobject);

ASF_WORD get16unit(unsigned char* sourcebuffer);
ASF_DWORD get32unit(unsigned char* sourcebuffer);

int data_packet_read(ASF_DWORD remain_length,ASF_Data_Packet* data_packet,Asf_Stream *pStream,Video_Stream* video_buffer,ASF_WORD stream_number);
//int data_packet_read_copy(ASF_DWORD remain_length,ASF_Data_Packet* data_packet,Asf_Stream *pStream,ASF_QWORD *audio_length,ASF_QWORD *video_frame_count,ASF_DWORD *video_frame_number,ASF_WORD	stream_number);
int compareGUID(GUID sourceguid);

typedef enum Type { Type_imaadpcm, Type_mp3, Type_wma, Type_other }	Type_t;

#endif