CC=/usr/local/arm-eabi-toolchain/bin/arm-none-eabi-gcc-4.3.3
CFLAGS = -g -O2
BASE = ../../../../..

all: strtok


#INC = -I ../*/Inc/ -I*/ ../../../Inc/ -I ../../../../../Host/SPI/Inc/
INC = -I /usr/include -I $(BASE)/Baseband/Samples/HIC_Client/Inc -I $(BASE)/Baseband/Libs/CmdSet/Inc -I $(BASE)/Baseband/Libs/HIC/Inc -I $(BASE)/Baseband/Libs/Platform/ads_W90N740/Inc -I $(BASE)/Baseband/Libs/Platform/ads_W90N740/SysLib/rom/Inc -I $(BASE)/Baseband/Libs/Platform/Inc -I $(BASE)/Baseband/Libs/Platform/gcc_W90N740/Inc -I $(BASE)/Baseband/Libs/DMA/Inc -I $(BASE)/Baseband/Libs/LCM/Inc -I $(BASE)/Host/VideoPhone/Inc -I $(BASE)/Host/VideoPhone/Samples/VideoPhoneTest/Inc -I $(BASE)/Host/VideoPhone/Test/VpBitrateControlTest/Inc -I $(BASE)/Host/VPOST/Inc -I $(BASE)/Host/UPnP/Inc -I $(BASE)/Host/libFunc_Through_HIC/Inc -I $(BASE)/Host/libFunc_Through_HIC/Samples/FuncTest/Inc -I $(BASE)/Host/libTinyThread/Inc -I $(BASE)/Host/libTinyThread/Samples/Message/Inc -I $(BASE)/Host/libTinyThread/Samples/Threads/Inc -I $(BASE)/Host/FMI/eCos_new/Inc -I $(BASE)/Host/lib3gp_test/Inc -I $(BASE)/Host/libRemoteFunc/Inc -I $(BASE)/Host/libRemoteFunc/Samples/RemoteNetTest/Inc -I $(BASE)/Host/libRemoteFunc/Samples/RemoteBoundaryTest/Inc -I $(BASE)/Host/libRemoteFunc/Samples/RemoteStressTest/Inc -I $(BASE)/Host/libRemoteFunc/Samples/RemoteFuncTest/Inc -I $(BASE)/Host/libHIC_Host/Inc -I $(BASE)/Host/libHIC_Host/Samples/HICTest/Inc -I $(BASE)/Host/Hash_Pool/Inc -I $(BASE)/Host/Hash_Pool/Sample/Hash_Pool_Test/Inc -I $(BASE)/Host/VPE/Inc -I $(BASE)/Host/JPEG/Inc -I $(BASE)/Host/libCmpImg/Inc -I $(BASE)/Host/libCmpImg/Samples/JpegCmp/Inc -I $(BASE)/Host/LibCamera/Inc -I $(BASE)/Host/LibCamera/Samples/CameraTest/Inc -I $(BASE)/Host/libNS/Inc -I $(BASE)/Host/libList/Inc -I $(BASE)/Host/libRtspServer/Inc -I $(BASE)/Host/libRtspServer/Sample/Sample1/Inc -I $(BASE)/Host/libMemPool/Inc -I $(BASE)/Host/libFtp/Inc -I $(BASE)/Host/libAULaw/Inc -I $(BASE)/Host/libAULaw/Samples/TestAULaw/Inc -I $(BASE)/Host/libAULaw/Samples/ToneCompare/Inc -I $(BASE)/Host/WBFAT/ecos/Inc -I $(BASE)/Host/libIMA_ADPCM/Inc -I $(BASE)/Host/libBMP/Inc -I $(BASE)/Host/libUSB/Inc -I $(BASE)/Host/libUSB/Sample/TestUSB/Inc -I $(BASE)/Host/SPI/Inc -I $(BASE)/Host/wpa_supplicant/Inc -I $(BASE)/Host/libhttp/Inc -I $(BASE)/Host/libhttp/Samples/TestHttp/Inc -I $(BASE)/Host/libhttp/Test/SingleFunTest/Inc -I $(BASE)/Host/libLCDDisplay/Inc -I $(BASE)/Host/libLCDDisplay/Sample/TestLCM/Inc -I $(BASE)/Host/SysLib/Inc -I $(BASE)/Host/Audio/Inc -I $(BASE)/Host/powerctrl/Inc -I $(BASE)/Host/I2C/Inc -I $(BASE)/Host/libMemMgmt/Inc -I $(BASE)/Host/libMail/Inc -I $(BASE)/Host/libFont/Inc -I $(BASE)/Host/libFont/Samples/FontBypass/Inc -I $(BASE)/Host/libFont/Samples/TestLCM/Inc -I $(BASE)/Host/libasf/Inc -I $(BASE)/Host/libeCos/include

OBJS =

all: CameraTest
	@if exist * echo Build completed

rebuild: clean all

.PHONY: clean
clean:
	if exist *.o del *.o

CameraTest: CameraTest.c
	$(CC) $(CFLAGS) $(INC) $^ -o $@
#CameraTest: CameraTest.c -lm

