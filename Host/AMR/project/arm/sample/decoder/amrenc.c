#include <stdio.h>
#include "amrenc.h"

short input_buffer[AMR_ENC_LENGTH];
short output_buffer[AMR_ENC_MAX_PACKET_SIZE];

int main(void)
{
	FILE *rfp, *wfp;
	char *filename = "../../test.pcm";
	char *out_filename = "../../test.amr";
	int len;
	int mode = 7; /* 7=12.2k */
	int size;
	int nframe;


	if( (rfp=fopen(filename,"rb"))==NULL )
	{
		printf("Can't open file %s\n",filename);
		exit(-1);
	}
	if( (wfp=fopen(out_filename,"wb"))==NULL )
	{
		printf("Can't open file %s\n",out_filename);
		exit(-1);
	}
	
	/* To write AMR file header */
	fwrite(AMR_ENC_AMR_FILE_ID, 6, 1, wfp);

	amrInitEncode();
	
	nframe = 0;
	while(1)
	{
		if( (len=fread(input_buffer, sizeof(short), AMR_ENC_LENGTH, rfp)) != AMR_ENC_LENGTH )
		{
			if(len)
			{
				memset(&input_buffer[len], 0, AMR_ENC_LENGTH - len);
			}
			else
				break;
		
		}
	
		amrEncode(mode,0,input_buffer,output_buffer,&size);
	
		fwrite(output_buffer, size, 1, wfp);
		nframe ++;
		fflush(wfp);	
		printf("frame:%6d\n",nframe);
	}
	
	amrFinishEncode();

	return 0;
}