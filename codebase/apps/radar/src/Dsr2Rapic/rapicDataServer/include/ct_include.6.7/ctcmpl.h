/* LINUX 386 compiler setup */

#ifndef ctCTCMPL
#define ctCTCMPL

#include <stdio.h>
#include <fcntl.h>
#include <setjmp.h>
#include <memory.h>
#include <unistd.h>
#include <stdlib.h>

#ifdef ctSRVR
#define SERVER_OSMSG 		"For Linux"
#endif

#ifndef ctPortLINUX
#define ctPortLINUX
#endif
#define C255			-1
#define LOW_HIGH
#define ctDEFER_SLEEP 		usleep
#define ctDEFER_USLEEP_OVERRIDE
#define ctFCNTL
/* #define ctMEMCHRtxt  	pVOID */
#define ctSAVEunxFS
/* #define ctSIZET		unsigned int  */
#define ctrt_exit		exit

#ifdef ctSRVR
#undef ctSELECTthrd
#endif

#include "ctcmpl_u.h" 	/* Unix Specific defines */
#include "ctsysi.h" 	/* FairCom Server ctPortid header file */

#endif /* ~ctCTCMPL */
/* end of ctcmpl.h */

