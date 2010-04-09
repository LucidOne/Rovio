
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

	iJmp = setjmp(env);
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


int checkdiff(int scale, unsigned char *p1, unsigned char *p2)
{
	int i;
	int sum = 0;
	int tmp = 0;
	for(i = 0; i < scale*scale; i++)
	{
		tmp = (*(p1+i)-*(p2+i));
		sum += (tmp*tmp);
	}

//	printf("result:  %8f\n", sqrt(sum)/(scale*scale));
	return sum;
}



int check_region_deviation(struct diff_region * rgptr,int level, int * diffvalptr,
				int width, int threshold,int sensibility)
{
	int i;
	int  sum_diffval  = 0, sum_deviation = 0;
	int  aver_diffval = 0;

	struct diff_region * curptr = rgptr;

	while(curptr!=NULL)
	{
		if(curptr->lastpos&&(curptr->level==level))
		{
			for(i=0; i<curptr->lastpos; i++)
			{
				sum_diffval += diffvalptr[(curptr->diffval[i]>>4)+(curptr->diffval[i]&0x0f)*width];
			}

			aver_diffval = sum_diffval/curptr->lastpos;

			for(i=0; i<curptr->lastpos; i++)
			{
				sum_deviation += abs(diffvalptr[(curptr->diffval[i]>>4)+(curptr->diffval[i]&0x0f)*width]- aver_diffval);
			}

			curptr->deviation = (100*sum_deviation)/(curptr->lastpos*aver_diffval);

			if(curptr->lastpos<=2)
			{
				switch(sensibility)
				{
					case 1:
						if(aver_diffval*100>225*threshold)
							return 1;break;
					case 2:
						if(aver_diffval>4*threshold)
							return 1;break;
					case 3:
						if(aver_diffval*100>625*threshold)
							return 1;break;
					default:
						{
							fprintf(stderr,"incorrect sensibility\n");
							return -1;
						}
				}
			}
			else
			{
				switch(sensibility)
				{
					case 1:
						if(curptr->deviation > 35)
							return 1;break;
					case 2:
						if(curptr->deviation > 40)
							return 1;break;
					case 3:
						if(curptr->deviation > 45)
							return 1;break;
					default:
						{
							fprintf(stderr,"incorrect sensibility\n");
							return -1;
						}
				}
			}
		}
		curptr = curptr->next;
		sum_diffval = 0;
		sum_deviation = 0;
	}
	return 0;
}




int insert_to_region(int w, int h, int limit,struct diff_region* rgptr)
{
	int i, length;
	struct diff_region * curptr;
	curptr = rgptr;

	if(curptr->diffval == NULL)
	{
		curptr->diffval = malloc(limit*sizeof(unsigned char));
		if(curptr->diffval==NULL)
		{
			PRINT_MEM_OUT;
			return 1;
		}
		memset(curptr->diffval,0,limit*sizeof(unsigned char));
		curptr->diffval[0] = w<<4|h;
		curptr->leftmost = w<<4|h;
		curptr->lastpos++;
		return 0;
	}
	while(curptr != NULL)
	{
		length = curptr->lastpos;
		for(i=0;(curptr->flag == 0)&&i< (length+1)/2;i++)
		{

			if(((abs((curptr->diffval[i]>>4)-w)<=1)&&(abs((curptr->diffval[i]&0x0f)-h)<=1))
				||((abs((curptr->diffval[length-i-1]>>4)-w)<=1)&&(abs((curptr->diffval[length-i-1]&0x0f)-h)<=1)))
			{
				if((h - curptr->diffval[curptr->lastpos-1]&0xf)==1)
					curptr->leftmost = w<<4|h;
				curptr->diffval[curptr->lastpos++] = w<<4|h;

				return 0;
			}
		}

		if(curptr->next != NULL)
			curptr = curptr->next;
		else
		{
			curptr->next = malloc(sizeof(struct diff_region));
			if(curptr->next==NULL)
			{
				PRINT_MEM_OUT;
				return 1;
			}
			memset(curptr->next,0,sizeof(struct diff_region));
			curptr->next->pre = curptr;
			curptr = curptr->next;
			curptr->diffval	= malloc(limit*sizeof(unsigned char));
			if(curptr->diffval==NULL)
			{
				PRINT_MEM_OUT;
				return 1;
			}
			memset(curptr->diffval,0,limit*sizeof(unsigned char));
			curptr->diffval[0] = w<<4|h;
			curptr->lastpos++;
			curptr->leftmost = w<<4|h;
			return 0;
		}

	}
}

int set_region_flag(int h,struct diff_region* rgptr)
{
	int  needinsert;
	struct diff_region* curptr = rgptr;
	needinsert = 0;

	if(curptr->diffval==NULL)
		return 1;

	while(curptr != NULL)
	{
		if( (h - curptr->leftmost&0x0f) > 1 )
			curptr->flag = 1;/*needn't insert new element in such group*/
		else
		{
			curptr->flag = 0;
			needinsert++;
		}
		curptr = curptr->next;

	}

	return needinsert;
}

void set_region_level(int level,struct diff_region* rgptr)
{
	struct diff_region* curptr = rgptr;
	while(curptr != NULL)
	{
		if(curptr->level == 0)
			curptr->level = level;
		curptr = curptr->next;
	}


}

int do_merge(struct diff_region *p1, struct diff_region **pp2){

	int i,length;
	unsigned char w, h;
	struct diff_region *p2 = *pp2;

	if(p1->leftmost>>4 < p2->leftmost>>4)
	{
		w = p2->leftmost>>4;
		h = p2->leftmost&0x0f;
		length = p1->lastpos;
		for(i = 0; i < (length+1)/2; i++)
		{

			if(((abs((p1->diffval[i]>>4)-w)<=1)&&(abs((p1->diffval[i]&0x0f)-h)<=1))
				||((abs((p1->diffval[length-i-1]>>4)-w)<=1)&&(abs((p1->diffval[length-i-1]&0x0f)-h)<=1)))
				break;

		}
		if(i <(length+1)/2)
		{
			//p1 is const , modify p2
			memcpy(p1->diffval+p1->lastpos, p2->diffval, p2->lastpos);
			p1->lastpos += p2->lastpos;

			free(p2->diffval);
			p2->pre->next = p2->next;
			if(p2->next)
				p2->next->pre = p2->pre;
			*pp2 = p2->pre;
			free(p2);

			return 1;
		}
	}
	else
	{
		w = p1->leftmost>>4;
		h = p1->leftmost&0x0f;
		length = p2->lastpos;
		for(i = 0; i < (length+1)/2; i++)
		{

			if(((abs((p2->diffval[i]>>4)-w)<=1)&&(abs((p2->diffval[i]&0x0f)-h)<=1))
				||((abs((p2->diffval[length-i-1]>>4)-w)<=1)&&(abs((p2->diffval[length-i-1]&0x0f)-h)<=1)))
				break;
		}
		if(i < (length+1)/2)
		{
			//p1 is const , modify p2
			memcpy(p1->diffval+p1->lastpos, p2->diffval, p2->lastpos);
			p1->leftmost = p2->leftmost;
			p1->lastpos += p2->lastpos;

			free(p2->diffval);
			p2->pre->next = p2->next;
			if(p2->next)
				p2->next->pre = p2->pre;
			*pp2 = p2->pre;
			free(p2);
			return 1;
		}

	}
	return -1;
}

void merge_region(struct diff_region* rgptr, int level)
{
	struct diff_region* curptr = rgptr;
	struct diff_region* p2 = rgptr->next;

	while(curptr!=NULL)
	{
		if(curptr->level==level)
		{

			while(p2!=NULL)
			{
				if((p2->level==level)
				&&(abs(p2->leftmost&0x0f - curptr->leftmost&0x0f)<=1))
				{
					do_merge(curptr, &p2);
				}
				p2 = p2->next;
			}

		}
		curptr = curptr->next;
		if(curptr!=NULL)
			p2 = curptr->next;
		else
			break;
	}
	return;
}

void rls_region(struct diff_region * rgptr)
{
	struct diff_region *curptr;
	struct diff_region *nextptr;
	curptr = rgptr;
	nextptr = curptr;
	while(curptr!=NULL)
	{
		if(curptr->diffval)
			free(curptr->diffval);
		nextptr = curptr->next;
		free(curptr);
		curptr = nextptr;
	}
}


int cmp(unsigned char *bufp1, unsigned char *bufp2, int blksize,int sensibility)
{
	int  height,width;
	int wn, hn;
	int w, h, i, j;
	int changecount, currentcount = 0;
	unsigned char *blockptr1, *blockptr2;
	unsigned char *dataheader1, *dataheader2;
	unsigned char *datap1, *datap2;
	int *diffvalptr;
	int level, tmp, threshold;
	struct diff_region * regionstart;
	struct diff_region * px;
	int result = -1;

	if(((height=*bufp1)!=*bufp2)||((width=*(bufp1+1))!=*(bufp2+1)))
	{
		printf("can't compare due to the incompatible size \n");
		goto out1;
	}

	dataheader1 = datap1 = bufp1 + 2;
	dataheader2 = datap2 = bufp2 + 2;

	blockptr1 = (unsigned char *)malloc(blksize*blksize*sizeof(unsigned char));
	if(blockptr1 == NULL)
	{
		PRINT_MEM_OUT;
		goto out1;
	}

	blockptr2 =(unsigned char *)malloc(blksize*blksize*sizeof(unsigned char));
	if(blockptr2 == NULL)
	{
		PRINT_MEM_OUT;
		goto out2;
	}
	wn = width/blksize;
	hn = height/blksize;
	diffvalptr = malloc(wn*hn*sizeof(int));
	if(diffvalptr == NULL)
	{
		PRINT_MEM_OUT;
		goto out3;
	}
	memset(diffvalptr, 0, wn*hn*sizeof(int));

	regionstart = malloc(sizeof(struct diff_region));
	if(regionstart == NULL)
	{
		PRINT_MEM_OUT;
		goto out4;
	}
	memset(regionstart,0,sizeof(struct diff_region));

	level = 1;
	threshold = blksize*blksize*blksize*blksize;
	for(h = 0; h < hn; h++)
	{
		if(set_region_flag(h,regionstart)==0)
		{
			set_region_level(level,regionstart);
			if(check_region_deviation(regionstart,level,diffvalptr,wn,threshold,sensibility))
			{
				result = 1;
				goto out5;
			}
			level++;
		}
		merge_region(regionstart,0);

		for(w = 0; w < wn; w++)
		{

			datap1 = dataheader1 + h*width*blksize+w*blksize;
			datap2 = dataheader2 + h*width*blksize+w*blksize;

			for(i = 0; i < blksize; i++)
			{
				for(j = 0; j < blksize; j++)
				{
					*(blockptr1+blksize*i+j) = *(datap1++);
					*(blockptr2+blksize*i+j) = *(datap2++);
				}
				datap1 +=width -blksize;
				datap2 +=width -blksize;
			}

			currentcount = h*wn+w+1;

			if(( tmp = diffvalptr[h*wn+w] = checkdiff(blksize,blockptr1, blockptr2))>threshold)
			{
				switch(sensibility){
					case 1:
						if((100*tmp)>(625*threshold))
						{
							result = 1;
							goto out5;
						}
						break;
					case 2:
						if((tmp)>(9*threshold))
						{
							result = 1;
							goto out5;
						}
						break;
					case 3:
						if((100*tmp)>(1225*threshold))
						{
							result = 1;
							goto out5;
						}
						break;
					default:
						{
							fprintf(stderr,"incorrect sensibility\n");
							goto out5;
						}
				}
				if(insert_to_region(w,h,wn*hn,regionstart))
				{
					goto out5;
				}
			}
		}
	}
	merge_region(regionstart,0);

	if(check_region_deviation(regionstart,0,diffvalptr,wn,threshold,sensibility))
	{
		result = 1;
		goto out5;
	}


#if 0
	for(i =0 ;i<hn;i++)
		{
			for(j=0;j<wn;j++)
			{
				printf("%f ", (sqrt(diffvalptr[i*wn+j])/(blksize*blksize)));
			}
			printf("\n");
		}

	px = regionstart;
	while(px != NULL)
	{
		printf("region info:\n");
		for(i = 0; i < px->lastpos; i++)
		{
			printf("%x  ", px->diffval[i]);
		}
		printf("\n");
		printf("\tregion deviation: %d\n", px->deviation);
		px = px->next;
	}
#endif
	result = 0;
out5:
	rls_region(regionstart);
out4:
	if(diffvalptr!=NULL)
	{
		free(diffvalptr);
		diffvalptr = NULL;
	}
out3:
	if(blockptr2!=NULL) {
		free(blockptr2);
		blockptr2 = NULL;
	}
out2:
	if(blockptr1!=NULL) {
		free(blockptr1);
		blockptr1 = NULL;
	}
out1:
	return result;
}

