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

int data_object_read_copy(Asf_Stream *pStream,ASF_QWORD	*audio_length,ASF_WORD	stream_number);

int data_packet_read_copy(ASF_DWORD remain_length,ASF_Data_Packet* data_packet,Asf_Stream *pStream,ASF_QWORD *audio_length,ASF_QWORD *video_frame_count,ASF_DWORD *video_frame_number,ASF_WORD	stream_number);
int single_nocom_payload_read_copy(ASF_DWORD* payload_len,ASF_BYTE Specify_char ,Single_Payload_NoCom* rt_s_p, Asf_Stream *pStream,ASF_QWORD *audio_length,ASF_QWORD *video_frame_count,ASF_DWORD *video_frame_number,ASF_WORD stream_number);
int single_payload_read_copy(ASF_DWORD* payload_len,ASF_BYTE Specify_char,Payload_Data* rt_p_d, Asf_Stream *pStream,ASF_QWORD *audio_length,ASF_QWORD *video_frame_count,ASF_DWORD *video_frame_number,ASF_WORD stream_number);
int sub_single_payload_read_copy(ASF_DWORD* payload_len,ASF_BYTE Specify_char,Sub_Single_Payload* rt_s_p_d, Asf_Stream *pStream,ASF_QWORD *audio_length,ASF_QWORD *video_frame_count,ASF_DWORD *video_frame_number,ASF_WORD stream_number);
int single_com_payload_read_copy(ASF_DWORD* payload_len,ASF_BYTE Specify_char, Single_Payload_Com* rt_s_p, Asf_Stream *pStream,ASF_QWORD *audio_length,ASF_QWORD *video_frame_count,ASF_DWORD *video_frame_number,ASF_WORD stream_number);
int multi_payload_read_copy(ASF_BYTE Specify_char,Multiple_Payload* rt_M_P,Asf_Stream *pStream,ASF_QWORD *audio_length,ASF_QWORD *video_frame_count,ASF_DWORD *video_frame_number,ASF_WORD stream_number);
int multi_nc_payload_read_copy(ASF_BYTE Specify_char,Multiple_Payload_NC* rt_M_P_NC,Asf_Stream *pStream,ASF_QWORD *audio_length,ASF_QWORD *video_frame_count,ASF_DWORD *video_frame_number,ASF_WORD stream_number);
int multi_com_payload_read_copy(ASF_BYTE Specify_char,Multiple_Payload_C* rt_M_P_Com,Asf_Stream *pStream,ASF_QWORD *audio_length,ASF_QWORD *video_frame_count,ASF_DWORD *video_frame_number,ASF_WORD stream_number);

int err_cor_data_read_copy(ASF_DWORD* payload_len,Error_Correction_Data* e_c_d, Asf_Stream *pStream);

int pay_parse_inf_read_copy(ASF_DWORD* payload_len,Payload_Parsing_Inf* pp_inf,Asf_Stream *pStream);
int choose_data_read_copy(ASF_DWORD* payload_len,ASF_BYTE type,ASF_DWORD* ct_value,Asf_Stream *pStream);
int padding_data_read_copy(ASF_DWORD pad_data_len,Padding_Data* rt_padding_data, Asf_Stream *pStream);

BOOL pay_parse_inf_write(Payload_Parsing_Inf getppinf,Asf_Buffer_Stream* rtstream);
BOOL data_object_write(Data_Object d_o,Asf_Buffer_Stream* rt);
UINT32 cal_pay_data_len(Single_Payload_NoCom get_payload_data,Payload_Parsing_Inf getppinf);
BOOL single_payload_nocom_write(Single_Payload_NoCom get_payload_data,Payload_Parsing_Inf ppinf,Asf_Buffer_Stream* rtstream);


UINT32 get_chose_type_size(UINT32 signal);
BOOL write_choose_type(UINT8 gettype,Asf_Buffer_Stream* rtstream,Choose_Type content);





BOOL DATA_GUID_Write(Asf_Buffer_Stream* forguid,GUID forwrite);
BOOL DATA_QWORD_Write(Asf_Buffer_Stream* forguid,ASF_QWORD forwrite);
BOOL DATA_DWORD_Write(Asf_Buffer_Stream* forguid,ASF_DWORD forwrite);
BOOL DATA_WORD_Write(Asf_Buffer_Stream* forguid,ASF_WORD forwrite);



#endif

