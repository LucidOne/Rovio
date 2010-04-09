#ifndef __ASF_INDEX_H__
#define __ASF_INDEX_H__

#include "asf_types.h"
#include "asf_stream.h"
#include "wbtypes.h"

typedef struct
{
	ASF_DWORD	PacketNumber;
	ASF_WORD	PacketCount;
} Simple_Index_Entry;


typedef struct
{
	GUID 		ObjectID;
	ASF_QWORD 	ObjectSize;
	GUID		FileID;
	ASF_QWORD	EntryTimeInterval;
	ASF_DWORD	MaxPacketCount;
	ASF_DWORD	EntriesCount;
	Simple_Index_Entry	*Entries;
} Simple_Index_Object;

static const GUID index_guid = {
	0x90, 0x08, 0x00, 0x33, 0xb1, 0xe5, 0xcf, 0x11, 0x89, 0xf4, 0x00, 0xa0, 0xc9, 0x03, 0x49, 0xcb,
};

int Simple_Index_Object_Read (Simple_Index_Object *pObject, Asf_Stream *pStream);
int Simple_Index_Object_Write (const GUID File_ID, Asf_Stream *pStream);
void Simple_Index_Object_Free (Simple_Index_Object *pObject);
int Index_Entry_Write(Asf_Stream *pStream);
int Index_Entry_First_Half_Write(Asf_Stream *pStream);
int Index_Entry_Second_Half_Write(Asf_Stream *pStream);
BOOL Index_Entry_Remain(void);

#endif

