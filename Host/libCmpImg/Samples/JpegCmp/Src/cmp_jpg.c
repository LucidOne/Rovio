
#include "cmp_common.h"
#include "cdjpeg.h"		/* Common decls for cjpeg/djpeg applications */
#include "jversion.h"		/* for version message */

#include <ctype.h>		/* to declare isprint() */
//#include <math.h>

#ifdef USE_CCOMMAND		/* command-line reader for Macintosh */
#ifdef __MWERKS__
#include <SIOUX.h>              /* Metrowerks needs this */
#include <console.h>		/* ... and this */
#endif
#ifdef THINK_C
#include <console.h>		/* Think declares it here */
#endif
#endif


#include <setjmp.h>

static void Mem_Init_Source(struct jpeg_decompress_struct *pCInfo);
static boolean Mem_Fill_Input_Buffer(struct jpeg_decompress_struct *pCInfo);
static void Mem_Skip_Input_Data(struct jpeg_decompress_struct *pCInfo, long lBytes);
static void Mem_Term_Source(struct jpeg_decompress_struct *pCInfo);
void Jpeg_Memory_Src(struct jpeg_decompress_struct *pCInfo, char *pcInput, int iLenInput);
static void My_Error_Exit(struct jpeg_common_struct *pCInfo);


/* Create the add-on message string table. */

#define JMESSAGE(code,string)	string ,

static const char * const cdjpeg_message_table[] = {
#include "cderror.h"
  NULL
};



typedef struct {
  struct djpeg_dest_struct pub;	/* public fields */

  boolean is_os2;		/* saves the OS2 format request flag */

  jvirt_sarray_ptr whole_image;	/* needed to reverse row order */
  JDIMENSION data_width;	/* JSAMPLEs per row */
  JDIMENSION row_width;		/* physical width of one row in the BMP file */
  int pad_bytes;		/* number of padding bytes needed per row */
  JDIMENSION cur_output_row;	/* next row# to write to virtual array */
} bmp_dest_struct;

typedef bmp_dest_struct * bmp_dest_ptr;


struct diff_region {
	unsigned char flag;
	unsigned char lastpos;
	unsigned char leftmost;
	unsigned char level;
	unsigned char *diffval;
	int deviation;
	struct 	diff_region * pre;
	struct 	diff_region * next;

};




/*
 * Marker processor for COM and interesting APPn markers.
 * This replaces the library's built-in processor, which just skips the marker.
 * We want to print out the marker as text, to the extent possible.
 * Note this code relies on a non-suspending data source.
 */

LOCAL(unsigned int)
jpeg_getc (j_decompress_ptr cinfo)
/* Read next byte */
{
  struct jpeg_source_mgr * datasrc = cinfo->src;

  if (datasrc->bytes_in_buffer == 0) {
    if (! (*datasrc->fill_input_buffer) (cinfo))
      ERREXIT(cinfo, JERR_CANT_SUSPEND);
  }
  datasrc->bytes_in_buffer--;
  return GETJOCTET(*datasrc->next_input_byte++);
}


METHODDEF(boolean)
print_text_marker (j_decompress_ptr cinfo)
{
  boolean traceit = (cinfo->err->trace_level >= 1);
  INT32 length;
  unsigned int ch;
  unsigned int lastch = 0;

  length = jpeg_getc(cinfo) << 8;
  length += jpeg_getc(cinfo);
  length -= 2;			/* discount the length word itself */

  if (traceit) {
    if (cinfo->unread_marker == JPEG_COM)
      fprintf(stderr, "Comment, length %ld:\n", (long) length);
    else			/* assume it is an APPn otherwise */
      fprintf(stderr, "APP%d, length %ld:\n",
	      cinfo->unread_marker - JPEG_APP0, (long) length);
  }

  while (--length >= 0) {
    ch = jpeg_getc(cinfo);
    if (traceit) {
      /* Emit the character in a readable form.
       * Nonprintables are converted to \nnn form,
       * while \ is converted to \\.
       * Newlines in CR, CR/LF, or LF form will be printed as one newline.
       */
      if (ch == '\r') {
	fprintf(stderr, "\n");
      } else if (ch == '\n') {
	if (lastch != '\r')
	  fprintf(stderr, "\n");
      } else if (ch == '\\') {
	fprintf(stderr, "\\\\");
      } else if (isprint(ch)) {
	putc(ch, stderr);
      } else {
	fprintf(stderr, "\\%03o", ch);
      }
      lastch = ch;
    }
  }

  if (traceit)
    fprintf(stderr, "\n");

  return TRUE;
}

void set_cinfo(j_decompress_ptr cinfo)
{
	cinfo->out_color_space = JCS_GRAYSCALE;
	cinfo->scale_num = 1;
	cinfo->scale_denom = 8;

	return;
}


unsigned char * put_gray_to_buff(j_decompress_ptr cinfo, djpeg_dest_ptr dinfo, unsigned char **ppout)
{

	bmp_dest_ptr dest = (bmp_dest_ptr) dinfo;
	JSAMPARRAY image_ptr;
	register JSAMPROW data_ptr;
	JDIMENSION row;
	register JDIMENSION col;
	unsigned char * bufp;
	unsigned char * bufheader;

	bufheader = bufp = malloc(cinfo->output_height*dest->row_width*sizeof(unsigned char)+2);
	if (bufheader == NULL)
	{
		 PRINT_MEM_OUT;
		 return NULL;
	}
	if (ppout != NULL) *ppout = bufheader;

	*bufp = cinfo->output_height;
	*(++bufp) = dest->row_width;



  	for (row = cinfo->output_height; row > 0; row--) {
  		image_ptr = (*cinfo->mem->access_virt_sarray)
  		((j_common_ptr) cinfo, (jvirt_sarray_ptr) dest->whole_image, row-1, (JDIMENSION) 1, FALSE);
  		data_ptr = image_ptr[0];
  		for (col = dest->row_width; col > 0; col--) {
  			*(++bufp)=*data_ptr++;
  		}
  	}
  	return bufheader;
}
typedef struct {
	struct jpeg_source_mgr pub;	/* public fields */
	char *pcInput;
	int iInputLen;
	int iInputRead;
	char acBuffer[4];
} Mem_Source_Mgr;

static void Mem_Init_Source(struct jpeg_decompress_struct *pCInfo)
{
}

static boolean Mem_Fill_Input_Buffer(struct jpeg_decompress_struct *pCInfo)
{
	Mem_Source_Mgr *pMemSrc = (Mem_Source_Mgr *)pCInfo->src;
	int iBytesThis = pMemSrc->iInputLen - pMemSrc->iInputRead;

	if (iBytesThis > 0)
	{
		pMemSrc->pub.next_input_byte = pMemSrc->pcInput + pMemSrc->iInputRead;
		pMemSrc->pub.bytes_in_buffer = iBytesThis;
		pMemSrc->iInputRead += iBytesThis;
	}
	else
	{
		if (pMemSrc->iInputRead == 0)	/* Treat empty input file as fatal error */
			ERREXIT(pCInfo, JERR_INPUT_EMPTY);
		WARNMS(pCInfo, JWRN_JPEG_EOF);
		/* Insert a fake EOI marker */
		pMemSrc->acBuffer[0] = (JOCTET)0xFF;
		pMemSrc->acBuffer[1] = (JOCTET)JPEG_EOI;
		pMemSrc->pub.next_input_byte = pMemSrc->acBuffer;
		pMemSrc->pub.bytes_in_buffer = 2;
	}
	return TRUE;
}

static void Mem_Skip_Input_Data(struct jpeg_decompress_struct *pCInfo, long lBytes)
{
	if (lBytes > 0)
	{
		if (lBytes > pCInfo->src->bytes_in_buffer)
			lBytes = pCInfo->src->bytes_in_buffer;

		pCInfo->src->next_input_byte += lBytes;
		pCInfo->src->bytes_in_buffer -= lBytes;
fprintf(stderr, "%d %d %d\n", pCInfo->src->next_input_byte,
	pCInfo->src->bytes_in_buffer,lBytes);
	}
}

static void Mem_Term_Source(struct jpeg_decompress_struct *pCInfo)
{
}

void Jpeg_Memory_Src(struct jpeg_decompress_struct *pCInfo, char *pcInput, int iInputLen)
{
	Mem_Source_Mgr *pMemSrc;

	if (pCInfo->src == NULL)
	{
		pCInfo->src = (struct jpeg_source_mgr *)
			(*pCInfo->mem->alloc_small) ((j_common_ptr) pCInfo, JPOOL_PERMANENT, SIZEOF(Mem_Source_Mgr));
		pMemSrc = (Mem_Source_Mgr *) pCInfo->src;
		pMemSrc->pcInput = pcInput;
		pMemSrc->iInputLen = iInputLen;
		pMemSrc->iInputRead = 0;
	}

	pMemSrc = (Mem_Source_Mgr *) pCInfo->src;
	pMemSrc->pcInput = pcInput;
	pMemSrc->iInputLen = iInputLen;

	pMemSrc->pub.init_source = Mem_Init_Source;
	pMemSrc->pub.fill_input_buffer = Mem_Fill_Input_Buffer;
	pMemSrc->pub.skip_input_data = Mem_Skip_Input_Data;
	pMemSrc->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
	pMemSrc->pub.term_source = Mem_Term_Source;
	pMemSrc->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
	pMemSrc->pub.next_input_byte = NULL; /* until buffer loaded */
}

jmp_buf env;

static void My_Error_Exit (struct jpeg_common_struct *pCInfo)
{
	/* Always display the message */
	(*pCInfo->err->output_message) (pCInfo);

	/* Let the memory manager delete any temp files before we die */
	jpeg_destroy(pCInfo);

//	fprintf(stderr, "所有会退出的错误都走到这里, 可否longjmp ?\n");
	PTE;
	longjmp(env, -1);
	exit(EXIT_FAILURE);
}


int djpeg (char *inbuf,int inbuflen,unsigned char **ppout)
{
  struct jpeg_decompress_struct cinfo ;
  struct jpeg_error_mgr jerr ;

  djpeg_dest_ptr dest_mgr = NULL;
  JDIMENSION num_scanlines;

  if (ppout == NULL) return -1;
  *ppout = NULL;

  /* Initialize the JPEG decompression object with default error handling. */
  cinfo.err = jpeg_std_error(&jerr);
  cinfo.err->error_exit = My_Error_Exit;

  jpeg_create_decompress(&cinfo);
  /* Add some application-specific error messages (from cderror.h) */
  jerr.addon_message_table = cdjpeg_message_table;
  jerr.first_addon_message = JMSG_FIRSTADDONCODE;
  jerr.last_addon_message = JMSG_LASTADDONCODE;

  /* Insert custom marker processor for COM and APP12.
   * APP12 is used by some digital camera makers for textual info,
   * so we provide the ability to display it as text.
   * If you like, additional APPn marker types can be selected for display,
   * but don't try to override APP0 or APP14 this way (see libjpeg.doc).
   */
  jpeg_set_marker_processor(&cinfo, JPEG_COM, print_text_marker);
  jpeg_set_marker_processor(&cinfo, JPEG_APP0+12, print_text_marker);


  set_cinfo(&cinfo);

  Jpeg_Memory_Src(&cinfo, inbuf, inbuflen);

  /* Read file header, set default decompression parameters */
  (void) jpeg_read_header(&cinfo, TRUE);

  /* Adjust default decompression parameters by re-parsing the options */

  set_cinfo(&cinfo);
  /* Initialize the output module now to let it override any crucial
   * option settings (for instance, GIF wants to force color quantization).
   */
  dest_mgr = jinit_write_bmp(&cinfo, FALSE);

  /* Start decompressor */
  (void) jpeg_start_decompress(&cinfo);

  /* Write output file header */
  (*dest_mgr->start_output) (&cinfo, dest_mgr);

  /* Process data */
  while (cinfo.output_scanline < cinfo.output_height) {
    num_scanlines = jpeg_read_scanlines(&cinfo, dest_mgr->buffer,
					dest_mgr->buffer_height);
    (*dest_mgr->put_pixel_rows) (&cinfo, dest_mgr, num_scanlines);
  }


  /* Finish decompression and release memory.
   * I must do it in this order because output module has allocated memory
   * of lifespan JPOOL_IMAGE; it needs to finish before releasing memory.
   */
  put_gray_to_buff(&cinfo,dest_mgr,ppout);	 //toview
  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);


  return 0;
}

int djpeg_ex (char *inbuf, int inbuflen, unsigned char **ppout)
{
	int iJmp;
	int iRt;

	PTE;
	iJmp = setjmp(env);
	PTE;
	if (iJmp == 0)
	{
		*ppout = 0;
		iRt = djpeg(inbuf, inbuflen, ppout);
		if(*ppout==NULL)
		iRt = -1;
	}
	else
	{
		if (*ppout != NULL)
		{
			free(*ppout);
			*ppout = NULL;
		}
		iRt = -1;
	}

	return iRt;
}


