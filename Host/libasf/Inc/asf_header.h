#ifndef __ASF_HEADER__
#define __ASF_HEADER__

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

BOOL checkoutguid(GUID sourceguid, GUID sobject);
int compareGUID(GUID sourceguid);


int asf_read_h_o(Head_Object* h_o,Asf_Stream *pStream);
int asf_read_f_p_o(File_Proper_Object* f_p_o, Asf_Stream *pStream);
int asf_read_h_e_o(Header_Ext_Object* h_e_o, Asf_Stream *pStream);
int asf_read_s_p_o(ASF_DWORD SPO_Num,Stream_Proper_Object* s_p_o, Asf_Stream *pStream);
int asf_read_c_d_o(Content_Des_Object* c_d_o, Asf_Stream *pStream);
int asf_read_c_l_o(Codec_List_Object* c_l_o, Asf_Stream *pStream);
int asf_read_e_c_d_o(Ex_Content_Des_Object* e_c_d_o, Asf_Stream *pStream);
int asf_read_s_b_p_o(Str_Bit_Pro_Object* s_b_p_o, Asf_Stream *pStream);
int asf_read_s_c_o(Script_Command_Object* s_c_o, Asf_Stream *pStream);
int asf_read_m_o(Marker_Object* m_o, Asf_Stream *pStream);
int asf_read_b_m_e_o(Bit_Mut_Exc_Object* b_m_e_o, Asf_Stream *pStream);
int asf_read_e_c_o(Error_Cor_Object* e_c_o, Asf_Stream *pStream);
int asf_read_c_b_o(Content_Brand_Object* c_b_o, Asf_Stream *pStream);
int asf_read_c_e_o(Content_Encryption_Object* c_e_o, Asf_Stream *pStream);
int asf_read_e_c_e_o(Ext_Con_Enc_Object* e_c_e_o, Asf_Stream *pStream);
int asf_read_d_s_o(Digital_Signature_Object* d_s_o, Asf_Stream *pStream);
int asf_read_a_e_c_e_o(ASF_DWORD AECEO_Num,Alt_Ext_Con_Enc_Object* a_e_c_e_o, Asf_Stream *pStream);
int asf_read_p_o(ASF_DWORD PO_Num,Padding_Object* p_o, Asf_Stream *pStream);



int str_bit_pro_object_write(const Str_Bit_Pro_Object *pObject, Asf_Stream *pStream);
int file_pro_object_write(const File_Proper_Object *pObject, Asf_Stream *pStream);
int header_ext_object_write(const Header_Ext_Object *pObject, Asf_Stream *pStream);
int head_object_write(const Head_Object_For_Record *pObject, Asf_Stream *pStream);
int stream_pro_object_write(const Stream_Proper_Object *pObject, Asf_Stream *pStream);
int codec_list_object_write(const Codec_List_Object *pObject, Asf_Stream *pStream);
int ex_con_des_object_write(const Ex_Content_Des_Object *pObject, Asf_Stream *pStream);
int content_des_object_write(const Content_Des_Object *pObject, Asf_Stream *pStream);

ASF_WORD get16unit(unsigned char* sourcebuffer);
ASF_DWORD get32unit(unsigned char* sourcebuffer);



#endif

