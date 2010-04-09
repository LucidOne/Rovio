#ifndef C_STRING_H
#define C_STRING_H

#ifndef BOOL
#define BOOL char
#define TRUE 1
#define FALSE 0
#endif

typedef struct
{
	char *pcName;
	char *pcValue;
} NAMEDSTRING_T;

char *httpMyStrncpy(char *pcDest, const char *pcSrc, int iLength);

/* 将pcString按cSeparator作为分割，存放在一个LIST结构的双向链表中 */
LIST *httpSplitString(const char *pcString, char cSeparator);

/* 释放SplitString函数生成的链表及其中的字符串资源 */
void httpDeleteSplitString(LIST *pList);

/* 得到一个去除前后空白的字符串，用完需要free */
char *httpGetTrimString(const char *pcString);

/* 得到一个经C转义化的字符串，用完需要free
** 如ab"cd, 会得到ab\"cd */
char *httpGetCStyleString(const char *pcString);
void httpDeleteCStyleString(char *pcString);

/* 得到一个经web转义化的字符串，用完需要free
**   ab<cd, 会得到ab&lt;cd */
char *httpGetWebStyleString(const char *pcString);

/* 得到一个先经web转义化，再经C/java转义化的字符串，用完要free */
char *httpGetWebInCString(const char *pcStr);
void httpDeleteWebInCString(char *pcStr);

/* 将字符串转换为IP地址 */
unsigned long httpString2IP(const char *pcString);
/* 将IP地址转换为字符串, pcStringOut空间必须>=16, 返回pcStringOut字符串的长度 */
int httpIP2String(unsigned long ulIP, char *pcStringOut);

/* 将字符串转换为long */
long httpString2Long(const char *pcString);
unsigned long httpString2ULong(const char *pcString);
/* 将long转换为字符串, pcStringOut空间必须>=32, 返回pcStringOut字符串的长度 */
int httpLong2String(long lVal, char *pcStringOut);

/* 将字符串转换为BOOL */
BOOL httpString2Bool(const char *pcString);
/* 将BOOL转换为字符串, pcStringOut空间必须>=6("true"或"false"), 返回pcStringOut字符串的长度 */
int httpBool2String(BOOL bVal, char *pcStringOut);

/* 将十六进制字符串转换为对应的ASC字符,如"41"转化为A */
char httpHex2Char(const char *str);
/* 将ASC字符转换为十六进制字符串, 返回2 */
int httpChar2Hex(char ch, char *pcRtHex);


/* 创建一个字典 */
LIST *httpCreateDict(void);

/* 销毁一个字典 */
void httpDeleteDict(LIST *pList);

/* 从字典中删除一项 */
BOOL httpDelDictParam(LIST *pList, const char *pcName);

/* 检查一个字典(NAMEDSTRING_T, Name,Value对)的链表中是否含有指定的项
** 如果不含有,下面函数的返回值为:
	httpGetString()	""
	httpGetIP()		0
	httpGetIP4()	0
	GetInt32()	0
	httpGetBool()	0
*/
LISTNODE *httpIsExistParam(const LIST *pList, const char *pcName);

/* 以下四函数根据pcName从list链表中取得值，
** 该链表中每个节点元素为NAMEDSTRING_T(Name,Value对)类型 */
const char *httpGetString(const LIST *pList, const char *pcName);
unsigned long httpGetIP(const LIST *pList, const char *pcName);
unsigned long httpGetIP4(const LIST *pList, const char *pcName);
long httpGetLong(const LIST *pList, const char *pcName);
unsigned long httpGetULong(const LIST *pList, const char *pcName);
BOOL httpGetBool(const LIST *pList, const char *pcName);

/* 以下四函数重新设置list中pcName和pcValue */
LISTNODE *httpSetString(LIST *pList, const char *pcName, const char *pcValue);
LISTNODE *httpSetIP(LIST *pList, const char *pcName, unsigned long ulValue);
LISTNODE *httpSetLong(LIST *pList, const char *pcName, long lValue);
LISTNODE *httpSetBool(LIST *pList, const char *pcName, BOOL bValue);


/* Define XML structure */
typedef struct tagXML
{
	struct tagXML *pParXML;
	LISTNODE *pParXMLNode;

	char *pcName;
	LIST *plAttrib;
	LIST *plSubXML;
} XML;

XML *httpCreateXML(const char *pcName);
XML *httpCreateXMLText(const char *pcText);
void httpDeleteXML(XML *pXML);
XML *httpInsertXMLBefore(XML *pXML, XML *pInsert);
XML *httpInsertXMLAfter(XML *pXML, XML *pInsert);
XML *httpAppendXML(XML *pXML, XML *pSubXML);
XML *httpDetachXML(XML *pXML, XML *pSubXML);
XML *httpGetParentXML(XML *pXML);
void httpDumpXML(XML *pXML, void (*Dumper)(const char *, void *), void *pParam);

#endif
