#ifndef ctCTCMPL_U
#define ctCTCMPL_U

#define bigadr(tp,off)	 	(((pTEXT) (tp)) + (off))
#define cpy4bf(dp,sp)    	memcpy(dp,sp,4)
#define cpybig(dp,sp,n)	 	memcpy(dp,sp,(UINT) (n))
#define cpybuf(dp,sp,n)  	memcpy(dp,sp,(UINT) (n))
#define cpylng(dp,sp,n)  	cpy4bf(dp,sp)
#define cpylodl(hdp,sp,n) 	cpylod(hdp,sp,(UINT) (n))
#define cpymbuf			cpybuf
#define cpymsg			cpybuf
#define cpysrcl(dp,hsp,n) 	cpysrc(dp,hsp,(UINT) (n))
#define cpysrcm			cpysrcl
#define ct_NULL			0L
#define ctbfill(dp,ch,n) 	memset(dp,ch,(UINT) (n))
#define ctCONV
#define ctDECL
#define ctEXCLCREAT
#define ctEXPORT
#define ctLOKCNT
#define ctMEM
#define ctmfill			ctbfill
#define ctNOMEMCMP
#define ctPortUNIX
#define ctrt_fclose		fclose
#define ctrt_filcmp		strcmp
#define ctrt_fopen		fopen
#define ctrt_fprintf		fprintf
#define ctrt_fscanf		fscanf
#define ctrt_getcwd		ctGetCwd
#define ctrt_memchr		memchr
#define ctrt_memcmp		memcmp
#define ctrt_printf		printf
#define ctrt_sprintf		sprintf
#define ctrt_strcat		strcat
#define ctrt_strcmp		strcmp
#define ctrt_strcpy		strcpy
#define ctrt_strlen		strlen
#define ctrt_strncmp		strncmp
#define ctrt_strncpy		strncpy
#define ctrt_tmpnam		tmpnam
#define ctrt_toupper		toupper
#define ctsfill(dp,ch,n) 	memset(dp,ch,(UINT) (n))
#define fnCTSBLD
#define icpylng(dp,sp,n) 	*dp++ = *sp++; *dp++ = *sp++; *dp++ = *sp++; *dp++ = *sp++
#define mblllc(n,s)	 	mballc(n,(UINT) (s))
#define PERC_HD

#ifndef SIZEOF
#define SIZEOF 			sizeof
#endif

/*^***********************************/
#ifdef ctSRVR
#define ctDaemon    		become_daemon
#define ctMILLISECOND(t)	(t < 1 ? 0 : (t + 16) / 17)
#define ctUnixLaunch
#define LQFREE	2
#define SERVER_ROOT		"/usr/ctsrv/"

#ifndef ctSRV_DEFAULTCOMM
#ifdef  ctSELECTthrd
#define ctSRV_DEFAULTCOMM	"FSTCPIP"
#else
#define ctSRV_DEFAULTCOMM	"F_TCPIP"
#endif
#endif  /* ctSRV_DEFAULTCOMM */

#else /* ~ctSRVR */

#ifndef ctBNDSRVR
#define ctNOEXCL 
#endif	

#endif /* ~ctSRVR */
/*~***********************************/


/*^***********************************/
#if (defined(ctPortNATIVE_THREADS) && defined(ctSAVEunxFS))
#define COMMIT_DELAY (1L)
#else
#define COMMIT_DELAY (-1L)
#endif
/*~***********************************/

/*^***********************************/
#if (defined(ctPortFAIRCOM_THREADS) || defined(ctPortNATIVE_THREADS))
#include "ctsthd.h"  /* FairCom System Threading Header */
#else
#define OWNER   (1)
#endif
/*~***********************************/

#endif /* ~ctCTCMPL_U */
/* end ctcmpl_u.h */

