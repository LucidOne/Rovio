#ifndef _WBFAT_NTP_H_
#define _WBFAT_NTP_H_


#define JAN_1970   2208988800.0        /* 1970 - 1900 in seconds */
#define NTP_SCALE  4294967296.0        /* 2^32, of course! */

#define NTP_PACKET_MIN       48        /* Without authentication */
#define NTP_PACKET_MAX       68        /* With authentication (ignored) */
#define NTP_DISP_FIELD        8        /* Offset of dispersion field */
#define NTP_REFERENCE        16        /* Offset of reference timestamp */
#define NTP_ORIGINATE        24        /* Offset of originate timestamp */
#define NTP_RECEIVE          32        /* Offset of receive timestamp */
#define NTP_TRANSMIT         40        /* Offset of transmit timestamp */

#define NTP_LI_FUDGE          0        /* The current 'status' */
#define NTP_VERSION           3        /* The current version */
#define NTP_VERSION_MAX       4        /* The maximum valid version */
#define NTP_STRATUM          15        /* The current stratum as a server */
#define NTP_STRATUM_MAX      15        /* The maximum valid stratum */
#define NTP_POLLING           8        /* The current 'polling interval' */
#define NTP_PRECISION         0        /* The current 'precision' - 1 sec. */

#define NTP_PASSIVE           2        /* NTP symmetric passive response */
#define NTP_CLIENT            3        /* NTP client request */
#define NTP_SERVER            4        /* NTP server response */

#define NTP_INSANITY     3600.0        /* Errors beyond this are hopeless */

#ifdef IN_IP_CAMERA
void *NtpThread(void *pParam);
void NtpSetTime(char *pcServer);
#endif

#endif	/* _WBFAT_NTP_H_ */
