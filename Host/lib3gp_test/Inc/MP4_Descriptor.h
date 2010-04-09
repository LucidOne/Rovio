#ifndef _MP4_DESCRIPTOR_H_
#define _MP4_DESCRIPTOR_H_

/* Object Descriptor Class Tags for Descriptors */
enum 
{
	ObjectDescrTag 				= 0x1,
	InitialObjectDescrTag 		= 0x2,
	ES_DescrTag 				= 0x3,
	DecoderConfigDescrTag 		= 0x4,
	DecSpecificInfoTag 			= 0x5,
	SLConfigDescrTag			= 0x6,
	ContentIdentDescrTag		= 0x7,
	SupplContentIdentDescrTag 	= 0x8,
	IPI_DescrPointerTag			= 0x9,
	IPMP_DescrPointerTag		= 0xA,
	IPMP_DescrTag				= 0xB,
	QoS_DescrTag				= 0xC,
	RegistrationDescrTag		= 0xD,
	ES_ID_IncTag				= 0xE,
	ES_ID_RefTag				= 0xF,
	MP4_IOD_Tag					= 0x10,
	MP4_OD_Tag					= 0x11,
	IPL_DescrPointerRefTag		= 0x12,
	ExtendedProfileLevelDescrTag = 0x13,
	profileLevelIndicationIndexDescrTag = 0x14,
	//0x15-0x3F Reserved for ISO use
	ContentClassificationDescrTag = 0x40,
	KeyWordDescrTag				= 0x41,
	RatingDescrTag				= 0x42,
	LanguageDescrTag			= 0x43,
	ShortTextualDescrTag		= 0x44,
	ExpandedTextualDescrTag		= 0x45,
	ContentCreatorNameDescrTag 	= 0x46,
	ContentCreationDateDescrTag = 0x47,
	OCICreatorNameDescrTag		= 0x48,
	OCICreationDateDescrTag		= 0x49,
	SmpteCameraPositionDescrTag = 0x4A
	//0x4B-0x5F Reserved for ISO use (OCI extensions)
	//0x60-0xBF Reserved for ISO use
	//0xC0-0xFE User private
};


/* Object Descriptor Class Tags for Commands */
enum 
{
	ObjectDescrUpdateTag 	= 0x01,
	ObjectDescrRemoveTag 	= 0x02,
	ES_DescrUpdateTag 		= 0x03,
	ES_DescrRemoveTag 		= 0x04,
	IPMP_DescrUpdateTag 	= 0x05,
	IPMP_DescrRemoveTag 	= 0x06,
	ES_DescrRemoveRefTag 	= 0x07
	//0x08-0xBF Reserved for ISO (command tags)
	//0xC0-0xFE User private
	//0xFF forbidden
};


/* stream Type Value */
enum
{
	ObjectDescriptorStream 	= 0x1,
	ClockReferenceStream 	= 0x2,
	SceneDescriptionStream 	= 0x3,
	VisualStream			= 0x4,
	AudioStream				= 0x5,
	MPEG7Stream				= 0x6,
	IPMPStream				= 0x7,
	ObjectContentInfoStream	= 0x8,
	MPEGJStream				= 0x9
	//0x0A - 0x1F reserved for ISO use
	//0x20 - 0x3F user private
};


#define AUDIO_SPECIFIC_CONFIG	0x00000004	/* AudioSpecificConfig presented */
#define GA_SPECIFIC_CONFIG		0x00000008	/* GASpecificConfig presented */
#define PROGRAM_CONFIG_ELEMENT	0x00000010	/* program_config_element() of GASpecificConfig presented */
#define CELP_SPECIFIC_CONFIG	0x00000020	/* CelpSpecificConfig presented */
#define HVXC_SPECIFIC_CONFIG	0x00000040	/* HvxcSpecificConfig presented */
#define TTS_SPECIFIC_CONFIG		0x00000080	/* TTSSpecificConfig presented */
#define STRUCTURE_AUDIO_CONFIG	0x00000100	/* StructuredAudioSpecificConfig presented */
#define ERCELP_SPECIFIC_CONFIG	0x00000200	/* ErrorResilientCelpSpecificConfig presented */
#define ERHVXC_SPECIFIC_CONFIG	0x00000400	/* ErrorResilientHvxcSpecificConfig presented */
#define PARM_SPECIFIC_CONFIG	0x00000800	/* ParametricSpecificConfig presented */
#define EP_SPECIFIC_CONFIG		0x00001000	/* ErrorProtectionSpecificConfig presented */


/* refer to ISO/IEC 14496-3 sub4.4.1 (p.12), GASpecificConfig() */
typedef struct	
{
	UINT8		frameLengthFlag;
	UINT8		dependsOnCoreCoder;
	UINT16		coreCoderDelay;
	UINT8		extensionFlag;
	UINT8		layerNr;
	UINT8		numOfSubFrame;
	UINT16		layer_length;
	UINT8		aacSectionDataResilienceFlag;
	UINT8		aacScalefactorDataResilienceFlag;
	UINT8		aacSpectralDataResilienceFlag;
	UINT8		extensionFlag3;
	
	/* program_config_element(), only valid when !channelConfiguration */
	UINT8		element_instance_tag;
	UINT8		object_type;
	UINT8		sampling_frequency_index;
	UINT8		num_front_channel_elements;
	UINT8		num_side_channel_elements;
	UINT8		num_back_channel_elements;
	UINT8		num_lfe_channel_elements;
	UINT8		num_assoc_data_elements;
	UINT8		num_valid_cc_elements;
	UINT8		mono_mixdown_present;
	UINT8		mono_mixdown_element_number;
	UINT8		stereo_mixdown_present;
	UINT8		stereo_mixdown_element_number;
	UINT8		matrix_mixdown_idx_present;
	UINT8		matrix_mixdown_idx;
	UINT8		pseudo_surround_enable;
	UINT8		front_element_is_cpe[16];
	UINT8		front_element_tag_select[16];
	UINT8		side_element_is_cpe[16];
	UINT8		side_element_tag_select[16];
	UINT8		back_element_is_cpe[16];
	UINT8		back_element_tag_select[16];
	UINT8		lfe_element_tag_select[4];
	UINT8		assoc_data_element_tag_select[8];
	UINT8		cc_element_is_ind_sw[8];
	UINT8		valid_cc_element_tag_select[8];
	UINT8		comment_field_bytes;
	UINT8		comment_field_data[64];
}	GASpecificConfig_T;


/* refer to ISO/IEC 14496-3 sub3.3.1 (p.6), CelpSpecificConfig() */
typedef struct
{
	UINT8		isBaseLayer;
	UINT8		isBWSLayer;
	UINT8		CELP_BRS_id;
	
	/* CelpHeader */
	UINT8		ExcitationMode;		/* 0:MPE, 1:RPE */
	UINT8		SampleRateMode;
	UINT8		FineRateControl;
	UINT8		RPE_Configuration;
	UINT8		MPE_Configuration;
	UINT8		NumEnhLayers;
	UINT8		BandwidthScalabilityMode;
	
	/* CelpBWSenhHeader */
	UINT8		BWS_configuration;
}	CelpSpecificConfig_T;


/* refer to ISO/IEC 14496-3 sub2.3.1 (p.4), HvxcSpecificConfig() */
typedef struct
{
	UINT8		isBaseLayer;
	
	/* HVXCconfig() */
	UINT8		HVXCvarMode;
	UINT8		HVXCrateMode;
	UINT8		extensionFlag;
}	HvxcSpecificConfig_T;


/* refer to ISO/IEC 14496-3 sub6.4.1 (p.3), TTSSpecificConfig() */
typedef struct
{
	UINT8		TTS_Sequence_ID;
	UINT8		Gender_Enable;
	UINT8		Age_Enable;
	UINT8		Speech_Rate_Enable;
	UINT8		Prosody_Enable;
	UINT8		Video_Enable;
	UINT8		Lip_Shape_Enable;
	UINT8		Trick_Mode_Enable;
	UINT32		Language_Code;
}	TTSSpecificConfig_T;


/* refer to ISO/IEC 14496-3 sub3.3.2 (p.9), ErrorResilientCelpSpecificConfig() */
typedef struct
{
	UINT8		isBaseLayer;
	UINT8		isBWSLayer;
	UINT8		CELP_BRS_id;
	
	/* ER_SC_CelpHeader() */
	UINT8		ExcitationMode;
	UINT8		SampleRateMode;
	UINT8		FineRateControl;
	UINT8		SilenceCompression;
	UINT8		RPE_Configuration;
	UINT8		MPE_Configuration;
	UINT8		NumEnhLayers;
	UINT8		BandwidthScalabilityMode;
	
	/* CelpBWSenhHeader() */
	UINT8		BWS_configuration;
}	ErrCelpSpecificConfig_T;


/* refer to ISO/IEC 14496-3 sub2.3.3 (p.9), ErrorResilientHvxcSpecificConfig() */
typedef struct
{
	UINT8		isBaseLayer;
	
	/* ErHVXCconfig(), 14496-3 sub2.3.3 (p.9) */
	UINT8		HVXCvarMode;
	UINT8		HVXCrateMode;
	UINT8		extensionFlag;
	UINT8		var_ScalableFlag;
}	ErrHvxcSpecificConfig_T;


/* refer to ISO/IEC 14496-3 sub6.4.1 (p.3), TTSSpecificConfig() */
typedef struct
{
	UINT8		isBaseLayer;
	
	/* PARAconfig() */
	UINT8		PARAmode;
	UINT8		PARAextensionFlag;

	/* HILNenexConfig() */
	UINT8		HILNenhaLayer;
	UINT8		HILNenhaQuantMode;
		
	/* ErHVXCconfig(), 14496-3 sub2.3.3 (p.9) */
	UINT8		HVXCvarMode;
	UINT8		HVXCrateMode;
	UINT8		extensionFlag;
	UINT8		var_ScalableFlag;
	
	/* HILNconfig() */
	UINT8		HILNquantMode;
	UINT8		HILNmaxNumLine;
	UINT8		HILNsampleRateCode;
	UINT16		HILNframeLength;
	UINT8		HILNcontMode;
	
}	ParametricSpecificConfig_T;


typedef struct
{
	UINT8		audioObjectType;
	UINT8		channelConfiguration;
	UINT8		samplingFrequencyIndex;
	UINT32		samplingFrequency;
}	AudioSpecificConfig_T;



typedef struct
{
	UINT8		objectTypeIndication;	/* refer to 14496-1, p.30 */
	UINT8		streamType;			/* refer to 14496-1, p.31 */
	UINT8		upStream;			/* indicates that this stream is used for upstream information. */
	//UINT32		uBuffSizeDB;		/* is the size of the decoding buffer for this elementary stream in byte. */
	//UINT32		uMaxBitRate;			/* is the maximum bitrate in bits per second of this elementary stream in any time window of one second uTrackDuration. */
	//UINT32		uAvgBitRate;			/* is the average bitrate in bits per second of this elementary stream. For streams with variable bitrate this value shall be set to zero. */
}	DecoderConfigDescriptor_T;


typedef struct 
{
	UINT16		ES_ID;
	UINT8		streamDependenceFlag;
	UINT8		URL_Flag;
	UINT8		OCRstreamFlag;
	UINT8		streamPriority;
	UINT16		dependsOn_ES_ID;
	UINT8		URLlength;
	//UINT8		*URLstring;
	UINT16		OCR_ES_Id;
	
	UINT32		flag;			/* the presence of the following configuration */
	DecoderConfigDescriptor_T	decConfig;
	AudioSpecificConfig_T		audioConfig;
	GASpecificConfig_T			gaConfig;
	CelpSpecificConfig_T		celpConfig;
	HvxcSpecificConfig_T		hvxcConfig;
	TTSSpecificConfig_T			ttsConfig;
	ErrCelpSpecificConfig_T		errCelpConfig;
	ErrHvxcSpecificConfig_T		errHvxcConfig;
	ParametricSpecificConfig_T	parmConfig;
}	ES_Descriptor_T;


#endif	/* _MP4_DESCRIPTOR_H_ */