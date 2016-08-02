/******************************************************************************/
/******************************************************************************/
/* Function Name:
    lirpCommon.h 

   Purpose:
     This defines the structures of the messages passed between the
     server and the client.  

   Formal Parameters:
     None.

   Global Variables:
     

   Function Calls:
     None.
    
   Change History:
     7/25/03   Darcy Saxion   When collecting data with the maximum allowable
                              range bins, the server had a "Memory Fault".
                              Corrected MAX_DATAMSG_SIZE and moved to this
                              include file.  First, MAXBINS was 2048 - with the
                              addition of the bin for the burst pulse, we really
                              recieve 2049 range bins.  MAXBINS is defined in
                              rvp8.h.  Problems with adding 1 in a macro.  KEEP
                              the parenthesis in the definition of the macro
                              MAXLIRPBINS!
*/
/******************************************************************************/
/******************************************************************************/

/* rvp8.h is included for the I/Q data and header data structure */
#include "sigtypes.h"
#include "dsp.h"
#include "rvp8.h"
#include "user_lib.h"
#include "dsp_lib.h"
/*#include "/home/dsaxion/sigmet.8.01.13/rda/rvp8main/site/rvp8old.h" */

/* **********************************************************************
 * *                                                                    *
 * *          Parameter and Structure Definitions for the RVP8          *
 * *                                                                    *
 * **********************************************************************
 * File: include/rvp8old.h
 *
 *                        COPYRIGHT (c) 2001  BY
 *          SIGMET INCORPORATED, WESTFORD MASSACHUSETTS, U.S.A.
 * 
 * THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED
 * ONLY  IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
 * INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE  OR  ANY OTHER
 * COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
 * OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY
 * TRANSFERED.
 */

/* ***********************************************
 * *                                             *
 * *  RVP8 Live Time Series Shared Data Segment  *
 * *                                             *
 * ***********************************************
 */

/* ------------------------------
 * Shared structure for live (I,Q) pulse headers and time series data
 * that are being acquired by the RVP8 master threads.  These data are
 * accessed by any number of readers in an evesdropping manner via the
 * RVP8TS API.  The memory is writable only by the RVP8 master.
 */
/* Pulse header structure definition when data was collected at S-POL */
/* with LIRP prior to the 8.02.1 release                              */
struct rvp8PulseHdrOld {		/* Header information for each pulse */
  UINT1 lValid ;		  /* Header plus associated data are all valid */
  UINT1 iMajorMode ;		  /* RVP8 major mode, one of PMODE_xxx */
  UINT2 iMSecUTC ;		  /* MiliSeconds absolute time */
  UINT4 iTimeUTC ;		  /* Seconds absolute time */
  UINT4 iSeqNum ;		  /* Sequence number by which pulses are referenced */
  UINT4 iSysTime ;		  /* AQCLK1X (approx 36MHz) time of arrival */
  UINT4 iBtime ;		  /* System BTime (milliseconds) of arrival */
  UINT4 iPrevPRT, iNextPRT ;	  /* SysTime ticks to previous/next pulse */
  SINT4 iDataOffs[MAXVIQPERBIN] ; /* Data offset in fIQ[] for each Rx channel */
  BIN2  iAz, iEl ;		  /* Antenna azimuth and elevation */
  SINT2 iNumVecs ;		  /* Actual (I,Q) vectors (burst+data) for each Rx, */
  SINT2 iMaxVecs ;		  /*   and maximum number (which sets data stride). */
  UINT2 iWrapIQ ;		  /* Data wraparound counter for validity checks */
  UINT1 iAqMode ;		  /* Sequence numbers for acquisition mode */
  UINT1 iTgBank ;		  /* Trigger bank number (one of TBANK_xxx) */
  UINT2 iTgWave ;		  /* Trigger waveform sequence number within a bank */
  UINT1 iVIQPerBin ;		  /* (I,Q) vectors/bin, i.e., # of Rx channels */
  char pad51x1[1] ;
  struct uiqbits uiqPerm ;	  /* User specified bits (Permanent) */
  struct uiqbits uiqOnce ;	  /* User specified bits (One-Shot) */
  FLT4 fBurstMags[MAXVIQPERBIN] ; /* Burst pulse magnitudes for each Rx channel */
  BIN2 iBurstArgs[MAXVIQPERBIN] ; /* Burst phase changes (PrevPulse - ThisPulse) */
} ;



#define  STD_MSG_SIZE  80       /* Size of message buffer to print to stdout */
#define  INFINITY      -1       /* Arbitrary value for infinity */


/* Maximum number of range bins per pulse - number of I/Q pairs per pulse    */
/* MAXBINS is the maximum number of bins defined in rvp8.h - add one for     */
/* burst pulse                                                               */
#define  MAXLIRPBINS (RVP8MAXBINS+1)

/* Maximum number of pulses received from a data read (rvp8tsGetPulses) */
#define  MAXPULSES   40 

/* Maximum number of errors allowed before stopping trying to collect data
 *     from the rvp8.  For now keep the number small to see errors quicker.  */
#define  MAXERROR  100 


/* Size read buffer to be able to hold entire data message.  It depends on */
/* the size of the message header, the pulse data header, the largest data */
/* format (float), maximum number of range bins, and the maximum number of */
/* pulses requested from the rvp8 at one time.                             */

#define  MAX_DATAMSG_SIZE  (DATAINFO + ((sizeof(struct pulseData) + (sizeof (float) * 2 * MAXLIRPBINS)) * MAXPULSES)) 



/******************************************************************************/


/* #define VERBOSE  */
#ifdef VERBOSE 
#define  LOG_MSG() if (LogFp != NULL) { fprintf (LogFp, "%s", LogMsg); fflush(LogFp); }
#define  PRT_MSG() { fprintf (stdout, "%s", LogMsg); fflush(stdout); }
#else
#define  LOG_MSG()
#define  PRT_MSG()
#endif

/* define debug constants */
#ifdef TESTING
#define  DEBUG_MSG() if (LogFp != NULL) fprintf (LogFp, "%s", LogMsg)
#define  DEBUGPRT_MSG() fprintf (stdout, "%s", LogMsg)
#else
#define  DEBUG_MSG()
#define  DEBUGPRT_MSG()
#endif
/******************************************************************************/

#define VERSIONONE  1    /* Compatible with RDA-8.01-14 and prior */
#define VERSION  2    /* This is the **current** version 
                         -- Compatible with RDA-8.02.1 and after */
#ifdef BENDIAN
#define DATAID 0x52565453
#else
#define DATAID 0x53545652
#endif

/*
 *  Message type definitions *
 */
#define	MSG_INFO	1
#define	MSG_CNTL	2
#define	MSG_DATA	3

/*
 * Information message severity flag definitions *
 */
#define	INFO_ERR	   1
#define	INFO_WARN   2

/*
 *	Information message detail flag definitions
 */
#define  SUCCESS           0     
#define  FAILURE          -1   

/*  ERROR CODES are found in lirpErrors.h */

/*
 * Control message action definitions
 *    defines what kind of control message
 */
#define	CNTL_STOP		1
#define	CNTL_QUIT		2
#define	CNTL_DONE		3 
#define	CNTL_DATA_REQ	4 

/*
 * Control message data request boundary definition types 
 *  !!!!  this method allows one type of selection at a time only !!!!
 *  !!!!  will we want to, say, collect a defined sector for 10 minutes? !!!
 */
#define  DRTIME   1  /* select using time as criteria */
#define  DRSECT   2  /* select a sector */
#define  DRVCP    3  /* select a VCP */

/**********************  Message Structue definitons ************************/
/*
 *  Information Message Definition  
 *  These messages contain warning and error information 
 */
struct msgInfo  {
   UINT1		type ;    /* indicates Information message */
	UINT1		severity ;  /* warning or error message */
	UINT1		detail ;    /* what kind of error or warning */
   UINT1    filler;     /* make structure an even number of bytes */
   UINT4    expSeqNum;     /* make structure an even number of bytes */
   UINT4    newSeqNum;     /* make structure an even number of bytes */
} ; /* msgInfo */

/*
 *  Control Message Definition 
 *  These messages contain the controlling commands 
 */
struct msgCntl  {
	UINT1 	type ;             /* indicates Control message */  
	UINT1 	action ;             /* what kind of control message - stop, etc */
	UINT1 	boundType ;          /* if data request, define way of selecting */
   UINT1    packedData;          /* Flag indicating packed data is requested */
	time_t 	duration ;           /* if selecting by time, duration in HH:MM */
	float 	begRange, endRange ; /* select by area, begin and end range */
	float		begAz, endAz ;       /* select by area, begin and end azimuth */
	float		begEl, endEl ;       /* select by area, begin and end elevation */
	int		numVCPs ;            /* select by number of VCPs to record */
} ; /* msgCntl */

/*
 *  Data Message Definition 
*/

/* information about the data associated with each pulse worth of IQ data */
struct pulseData  { 
   unsigned int id;
   UINT4 recordSize;            /* size of this data record in bytes */
   UINT4 version;               /* version of data structure */
   SINT4 packedData;            /* packed data format? 1 => Low SNR Format  */
                                /*                     2 => High SNR Format */
                                /*             format defined in rvp8 setup */
#define LOWSNRPACK (1)
#define HIGHSNRPACK (2)
   UINT2 startGate;             /* beginning range bin of data */
   UINT2 endGate;               /* ending gate of data */
   UINT4 spare[27];             /* spares for future growth */
   struct rvp8PulseHdr header; /* header info describing I/Q environment */
/* struct rvp8PulseHdrOld header; */ /* header info describing I/Q environment */
  
} ;  /* end pulseData */

/* old data structure for backwards compatibility */
struct pulseDataOld  { 
   unsigned int id;
   UINT4 recordSize;            /* size of this data record in bytes */
   UINT4 version;               /* version of data structure */
   SINT4 packedData;            /* packed data format? TRUE/FALSE */
   UINT2 startGate;             /* beginning range bin of data */
   UINT2 endGate;               /* ending gate of data */
   UINT4 spare[27];             /* spares for future growth */
   struct rvp8PulseHdrOld header;  /* header info describing I/Q environment */
} ;  /* end pulseData */

union pulseDataUnion  {
   struct pulseData pdnew;
   struct pulseDataOld pdold;
};

#define DATAINFO     12   /* number of bytes for data structure msgData */
                          /*   excluding the pulseData member, this is the */
                          /*   information associated with the data message */
                          /* these bytes are retrieved first with a data read */

/* header info of the data message sent between server and client */
struct msgData {
   UINT1             type ;            /* indicates Data message */
                                       /* compiler pads with 3 bytes here */
   unsigned long     numPulses;        /* number of pulses in this group */
   unsigned long     totalBytes;       /* total number of bytes of this group */
   struct pulseData  dataHeader;       /* data header information, */
                                       /*    written to file */
} ;  /* msgData */
