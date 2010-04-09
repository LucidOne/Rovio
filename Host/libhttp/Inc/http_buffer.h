#ifndef __HTTP_BUFFER_H__
#define __HTTP_BUFFER_H__

struct _hash_struct_ {
	char *key;
	char *value;
	struct _hash_struct_ *next;
};
typedef struct _hash_struct_ hash_struct;


#ifndef HTTP_WITH_MALLOC_LEVEL1
// 20 bytes
typedef struct
{
	LIST	listbuf;
	
	LIST_T	list;
	int		ref_count;
}HTTP_LIST_BUF_T;
DECLARE_MEM_POOL (bufLIST, HTTP_LIST_BUF_T)

// 28 bytes
typedef struct
{
	LISTNODE	listnodebuf;
	
	LIST_T	list;
	int		ref_count;
}HTTP_LISTNODE_BUF_T;
DECLARE_MEM_POOL (bufLISTNODE, HTTP_LISTNODE_BUF_T)

// 20 bytes
typedef struct
{
	NAMEDSTRING_T	namestringbuf;
	
	LIST_T	list;
	int		ref_count;
}HTTP_NAMESTRING_BUF_T;
DECLARE_MEM_POOL (bufNAMESTRING, HTTP_NAMESTRING_BUF_T)

// 24 bytes
typedef struct
{
	INNERPART_T	innerpartbuf;
	
	LIST_T	list;
	int		ref_count;
}HTTP_INNERPART_BUF_T;
DECLARE_MEM_POOL (bufINNERPART, HTTP_INNERPART_BUF_T)

// 24 bytes
typedef struct
{
	hash_struct	hashstructbuf;
	
	LIST_T	list;
	int		ref_count;
}HTTP_HASH_BUF_T;
DECLARE_MEM_POOL (bufHASH, HTTP_HASH_BUF_T)

// 24 bytes
typedef struct
{
	EMBEDFUN_T	embedfunbuf;
	
	LIST_T	list;
	int		ref_count;
}HTTP_EMBEDFUN_BUF_T;
DECLARE_MEM_POOL (bufEMBEDFUN, HTTP_EMBEDFUN_BUF_T)

// 0.5k bytes
typedef struct
{
	char	pathbuf[MAX_PATH_LENGTH * 2 + 1];
	
	LIST_T	list;
	int		ref_count;
}HTTP_PATH_BUF_T;
DECLARE_MEM_POOL (bufPATH, HTTP_PATH_BUF_T)

// 5k bytes
typedef struct
{
	request	requestbuf;
	
	LIST_T	list;
	int		ref_count;
}HTTP_REQUEST_BUF_T;
DECLARE_MEM_POOL (bufREQUEST, HTTP_REQUEST_BUF_T)

// 8 bytes
typedef struct
{
	SPLIT_ITEM_T	splititembuf;
	
	LIST_T	list;
	int		ref_count;
}HTTP_SPLIT_ITEM_T;
DECLARE_MEM_POOL (bufSPLITITEM, HTTP_SPLIT_ITEM_T)

// 20 bytes
typedef struct
{
	XML	xmlbuf;
	
	LIST_T	list;
	int		ref_count;
}HTTP_XML_T;
DECLARE_MEM_POOL (bufXML, HTTP_XML_T)
#endif

#endif
