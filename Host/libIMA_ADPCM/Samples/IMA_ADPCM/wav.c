/*
 * Microsoft's WAVE sound format driver
 *
 * This source code is freely redistributable and may be used for
 * any purpose.  This copyright notice must be maintained. 
 * Lance Norskog And Sundry Contributors are not responsible for 
 * the consequences of using this software.
 *
 * Change History:
 *
 * November  23, 1999 - Stan Brooks (stabro@megsinet.com)
 *   Merged in gsm support patches from Stuart Daines...
 *   Since we had simultaneously made similar changes in
 *   wavwritehdr() and wavstartread(), this was some
 *   work.  Hopefully the result is cleaner than either
 *   version, and nothing broke.
 *
 * November  20, 1999 - Stan Brooks (stabro@megsinet.com)
 *   Mods for faster adpcm decoding and addition of IMA_ADPCM
 *   and ADPCM  writing... low-level codex functions moved to
 *   external modules ima_rw.c and adpcm.c. Some general cleanup,
 *   consistent with writing adpcm and other output formats.
 *   Headers written for adpcm include the 'fact' subchunk.
 *
 * September 11, 1998 - Chris Bagwell (cbagwell@sprynet.com)
 *   Fixed length bug for IMA and MS ADPCM files.
 *
 * June 1, 1998 - Chris Bagwell (cbagwell@sprynet.com)
 *   Fixed some compiler warnings as reported by Kjetil Torgrim Homme
 *   <kjetilho@ifi.uio.no>.
 *   Fixed bug that caused crashes when reading mono MS ADPCM files. Patch
 *   was sent from Michael Brown (mjb@pootle.demon.co.uk).
 *
 * March 15, 1998 - Chris Bagwell (cbagwell@sprynet.com)
 *   Added support for Microsoft's ADPCM and IMA (or better known as
 *   DVI) ADPCM format for wav files.  Thanks goes to Mark Podlipec's
 *   XAnim code.  It gave some real life understanding of how the ADPCM
 *   format is processed.  Actual code was implemented based off of
 *   various sources from the net.
 *
 * August 10, 2004 - Shirley (clyu2@winbond.com.cn)
 *	 Make this file can work standalone
 *
 * NOTE: Previous maintainers weren't very good at providing contact
 * information.
 *
 * Copyright 1992 Rick Richardson
 * Copyright 1991 Lance Norskog And Sundry Contributors                                                                            
 * Copyright (c) 2005 - 2006 Winbond Electronics Corp. All rights reserved.   
 *
 *
 * Fixed by various contributors previous to 1998:
 * 1) Little-endian handling
 * 2) Skip other kinds of file data
 * 3) Handle 16-bit formats correctly
 * 4) Not go into infinite loop
 *
 * User options should override file header - we assumed user knows what
 * they are doing if they specify options.
 * Enhancements and clean up by Graeme W. Gill, 93/5/17
 *
 * Info for format tags can be found at:
 *   http://www.microsoft.com/asf/resources/draft-ietf-fleischman-codec-subtree-01.txt
 *
 */

#include "wbtypes.h"
#include "string.h"		/* Included for strncmp */
#include "stdlib.h"		/* Included for malloc and free */
#include "stdio.h"
#include "errno.h"

# ifdef HAVE_UNISTD_H
# include "unistd.h"		/* For SEEK_* defines if not found in stdio */
# endif

#include "st_i.h"
#include "wav.h"
#include "ima_rw.h"
#include "adpcm.h"
#ifdef ENABLE_GSM
#include "gsm/gsm.h"
#endif

#undef PAD_NSAMPS
/* #define PAD_NSAMPS */


struct wavstuff	wavhead;

const char *st_sizes_str[] = {
        "NONSENSE!",
        "bytes",
        "shorts",
        "NONSENSE",
        "longs",
        "NONSENSE",
        "NONSENSE",
        "NONSENSE",
        "long longs"
};

const char *st_encodings_str[] = {
        "NONSENSE!",
        "unsigned",
        "signed (2's complement)",
        "u-law",
        "a-law",
	"floating point",
        "adpcm",
        "ima_adpcm",
        "gsm",
	"inversed u-law",
	"inversed A-law",
	"MPEG audio (layer I, III or III)"
};

/*
#if sizeof(struct wavstuff) > PRIVSIZE
#	warn "Uh-Oh"
#endif
*/

static char *wav_format_str();
static uint32_t findChunk(int fp, const char *Label);

/* wavwritehdr:  write .wav headers as follows:
 
bytes      variable      description
0  - 3     'RIFF'
4  - 7     wRiffLength   length of file minus the 8 byte riff header
8  - 11    'WAVE'
12 - 15    'fmt '
16 - 19    wFmtSize       length of format chunk minus 8 byte header 
20 - 21    wFormatTag     identifies PCM, ULAW etc
22 - 23    wChannels      
24 - 27    dwSamplesPerSecond  samples per second per channel
28 - 31    dwAvgBytesPerSec    non-trivial for compressed formats
32 - 33    wBlockAlign         basic block size
34 - 35    wBitsPerSample      non-trivial for compressed formats

PCM formats then go straight to the data chunk:
36 - 39    'data'
40 - 43     dwDataLength   length of data chunk minus 8 byte header
44 - (dwDataLength + 43)   the data

non-PCM formats must write an extended format chunk and a fact chunk:

ULAW, ALAW formats:
36 - 37    wExtSize = 0  the length of the format extension
38 - 41    'fact'
42 - 45    dwFactSize = 4  length of the fact chunk minus 8 byte header
46 - 49    dwSamplesWritten   actual number of samples written out
50 - 53    'data'
54 - 57     dwDataLength  length of data chunk minus 8 byte header
58 - (dwDataLength + 57)  the data


GSM6.10  format:
36 - 37    wExtSize = 2 the length in bytes of the format-dependent extension
38 - 39    320           number of samples per  block 
40 - 43    'fact'
44 - 47    dwFactSize = 4  length of the fact chunk minus 8 byte header
48 - 51    dwSamplesWritten   actual number of samples written out
52 - 55    'data'
56 - 59     dwDataLength  length of data chunk minus 8 byte header
60 - (dwDataLength + 59)  the data
(+ a padding byte if dwDataLength is odd) 


note that header contains (up to) 3 separate ways of describing the
length of the file, all derived here from the number of (input)
samples wav->numSamples in a way that is non-trivial for the blocked 
and padded compressed formats:

wRiffLength -      (riff header) the length of the file, minus 8 
dwSamplesWritten - (fact header) the number of samples written (after padding
                   to a complete block eg for GSM)
dwDataLength     - (data chunk header) the number of (valid) data bytes written

*/

int wavwritehdr(st_signalinfo_t *sinfo, int second_header) 
{
    wav_t	wav = (wav_t) &wavhead;

    /* variables written to wav file header */
    /* RIFF header */    
    uint32_t wRiffLength ;  /* length of file after 8 byte riff header */
    /* fmt chunk */
    uint16_t wFmtSize = 16;       /* size field of the fmt chunk */
    uint16_t wFormatTag = 0;      /* data format */
    uint16_t wChannels;           /* number of channels */
    uint32_t dwSamplesPerSecond;  /* samples per second per channel*/
    uint32_t dwAvgBytesPerSec=0;  /* estimate of bytes per second needed */
    uint16_t wBlockAlign=0;       /* byte alignment of a basic sample block */
    uint16_t wBitsPerSample=0;    /* bits per sample */
    /* fmt chunk extension (not PCM) */
    uint16_t wExtSize=0;          /* extra bytes in the format extension */
    uint16_t wSamplesPerBlock;    /* samples per channel per block */
    /* wSamplesPerBlock and other things may go into format extension */

    /* fact chunk (not PCM) */
    uint32_t dwFactSize=4;	  /* length of the fact chunk */
    uint32_t dwSamplesWritten=0;  /* windows doesnt seem to use this*/

    /* data chunk */
    uint32_t  dwDataLength=0x7ffff000L;	/* length of sound data in bytes */
    /* end of variables written to header */

    /* internal variables, intermediate values etc */
    int bytespersample; /* (uncompressed) bytes per sample (per channel) */
    long blocksWritten = 0;

    dwSamplesPerSecond = sinfo->rate;
    wChannels = sinfo->channels;

    /* Check to see if encoding is ADPCM or not.  If ADPCM
     * possibly override the size to be bytes.  It isn't needed
     * by this routine will look nicer (and more correct)
     * on verbose output.
     */
    if ((sinfo->encoding == ST_ENCODING_ADPCM ||
	 sinfo->encoding == ST_ENCODING_IMA_ADPCM ||
	 sinfo->encoding == ST_ENCODING_GSM) &&
	 sinfo->size != ST_SIZE_BYTE)
    {
		fprintf(stderr, "Overriding output size to bytes for compressed data.\n");
		sinfo->size = ST_SIZE_BYTE;
    }

    switch (sinfo->size)
    {
	case ST_SIZE_BYTE:
	    wBitsPerSample = 8;
	    if (sinfo->encoding != ST_ENCODING_UNSIGNED &&
		    sinfo->encoding != ST_ENCODING_ULAW &&
		    sinfo->encoding != ST_ENCODING_ALAW &&
		    sinfo->encoding != ST_ENCODING_GSM &&
		    sinfo->encoding != ST_ENCODING_ADPCM &&
		   sinfo->encoding != ST_ENCODING_IMA_ADPCM)
	    {
			fprintf(stderr,"Do not support %s with 8-bit data.  Forcing to unsigned\n",st_encodings_str[(unsigned char)sinfo->encoding]);
			sinfo->encoding = ST_ENCODING_UNSIGNED;
	    }
	    break;
	case ST_SIZE_WORD:
	    wBitsPerSample = 16;
	    if (sinfo->encoding != ST_ENCODING_SIGN2)
	    {
			fprintf(stderr,"Do not support %s with 16-bit data.  Forcing to Signed.\n",st_encodings_str[(unsigned char)sinfo->encoding]);
			sinfo->encoding = ST_ENCODING_SIGN2;
	    }
	    break;
	case ST_SIZE_DWORD:
	    wBitsPerSample = 32;
	    if (sinfo->encoding != ST_ENCODING_SIGN2)
	    {
			fprintf(stderr,"Do not support %s with 16-bit data.  Forcing to Signed.\n",st_encodings_str[(unsigned char)sinfo->encoding]);
			sinfo->encoding = ST_ENCODING_SIGN2;
	    }

	    break;
	default:
	    fprintf(stderr,"Do not support %s in WAV files.  Forcing to Signed Words.\n",st_sizes_str[(unsigned char)sinfo->size]);
	    sinfo->encoding = ST_ENCODING_SIGN2;
	    sinfo->size = ST_SIZE_WORD;
	    wBitsPerSample = 16;
	    break;
    }

    wSamplesPerBlock = 1;	/* common default for PCM data */

    switch (sinfo->encoding)
    {
	case ST_ENCODING_UNSIGNED:
	case ST_ENCODING_SIGN2:
	    wFormatTag = WAVE_FORMAT_PCM;
	    bytespersample = (wBitsPerSample + 7)/8;
	    wBlockAlign = wChannels * bytespersample;
	    break;
	case ST_ENCODING_ALAW:
	    wFormatTag = WAVE_FORMAT_ALAW;
	    wBlockAlign = wChannels;
	    break;
	case ST_ENCODING_ULAW:
	    wFormatTag = WAVE_FORMAT_MULAW;
	    wBlockAlign = wChannels;
	    break;
	case ST_ENCODING_IMA_ADPCM:
	    if (wChannels>16)
	    {
			fprintf(stderr,"Channels(%d) must be <= 16\n",wChannels);
			return ST_EOF;
	    }
	    wFormatTag = WAVE_FORMAT_IMA_ADPCM;
	    wBlockAlign = wChannels * 256; /* reasonable default */
	    wBitsPerSample = 4;
	    wExtSize = 2;
	    wSamplesPerBlock = ImaSamplesIn(0, wChannels, wBlockAlign, 0);
	    break;
	case ST_ENCODING_ADPCM:
#ifdef ENABLE_ADPCM
	    if (wChannels>16)
	    {
			st_fail_errno(ft,ST_EOF,"Channels(%d) must be <= 16\n",wChannels);
			return ST_EOF;
	    }
	    wFormatTag = WAVE_FORMAT_ADPCM;
	    wBlockAlign = wChannels * 128; /* reasonable default */
	    wBitsPerSample = 4;
	    wExtSize = 4+4*7;      /* Ext fmt data length */
	    wSamplesPerBlock = AdpcmSamplesIn(0, wChannels, wBlockAlign, 0);
	    break;
#else
	    fprintf(stderr,"sorry, no ADPCM support\n");
	    return ST_EOF;
#endif
	case ST_ENCODING_GSM:
#ifdef ENABLE_GSM
	    if (wChannels!=1)
	    {
		fprintf(stderr,"Overriding GSM audio from %d channel to 1\n",wChannels);
		wChannels = ft->info.channels = 1;
	    }
	    wFormatTag = WAVE_FORMAT_GSM610;
	    /* dwAvgBytesPerSec = 1625*(dwSamplesPerSecond/8000.)+0.5; */
	    wBlockAlign=65;
	    wBitsPerSample=0;  /* not representable as int   */
	    wExtSize=2;        /* length of format extension */
	    wSamplesPerBlock = 320;
#else
	    fprintf(stderr,"sorry, no GSM6.10 support\n");
	    return ST_EOF;
#endif
	    break;
    }
    wav->formatTag = wFormatTag;
    wav->blockAlign = wBlockAlign;
    wav->samplesPerBlock = wSamplesPerBlock;

    if (!second_header) { 	/* adjust for blockAlign */
		blocksWritten = dwDataLength/wBlockAlign;
		dwDataLength = blocksWritten * wBlockAlign;
		dwSamplesWritten = blocksWritten * wSamplesPerBlock;
    } else { 	/* fixup with real length */
		dwSamplesWritten = wav->numSamples;
		switch(wFormatTag)
		{
		case WAVE_FORMAT_ADPCM:
		case WAVE_FORMAT_IMA_ADPCM:
			dwDataLength = wav->dataLength;
			break;
#ifdef ENABLE_GSM
	    case WAVE_FORMAT_GSM610:
		/* intentional case fallthrough! */
#endif
	    default:
			dwSamplesWritten /= wChannels; /* because how rawwrite()'s work */
			blocksWritten = (dwSamplesWritten+wSamplesPerBlock-1)/wSamplesPerBlock;
			dwDataLength = blocksWritten * wBlockAlign;
		}
    }

#ifdef ENABLE_GSM
    if (wFormatTag == WAVE_FORMAT_GSM610)
		dwDataLength = (dwDataLength+1) & ~1; /*round up to even */
#endif

    if (wFormatTag != WAVE_FORMAT_PCM)
	wFmtSize += 2+wExtSize; /* plus ExtData */

    wRiffLength = 4 + (8+wFmtSize) + (8+dwDataLength); 
    if (wFormatTag != WAVE_FORMAT_PCM) /* PCM omits the "fact" chunk */
	wRiffLength += (8+dwFactSize);

    /* dwAvgBytesPerSec <-- this is BEFORE compression, isn't it? guess not. */
    dwAvgBytesPerSec = (double)wBlockAlign*sinfo->rate / (double)wSamplesPerBlock + 0.5;

    /* figured out header info, so write it */
    st_writes(sinfo, "RIFF");
    st_writedw(sinfo, wRiffLength);
    st_writes(sinfo, "WAVE");
    st_writes(sinfo, "fmt ");
    st_writedw(sinfo, wFmtSize);
    st_writew(sinfo, wFormatTag);
    st_writew(sinfo, wChannels);
    st_writedw(sinfo, dwSamplesPerSecond);
    st_writedw(sinfo, dwAvgBytesPerSec);
    st_writew(sinfo, wBlockAlign);
    st_writew(sinfo, wBitsPerSample); /* end info common to all fmts */

    /* if not PCM, we need to write out wExtSize even if wExtSize=0 */
    if (wFormatTag != WAVE_FORMAT_PCM)
		st_writew(sinfo,wExtSize);

    switch (wFormatTag)
    {
		int i;
	case WAVE_FORMAT_IMA_ADPCM:
		st_writew(sinfo, wSamplesPerBlock);
		break;
	case WAVE_FORMAT_ADPCM:
#ifdef ENABLE_ADPCM
		st_writew(sinfo, wSamplesPerBlock);
		st_writew(sinfo, 7); /* nCoefs */
		for (i=0; i<7; i++) {
		    st_writew(sinfo, iCoef[i][0]);
		    st_writew(sinfo, iCoef[i][1]);
		}
#endif
		break;
#ifdef ENABLE_GSM
	case WAVE_FORMAT_GSM610:
		st_writew(sinfo, wSamplesPerBlock);
		break;
#endif
	default:
		break;
    }

    /* if not PCM, write the 'fact' chunk */
    if (wFormatTag != WAVE_FORMAT_PCM){
		st_writes(sinfo, "fact");
		st_writedw(sinfo,dwFactSize); 
		st_writedw(sinfo,dwSamplesWritten);
    }

    st_writes(sinfo, "data");
    st_writedw(sinfo, dwDataLength);		/* data chunk size */

    if (!second_header) {
		fprintf(stderr,"Writing Wave file: %s format, %d channel%s, %d samp/sec\n",
			wav_format_str(wFormatTag), wChannels,
			wChannels == 1 ? "" : "s", dwSamplesPerSecond);
		fprintf(stderr,"        %d byte/sec, %d block align, %d bits/samp\n",
			dwAvgBytesPerSec, wBlockAlign, wBitsPerSample);
    } else {
		fprintf(stderr,"Finished writing Wave file, %u data bytes %u samples\n",
			dwDataLength,wav->numSamples);
#ifdef ENABLE_GSM
	if (wFormatTag == WAVE_FORMAT_GSM610){
	    fprintf(stderr,"GSM6.10 format: %u blocks %u padded samples %u padded data bytes\n",
		    blocksWritten, dwSamplesWritten, dwDataLength);
	    if (wav->gsmbytecount != dwDataLength)
		fprintf(stderr,"help ! internal inconsistency - data_written %u gsmbytecount %u\n",
			dwDataLength, wav->gsmbytecount);

	}
#endif
    }
    return ST_SUCCESS;
}

int checkwavfile(int fp, st_signalinfo_t *sinfo)
{
	wav_t	wav = (wav_t) &wavhead;
	char	magic[5];
    uint32_t	len;
    int		rc;
    /* wave file characteristics */
    uint32_t      dwRiffLength;
    unsigned short wChannels;	    /* number of channels */
    uint32_t      dwSamplesPerSecond; /* samples per second per channel */
    uint32_t      dwAvgBytesPerSec;/* estimate of bytes per second needed */
    uint16_t wBitsPerSample;  /* bits per sample */
    uint32_t wFmtSize;
    uint16_t wExtSize = 0;    /* extended field for non-PCM */

    uint32_t      dwDataLength;    /* length of sound data in bytes */
    st_size_t    bytesPerBlock = 0;
    int    bytespersample;	    /* bytes per sample (per channel */
    char text[256];
    uint32_t      dwLoopPos;
    
    if (st_reads(fp, magic, 4) == ST_EOF || strncmp("RIFF", magic, 4))
    {
		fprintf(stderr,"WAVE: RIFF header not found\n");
		return ST_EOF;
    }

    st_readdw(fp, &dwRiffLength);

    if (st_reads(fp, magic, 4) == ST_EOF || strncmp("WAVE", magic, 4))
    {
		fprintf(stderr,"WAVE header not found\n");
		return ST_EOF;
    }

    /* Now look for the format chunk */
    wFmtSize = len = findChunk(fp, "fmt ");
    /* findChunk() only returns if chunk was found */
    
    if (wFmtSize < 16)
    {
		fprintf(stderr,"WAVE file fmt chunk is too short\n");
		return ST_EOF;
    }

    st_readw(fp, &(wav->formatTag));
    st_readw(fp, &wChannels);
    st_readdw(fp, &dwSamplesPerSecond);
    st_readdw(fp, &dwAvgBytesPerSec);	/* Average bytes/second */
    st_readw(fp, &(wav->blockAlign));	/* Block align */
    st_readw(fp, &wBitsPerSample);	/* bits per sample per channel */
    len -= 16;

    switch (wav->formatTag)
    {
    case WAVE_FORMAT_UNKNOWN:
		fprintf(stderr,"WAVE file is in unsupported Microsoft Official Unknown format.\n");
		return ST_EOF;
	
    //case WAVE_FORMAT_PCM:
	case WAVE_FORMAT_IMA_ADPCM:
		break;
	
	default: 
		return ST_EOF;
	}
	sinfo->channels = wChannels;
	sinfo->rate = dwSamplesPerSecond;

	/* non-PCM formats have extended fmt chunk.  Check for those cases. */
    if (wav->formatTag != WAVE_FORMAT_PCM) {
		if (len >= 2) {
		    st_readw(fp, &wExtSize);
	    	len -= 2;
		} else {
	    	fprintf(stderr,"wave header missing FmtExt chunk\n");
		}
    }
printf("st_wavstartread 07 %d\n",wav->formatTag);
    if (wExtSize > len)
    {
		fprintf(stderr,"wave header error: wExtSize inconsistent with wFmtLen %d",errno);
		return ST_EOF;
    }
    switch (wav->formatTag)
    {
	case WAVE_FORMAT_IMA_ADPCM:
		if (wExtSize < 2)
		{
	    	fprintf(stderr,"format[%s]: expects wExtSize >= %d\n",
		    	wav_format_str(wav->formatTag), 2);
	    	return ST_EOF;
		}

		if (wBitsPerSample != 4)
		{
	    	fprintf(stderr,"Can only handle 4-bit IMA ADPCM in wav files\n");
	    	return ST_EOF;
		}

		st_readw(fp, &(wav->samplesPerBlock));
		bytesPerBlock = ImaBytesPerBlock(sinfo->channels, wav->samplesPerBlock);
		if (bytesPerBlock > wav->blockAlign || wav->samplesPerBlock%8 != 1)
		{
	    	fprintf(stderr,"format[%s]: samplesPerBlock(%d) incompatible with blockAlign(%d)\n",
				wav_format_str(wav->formatTag), wav->samplesPerBlock, wav->blockAlign);
	    	return ST_EOF;
		}

		len -= 2;

		bytespersample = ST_SIZE_WORD;  /* AFTER de-compression */
		break;
	default:
		bytespersample = (wBitsPerSample + 7)/8;
	}
	switch (bytespersample)
    {
	
    case ST_SIZE_BYTE:
		/* User options take precedence */
		sinfo->size = ST_SIZE_BYTE;
	    sinfo->encoding = ST_ENCODING_UNSIGNED;
		break;
	
    case ST_SIZE_WORD:
		sinfo->size = ST_SIZE_WORD;
	    sinfo->encoding = ST_ENCODING_SIGN2;
		break;
	
    case ST_SIZE_DWORD:
		sinfo->size = ST_SIZE_DWORD;
	    sinfo->encoding = ST_ENCODING_SIGN2;
		break;
	
    default:
		fprintf(stderr,"Sorry, don't understand .wav size\n");
		return ST_EOF;
    }
        /* Skip anything left over from fmt chunk */
//    if( fseek(fp,len,SEEK_CUR) == -1 )
	if (fsFileSeek(fp, len, SEEK_CUR) < 0)
		fprintf(stderr, "checkwavfile: fseek error %d\n", errno);

    /* Now look for the wave data chunk */
    dwDataLength = len = findChunk(fp, "data");
    /* findChunk() only returns if chunk was found */

	/* Data starts here */
//	wav->dataStart = ftell(fp);
	return ST_SUCCESS;

}

/****************************************************************************/
/* General Sox WAV file code                                                */
/****************************************************************************/

static uint32_t findChunk(int fp, const char *Label)
{
    char magic[5];
    uint32_t len;
    for (;;)
    {
		if (st_reads(fp, magic, 4) == ST_EOF)
		{
	    	fprintf(stderr,"WAVE file has missing %s chunk\n", Label);
	    	return ST_EOF;
		}
		st_readdw(fp, &len);
		if (strncmp(Label, magic, 4) == 0)
	    	break;		/* Found the data chunk */
	
		if (fsFileSeek(fp, len, SEEK_CUR) < 0)
            fprintf(stderr,"findChunk: fseek error %d\n", errno);
    }
    return len;
}


/*
 * Return a string corresponding to the wave format type.
 */
static char *wav_format_str(unsigned wFormatTag) 
{
	switch (wFormatTag)
	{
		case WAVE_FORMAT_UNKNOWN:
			return "Microsoft Official Unknown";
		case WAVE_FORMAT_PCM:
			return "Microsoft PCM";
		case WAVE_FORMAT_ADPCM:
			return "Microsoft ADPCM";
	        case WAVE_FORMAT_IEEE_FLOAT:
		       return "IEEE Float";
		case WAVE_FORMAT_ALAW:
			return "Microsoft A-law";
		case WAVE_FORMAT_MULAW:
			return "Microsoft U-law";
		case WAVE_FORMAT_OKI_ADPCM:
			return "OKI ADPCM format.";
		case WAVE_FORMAT_IMA_ADPCM:
			return "IMA ADPCM";
		case WAVE_FORMAT_DIGISTD:
			return "Digistd format.";
		case WAVE_FORMAT_DIGIFIX:
			return "Digifix format.";
		case WAVE_FORMAT_DOLBY_AC2:
			return "Dolby AC2";
		case WAVE_FORMAT_GSM610:
			return "GSM 6.10";
		case WAVE_FORMAT_ROCKWELL_ADPCM:
			return "Rockwell ADPCM";
		case WAVE_FORMAT_ROCKWELL_DIGITALK:
			return "Rockwell DIGITALK";
		case WAVE_FORMAT_G721_ADPCM:
			return "G.721 ADPCM";
		case WAVE_FORMAT_G728_CELP:
			return "G.728 CELP";
		case WAVE_FORMAT_MPEG:
			return "MPEG";
		case WAVE_FORMAT_MPEGLAYER3:
			return "MPEG Layer 3";
		case WAVE_FORMAT_G726_ADPCM:
			return "G.726 ADPCM";
		case WAVE_FORMAT_G722_ADPCM:
			return "G.722 ADPCM";
		default:
			return "Unknown";
	}
}

/* Write null-terminated string (without \0). */
static int st_writes(st_signalinfo_t *sinfo, char *c)
{
	int nByteCnt;
	fsWriteFile( sinfo->fp, (UINT8*)c, 1, &nByteCnt);
	if(nByteCnt != strlen(c))
	{
		fprintf(stderr, "write files error %d\n",errno);
		return(ST_EOF);
	}
    return(ST_SUCCESS);
}
/* Write word. */
static int st_writew(st_signalinfo_t *sinfo, uint16_t uw)
{
	int nByteCnt;
	fsWriteFile( sinfo->fp, (UINT8*)&uw, 2, &nByteCnt);
 	if (nByteCnt != 2)
	{
    	fprintf(stderr, "write filew error %d\n",errno);
        return (ST_EOF);
    }
        return(ST_SUCCESS);
}
/* Write double word. */
static int st_writedw(st_signalinfo_t *sinfo, uint32_t udw)
{
	int nByteCnt;
	fsWriteFile( sinfo->fp, (UINT8*)&udw, 4, &nByteCnt);
    if (nByteCnt != 4)
    {
        fprintf(stderr, "write filedw error %d\n",errno);
        return (ST_EOF);
    }
    return(ST_SUCCESS);
}

/* Read and write known datatypes in "machine format".  Swap if indicated.
 * They all return ST_EOF on error and ST_SUCCESS on success.
 */
/* Read n-char string (and possibly null-terminating).
 * Stop reading and null-terminate string if either a 0 or \n is reached.
 */
static int st_reads(int fp, char *c, st_ssize_t len)
{
    char *sc;
    char in;
    int nByteCnt;

    sc = c;
    do
    {
        fsReadFile(fp, (UINT8 *)&in, 1, &nByteCnt);
        if(nByteCnt != 1)
        {
            *sc = 0;
			fprintf(stderr,"Read files error %d",errno);
            return (ST_EOF);
        }
        if (in == 0 || in == '\n')
        {
            break;
        }

        *sc = in;
        sc++;
    } while (sc - c < len);
    *sc = 0;
    return(ST_SUCCESS);
}

/* Read word. */
static int st_readw(int fp, uint16_t *uw)
{
	int nByteCnt;
	fsReadFile(fp, (UINT8 *)uw, 2, &nByteCnt);
    if (nByteCnt != 2)
    {
        fprintf(stderr, "Read filew error %d\n",errno);
        return (ST_EOF);
    }
    return ST_SUCCESS;
}

/* Read double word. */
static int st_readdw(int fp, uint32_t *udw)
{
	int nByteCnt;
	fsReadFile(fp, (UINT8 *)udw, 4, &nByteCnt);
    if (nByteCnt != 4)
    {
		fprintf(stderr, "Read filedw error %d\n",errno);
        return (ST_EOF);
    }
    return ST_SUCCESS;
}



