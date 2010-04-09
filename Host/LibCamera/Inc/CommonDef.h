#ifndef COMMONDEF_H
#define COMMONDEF_H

#define __stdio_h

#include "../../custom_config.h"
#include "IPCamera.h"
#include "ConfigInterface/configinterface.h"
#include "Version.h"
#include "Convert.h"
#include "Status.h"
#include "User.h"
#include "WebCameraLog.h"
#include "wireless/wlan0.h"
#include "Register.h"
#include "Camera/CameraCtl.h"
#include "System.h"

/* Kotech's include */
#include "MCU.h"
#include "NS.h"

#include "PeriodTask.h"
#include "PeriodTasks.h"
#include "Camera/W99702Camera.h"
#include "CameraConfig.h"
#include "Upload/Upload.h"
#include "HttpClient/HttpClient.h"
#include "pipe.h"
#include "TestNetServer.h"
#include "Mail/Mailapp.h"
#include "Mail/mail.h"
#ifdef RECORDER
#include "recorder.h"
#include "Mail/Mailapp.h"
#include "Ftp/Ftpapp.h"
#endif
#include "rtspserver.h"
#include "Ntp/Ntp.h"
#include "BypassClient.h"
#include "Mctest/mctest.h"
#include "WebFiles/web_files.h"
#include "USBConfig/vp_com.h"
#include "TransBack.h"
#include "Reset/IpcamReset.h"
void ServerStart(void);
#define ENABLE_SDIO do { diag_printf("ENABLE_SDIO..\n"); enable_sdio_int(); sdio_enable_SDIO_INT(); diag_printf("ENABLE_SDIO-OK\n"); } while ( 0 )
#define DISABLE_SDIO do { diag_printf("DISABLE_SDIO..\n"); disable_sdio_int(); sdio_disable_SDIO_INT(); diag_printf("DISABLE_SDIO-OK\n"); } while ( 0 )

#endif
