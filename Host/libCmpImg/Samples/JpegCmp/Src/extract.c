#include <stdio.h>
#include <stdlib.h>
#include "cmp_common.h"

struct rectangleinfo{
	int 	x1;
	int 	y1;
	int 	x2;
	int 	y2;

};

struct rstlineinfo{
	int span;
	int headoff;
	int tailoff;
};


#define MAX(a,b)	((a) > (b) ? (a) : (b))

char *
extract (const unsigned char *inbuf, int inbuflen, struct rectangleinfo * rectangleinfo, int *outlength)
{
	const unsigned char *mcudata;
	char *outbuf;
	int i, j, extendsize, count;
	int mcu_height, mcu_width, rst_width;
	int new_x1, new_x2, new_y1, new_y2;
	int random_x1, random_x2, random_y1, random_y2;
	unsigned int x_off, y_off,block;
	short tmp;
	struct rstlineinfo * rstinfo;
	int blocksize[4]={32,40,48,64};
	short rst_val_array[8]={0xd0ff,0xd1ff,0xd2ff,0xd3ff,0xd4ff,0xd5ff,0xd6ff,0xd7ff};
	int  toread_bytes=0, sof_pos=0,cur_pos=0;
	int width = 0 , height = 0, interval = 0, max_v_factor = 0, max_h_factor = 0, length = 0;
	char endstring[2]={0xff,0xd9};

	random_x1 = rectangleinfo->x1;
	random_x2 = rectangleinfo->x2;
	random_y1 = rectangleinfo->y1;
	random_y2 = rectangleinfo->y2;

	for(i=0;i<inbuflen;i++)
	{
		if(i+5 < inbuflen && inbuf[i]==0xff&&inbuf[i+1]==0xdd)
		{
			interval = interval |inbuf[i+4];
			interval = (interval <<8) |inbuf[i+5];
		}
		if(i+9 < inbuflen && inbuf[i]==0xff&&inbuf[i+1]==0xc0)
		{
			sof_pos = i;
			height = height |inbuf[i+5];
			height = (height <<8) |inbuf[i+6];
			width = width |inbuf[i+7];
			width = (width <<8) |inbuf[i+8];
			for(j=0;j<inbuf[i+9];j++)
			{
				int k = j*3 + i + 11;
				if (k < inbuflen)
				{
					max_v_factor = MAX(max_v_factor,inbuf[k]&0x0f);
					max_h_factor = MAX(max_h_factor,(inbuf[k]>>4));
				}
			}
		}
		if(i+3 < inbuflen && inbuf[i]==0xff&&inbuf[i+1]==0xda)
		{
			length = length |inbuf[i+2];
			length = (length <<8) |inbuf[i+3];
			length += i+2;
			break;
		}
	}

	if (interval == 0
		|| sof_pos == 0
		|| length == 0 || length >= inbuflen
		|| sof_pos+8 >= length
		|| max_v_factor == 0 || max_h_factor == 0)
		return NULL;
	switch(width)
	{
		case 640 :
			block = blocksize[3]; break;
		case 352 :
			block = blocksize[2]; break;
		case 320 :
			block = blocksize[1]; break;
		case 176 :
			block = blocksize[3]; break;
		default :
			return NULL;

	}
	x_off = max_h_factor*interval*8;
	y_off = max_v_factor*8;
	mcu_height = (height + y_off- 1)/y_off;
	mcu_width  = (width + max_h_factor*8 - 1 )/(max_h_factor*8);
	rst_width  = (mcu_width + interval -1)/ interval;
	new_x1 = random_x1/x_off;
	new_y1 = random_y1/y_off;

#if 0
	if(random_x2 < (new_x1+1)*max_h_factor*interval*8)
		new_x2 = new_x1 + 1 ;
	else
		new_x2 = random_x2/(max_h_factor*interval*8);

	if(random_y2 < (new_y1+1)*max_v_factor*8)
		new_y2 = new_y1 + 1;
	else
		new_y2 = random_y2/(max_v_factor*8);
#else
	if(block<x_off)
	{
		if(random_x2-random_x1 < x_off )
			new_x2 = new_x1 + 1 ;
		else
			new_x2 = random_x2/x_off;
	}
	else
	{
		if(random_x2-random_x1 < block)
		{
			new_x2 = new_x1 + (block+x_off-1)/x_off ;
			if((new_x2+1)*x_off>width)
			{
				new_x2 = width/x_off - 1;
				new_x1 = new_x2 - (block+x_off-1)/x_off;
			}
		}
		else
			new_x2 = random_x2/x_off;

	}

	if(block<y_off)
	{
		if(random_y2-random_y1 < y_off )
			new_y2 = new_y1 + 1 ;
		else
			new_y2 = random_y2/y_off;
	}
	else
	{
		if(random_y2-random_y1 < block)
		{
			new_y2 = new_y1 + (block+y_off-1)/y_off ;
			if((new_y2+1)*y_off>height)
			{
				new_y2 = height/y_off - 1;
				new_y1 = new_y2 - (block+y_off-1)/y_off;
			}
		}
		else
			new_y2 = random_y2/y_off;

	}

#endif
#if 0
	tmp = (new_x2 - new_x1)*(max_h_factor*interval*8);
	inbuf[sof_pos+8]= *((char *)&tmp);
	inbuf[sof_pos+7]= *((char *)&tmp + 1);
	tmp = (new_y2 - new_y1)*(max_v_factor*8);
	inbuf[sof_pos+6]= *((char *)&tmp);
	inbuf[sof_pos+5]= *((char *)&tmp + 1);
#endif

	if ((outbuf = malloc(length*sizeof(char))) == NULL)
	{
		PRINT_MEM_OUT;
		return NULL;
	}
	memcpy(outbuf,inbuf,length);
	toread_bytes = inbuflen-length-1;

#if 1
	tmp = (new_x2 - new_x1)*(max_h_factor*interval*8);
	outbuf[sof_pos+8]= *((char *)&tmp);
	outbuf[sof_pos+7]= *((char *)&tmp + 1);
	tmp = (new_y2 - new_y1)*(max_v_factor*8);
	outbuf[sof_pos+6]= *((char *)&tmp);
	outbuf[sof_pos+5]= *((char *)&tmp + 1);
#endif

	mcudata = &inbuf[length];
	if ((rstinfo = malloc(mcu_height*rst_width*sizeof(struct rstlineinfo))) == NULL)
	{
		PRINT_MEM_OUT;
		free(outbuf);
		return NULL;
	}

	count = 0;
	for (i = 0;i<toread_bytes;i++)
	{
		if((mcudata[i]==0xff)&&(mcudata[i+1]>=0xd0)&&(mcudata[i+1]<=0xd7)&&count<mcu_height*rst_width-1)
		{

			count++;
			if(count == 1)
			{
				rstinfo[count - 1].headoff = 0;
				rstinfo[count].headoff = i + 2;
			}
			else
				rstinfo[count].headoff = i+2;
			rstinfo[count-1].tailoff = i-1;
		}
		if(mcudata[i]==0xff&&mcudata[i+1]==0xd9)
			{
			rstinfo[count-1].tailoff = i-1;
				break;
			}
	}
	count = 0;
	extendsize = 0;
	for(i=new_y1;i<new_y2;i++)
	{
		for(j=new_x1;j<new_x2;j++)
		{
			rstinfo[i*rst_width+j].span=rstinfo[i*rst_width+j].tailoff - rstinfo[i*rst_width+j].headoff + 1;
			extendsize +=rstinfo[i*rst_width+j].span;
			count++;
		}
	}

	{
		char *newoutbuf;
		if ((newoutbuf = realloc(outbuf,(length+extendsize+count*2+2)*sizeof(char))) == NULL)
		{
			PRINT_MEM_OUT;
			free(outbuf);
			free(rstinfo);
			return NULL;
		}
		else outbuf = newoutbuf;
	}

	count = 0;
	for(i=new_y1;i<new_y2;i++)
	{
		for(j=new_x1;j<new_x2;j++)
		{

			cur_pos = i*rst_width+j;
			memcpy(&outbuf[length],(mcudata+rstinfo[cur_pos].headoff),rstinfo[cur_pos].span);
			length += rstinfo[cur_pos].span;
			memcpy(&outbuf[length],rst_val_array+count%8,2);
			length +=2;
			count++;
		}
	}

	memcpy(&outbuf[length],endstring,sizeof(char)*2);
	free(rstinfo);

	*outlength = length+2;

	return outbuf;
}

#if 0
unsigned char *ReadStream(FILE *pInFile, int *piLenInput)
{
	unsigned char *pcRt;
	int i, iRead, iReadTotal;

	for (i=0, pcRt=NULL, iReadTotal=0; ; i++)
	{
		pcRt = realloc(pcRt, 1024*(i+1));
		if (pcRt == NULL) return NULL;

		iRead = fread(pcRt+1024*i, 1, 1024, pInFile);
		if (iRead > 0) iReadTotal += iRead;
		if (iRead < 1024) break;
	}
	if (piLenInput != NULL) *piLenInput = iReadTotal;
	return pcRt;
}

int main(int argc, char **argv)
{
	FILE * input_file;
	FILE * output_file;
	struct rectangleinfo selectarea;
	unsigned char * inbuf=NULL;
	char *outbuf=NULL;
	int inbuflen,outlength;
	sscanf(argv[1], "%d,%d,%d,%d",
	       &selectarea.x1, &selectarea.y1, &selectarea.x2, &selectarea.y2);

	if((input_file = fopen(argv[2],"r+b")) == NULL)
	{
		fprintf(stderr,"can't open %s\n", argv[2]);
		exit(1);
	}
	if((output_file = fopen(argv[3],"w+b")) == NULL)
	{
		fprintf(stderr,"can't open %s\n", argv[3]);
		exit(1);
	}


	inbuf = ReadStream(input_file, &inbuflen);

	outbuf = extract(inbuf, inbuflen, &selectarea,&outlength);

	fwrite(outbuf,outlength,1,output_file);

	free(inbuf);
	free(outbuf);

	fclose(input_file);
	fclose(output_file);

	return 0;

}
#endif
