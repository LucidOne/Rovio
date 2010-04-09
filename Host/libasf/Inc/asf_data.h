#ifndef __ASF_DATA__
#define __ASF_DATA__

#ifdef ECOS
#include "stdio.h"
#include "stdlib.h"
#else
#include <stdio.h>
#include <stdlib.h>
#endif

#include "asf_types.h"
#include "wbtypes.h"
#include "asf_object.h"
#include "asf_buffer_stream.h"

int err_cor_data_read(ASF_DWORD* payload_len,Error_Correction_Data* e_c_d, Asf_Stream *pStream);
int data_packet_read(ASF_DWORD remain_length,ASF_Data_Packet* data_packet,Asf_Stream *pStream,Video_Stream* video_buffer,ASF_WORD stream_number);
int data_object_read(Asf_Stream *pStream);
int pay_parse_inf_read(ASF_DWORD* payload_len,Payload_Parsing_Inf* pp_inf,Asf_Stream *pStream);
int single_nocom_payload_read(ASF_DWORD* payload_len,ASF_BYTE Specify_char ,Single_Payload_NoCom* rt_s_p, Asf_Stream *pStream,Video_Stream* video_buffer,ASF_WORD stream_number);
int single_payload_read(ASF_DWORD* payload_len,ASF_BYTE Specify_char,Payload_Data* rt_p_d, Asf_Stream *pStream,Video_Stream* video_buffer,ASF_WORD stream_number);
int padding_data_read(ASF_DWORD	pad_data_len,Padding_Data* rt_padding_data, Asf_Stream *pStream);
int sub_single_payload_read(ASF_DWORD* payload_len,ASF_BYTE Specify_char,Sub_Single_Payload* rt_s_p_d, Asf_Stream *pStream,Video_Stream* video_buffer,ASF_WORD stream_number);
int single_com_payload_read(ASF_DWORD* payload_len,ASF_BYTE Specify_char, Single_Payload_Com* rt_s_p, Asf_Stream *pStream,Video_Stream* video_buffer,ASF_WORD stream_number);
int multi_payload_read(ASF_BYTE Specify_char,Multiple_Payload* rt_M_P,Asf_Stream *pStream,Video_Stream* video_buffer,ASF_WORD stream_number);
int multi_nc_payload_read(ASF_BYTE Specify_char,Multiple_Payload_NC* rt_M_P_NC,Asf_Stream *pStream,Video_Stream* video_buffer,ASF_WORD stream_number);
int multi_com_payload_read(ASF_BYTE Specify_char,Multiple_Payload_C* rt_M_P_Com,Asf_Stream *pStream,Video_Stream* video_buffer,ASF_WORD stream_number);

int choose_data_read(ASF_DWORD* payload_len,ASF_BYTE type,ASF_DWORD* ct_value,Asf_Stream *pStream);


int data_packet_write(const Rec_Buffer_Stream *pRecBuffStream, Asf_Stream *pStream);

UINT32 single_packet_header_fill(const Rec_Buffer_Stream* pRecBuffStream,UCHAR padding_length);
UINT32 multi_packet_header_fill(const Rec_Buffer_Stream* pRecBuffStream);
UINT32 video_data_write_sin(UINT32 buff_pos,const Rec_Buffer_Stream* pRecBuffStream,UINT32 video_data_length);
UINT32 single_packet_write(UCHAR padding_length,Rec_Buffer_Stream* pRecBuffStream,UINT32 video_data_length);
UINT32 video_data_write_mul(UINT32 buff_pos,const Rec_Buffer_Stream* pRecBuffStream,UINT32 video_data_length);
UINT32 audio_data_write_mul(UINT32 buff_pos,const Rec_Buffer_Stream* pRecBuffStream,UINT32 audio_data_length);
void multi_payload_number_reset(void);
void packet_padding_size_reset(unsigned char reset_value);
BOOL no_audio_remain(const Rec_Buffer_Stream *pRecBuffStream);
BOOL no_video_remain(const Rec_Buffer_Stream *pRecBuffStream);
UINT32 video_remain_length(const Rec_Buffer_Stream *pRecBuffStream);
void packet_sendtime_reset(void);
void packet_duration_reset(void);
void reset_index_entry(void);
void clean_data_buff(void);




#endif

