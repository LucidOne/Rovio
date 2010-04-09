static char g_acfile_cdir_xml[]=
{
#include "file_cdir_xml.h"
};
static WEB_FILE_T g_pgfile_cdir_xml=
{
	g_acfile_cdir_xml,
	"/cdir.xml",
	sizeof(g_acfile_cdir_xml),
};

static char g_acfile_cmgr_xml[]=
{
#include "file_cmgr_xml.h"
};
static WEB_FILE_T g_pgfile_cmgr_xml=
{
	g_acfile_cmgr_xml,
	"/cmgr.xml",
	sizeof(g_acfile_cmgr_xml),
};

void RegisterWebFiles ()
{
	httpRegisterEmbedFunEx("/cdir.xml", Http_WebFile, AUTH_USER, (void *)&g_pgfile_cdir_xml);
	httpRegisterEmbedFunEx("/cmgr.xml", Http_WebFile, AUTH_USER, (void *)&g_pgfile_cmgr_xml);
}

