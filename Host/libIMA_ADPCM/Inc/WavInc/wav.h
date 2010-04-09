/* wav.h - various structures and defines used by WAV converter. */

#ifndef WAV_H_INCLUDED
#define WAV_H_INCLUDED

#undef ENABLE_GSM
/* purloined from public Microsoft RIFF docs */

/* Private data for .wav file */
typedef struct wavstuff {
    st_size_t	   numSamples;     /* samples/channel reading: starts at total count and decremented  */
    		                   /* writing: starts at 0 and counts samples written */
    st_size_t	   dataLength;     /* needed for ADPCM writing */
    unsigned short formatTag;	   /* What type of encoding file is using */
    unsigned short samplesPerBlock;
    unsigned short blockAlign;
    st_size_t dataStart;  /* need to for seeking */
    
    /* following used by *ADPCM wav files */
    unsigned short nCoefs;	    /* ADPCM: number of coef sets */
    short	  *iCoefs;	    /* ADPCM: coef sets           */
    unsigned char *packet;	    /* Temporary buffer for packets */
    short	  *samples;	    /* interleaved samples buffer */
    short	  *samplePtr;       /* Pointer to current sample  */
    short	  *sampleTop;       /* End of samples-buffer      */
    unsigned short blockSamplesRemaining;/* Samples remaining per channel */    
    int 	   state[16];       /* step-size info for *ADPCM writes */

    /* following used by GSM 6.10 wav */
#ifdef ENABLE_GSM
    gsm		   gsmhandle;
    gsm_signal	   *gsmsample;
    int		   gsmindex;
    int		   gsmbytecount;    /* counts bytes written to data block */
#endif
} *wav_t;


#define	WAVE_FORMAT_UNKNOWN		(0x0000)
#define	WAVE_FORMAT_PCM			(0x0001) 
#define	WAVE_FORMAT_ADPCM		(0x0002)
#define WAVE_FORMAT_IEEE_FLOAT          (0x0003)
#define	WAVE_FORMAT_ALAW		(0x0006)
#define	WAVE_FORMAT_MULAW		(0x0007)
#define	WAVE_FORMAT_OKI_ADPCM		(0x0010)
#define WAVE_FORMAT_IMA_ADPCM		(0x0011)
#define	WAVE_FORMAT_DIGISTD		(0x0015)
#define	WAVE_FORMAT_DIGIFIX		(0x0016)
#define WAVE_FORMAT_DOLBY_AC2           (0x0030)
#define WAVE_FORMAT_GSM610              (0x0031)
#define WAVE_FORMAT_ROCKWELL_ADPCM      (0x003b)
#define WAVE_FORMAT_ROCKWELL_DIGITALK   (0x003c)
#define WAVE_FORMAT_G721_ADPCM          (0x0040)
#define WAVE_FORMAT_G728_CELP           (0x0041)
#define WAVE_FORMAT_MPEG                (0x0050)
#define WAVE_FORMAT_MPEGLAYER3          (0x0055)
#define WAVE_FORMAT_G726_ADPCM          (0x0064)
#define WAVE_FORMAT_G722_ADPCM          (0x0065)

#endif /* WAV_H_INCLUDED */

