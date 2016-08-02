/* *****************************************
 * *                                       *
 * * Major Mode PROC driver program        *
 * *                                       *
 * *****************************************
 * file: utils/dsp/pd.c
 *      
 *   COPYRIGHT (c) 1994, 1995, 1996, 1997, 1999, 2000, 2002, 2003  BY
 *          SIGMET INCORPORATED, WESTFORD MASSACHUSETTS, U.S.A.  
 * 
 * THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED
 * ONLY  IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
 * INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE  OR  ANY OTHER
 * COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
 * OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY
 * TRANSFERED. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "sigtypes.h"
#include "dsp.h"
#include "user_lib.h"
#include "setup.h"
#include "dsp_lib.h"
#include "antenna_lib.h"
#include "intelipp_lib.h"
#include "rdasubs_lib.h"
#include "sig_rtdisp.h"
#include "rtq_lib.h"
#include "rvpts.h"
#include "rvp8.h"

/* define what information the proc command puts in the
 * header before the ray data */

/* we're currently requesting:
 *       DSP_HDR_TAGS   4 words: begin/end az and begin/end el
 *       DSP_HDR_TSTAMP 1 word:  time series time stamp (milliseconds)
 *       DSP_HDR_MMTS   1 word:  bits showing a mismatch between the
 *                               data and processing setup */

#define HDRBITS  DSP_HDR_TAGS | DSP_HDR_TSTAMP | DSP_HDR_MMTS | DSP_HDR_GPARM

#define HDRSIZE  70
#define BEGAZIN  0
#define BEGELIN  1
#define ENDAZIN  2
#define ENDELIN  3
#define TSINDEX  4
/* #define MMINDEX  5 */
#define GPINDEX  5 
#define MMINDEX  69


struct CmdLineParms {
  UINT1 iMajorMode;
  UINT1 lAngleSync;
  UINT4 iSampSize;
  UINT4 iDfltSampSize;
  UINT4 iDopSampSize;
  UINT4 iSurvSampSize;
  UINT1 iPhaseCode;
  UINT1 iPolarization;
  UINT1 iWindowType;
  FLT4 fPrf;
  SINT4 PHBinCount;
  UINT2 iProcFlags;
  SINT2 iRangeSmooth;
  char  *ptrCP[20];
  UINT1 icp;
  SINT1 iClutFilt;
  struct dsp_data_mask *pDataMask;
  UINT1 lProdA2;
  UINT1 lShowAng;
  UINT1 lShowTS;
  UINT1 lSimTarget;
  UINT1 lDrive;
  UINT1 lChange;
} ;

struct gparm Gparm;
struct dspExParm ExParm;

static void usage( void ) ;
static void exit_handler( void ) ;
static void ProduceA2 ( UINT2 *pWordBuf,
			struct CmdLineParms *pCLP, UINT2 iMomCount ) ;
static void ProcCmdLine ( int argc_a, char *argv_a[],
			  struct CmdLineParms *pCLP) ;
static void SetupProcParams ( struct CmdLineParms *pCLP ) ;
int MomentCount ( struct CmdLineParms *pCLP ) ;
static Dsp *dsp_c = NULL ;
SINT2 iSubr[ RTQ_MAX_MOMENTS ] ;
const struct dsp_manual_setup *pDspSetup ;

/* ------------------------------
 * This program interfaces with the dsp library to drive processing by
 * various major modes and displays the resulting moments on a SIGMET
 * real time display data stream.
 */

int main( int argc_a, char *argv_a[] ) {

#define MAXREADWORDS 500000

  UINT1 lFlagEQ, lAngleCmp, i ;
  UINT4 iStartTime, iEndTime, iTime;
  MESSAGE iStatus ;
  struct dsp_data_mask DataMask = {0};
  SINT4 iActualCount, iWordsToRead ;
  UINT1 ParamChg = TRUE;
  UINT2 iWordBuf[ MAXREADWORDS ] ;
  UINT2 iMomCount, iProcMode;
  struct RtqQueueElement Elem = {0} ;
  UINT4 SeqNum = 0;
  /* UINT4 PrfTicks ;  */
  struct CmdLineParms CLP ;
  static char ConfParms[20][20] = {{0}} ;
  int RadialCount = 0;
     
  /* Setup the signal handler and exit handler, and check that we're a
   * valid user.
   */

  sig_establish( NULL, "pd" );

  if( atexit( exit_handler ) == -1 ) {
    sig_signal( errno_to_message_va( errno, 1, "pd->atexit()" ) ) ;
    exit(1) ;
  }
  if( !liris_operator( sig_username() ) ) {
    printf("Sorry, you must be an IRIS operator to run pd.\n") ;
    exit(EXIT_FAILURE) ;
  }

  /* Process arguments from the command line
   */

  for (i=0; i<20; i++) {
    CLP.ptrCP[i] = &ConfParms[i][0];
  }

  CLP.iMajorMode  = PMODE_USER4 ;
  CLP.lAngleSync = TRUE ;
  CLP.lShowAng = FALSE ;
  CLP.iDfltSampSize = 64 ;
  CLP.iDopSampSize  = 70 ;
  CLP.iSurvSampSize = 17 ;
  CLP.fPrf       = 1000.0 ;
  CLP.iPhaseCode = PHSEQ_FIXED ;
  CLP.iPolarization = POL_HORIZ_FIX ;
  /* CLP.iProcFlags = OPF_RNV | OPF_16B ;*/ /* Range normalize and 16 bit output from Proc */
  CLP.iProcFlags = OPF_RNV ; /* Range normalize and 16 bit output from Proc */
  CLP.iRangeSmooth = 4 ;
  CLP.iWindowType = WIN_BLACKMAN ;
  CLP.icp = 0 ;
  CLP.iClutFilt = 0 ;
  CLP.pDataMask = &DataMask;
  CLP.lProdA2 = FALSE;
  CLP.lShowTS = FALSE;
  CLP.PHBinCount = 1000 ;
  CLP.lSimTarget = FALSE ;
  CLP.lDrive = FALSE ;
  CLP.lChange = FALSE ;

  ProcCmdLine( argc_a, &argv_a[0], &CLP ) ;

  /* if the command line contained a file with command
   * line options, call ProcCmdLine */

  /* again to process those parameters */

  if ( CLP.icp > 0 ) {
    ProcCmdLine( CLP.icp, &CLP.ptrCP[0], &CLP ) ;
  }

  lAngleCmp  = FALSE ;
  lFlagEQ    = FALSE ;
  CLP.iSampSize  = CLP.iDfltSampSize ;
  
  /*
   * ant power up
   */

  printf( "before AntPowerUpMain\n" );
  fflush(NULL);
  iStatus = AntPowerUpMain( ANT_IAM_CLIENT, 0 ) ;
  if( iStatus != SS_NORMAL ) {
    sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
  }
  printf( "after AntPowerUpMain\n" );
  fflush(NULL);

  /* Attempt to open the DSP
   */

  printf( "Opening DSP\n" ) ;
  dsp_c = DspOpenForIo( &iStatus ) ;
  if( NULL == dsp_c ) {
    sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
  } else {
    sig_signal( iStatus ) ;
  }
  
  /* print out type of interface */

  {
    UINT4 interface_type = idsp_interface(dsp_c);
    switch (interface_type) {
    case DSPIFACE_SCSI:
      fprintf(stderr, "  interface type: SCSI\n");
      break;
    case DSPIFACE_NATIVE:
      fprintf(stderr, "  interface type: NATIVE\n");
      break;
    case DSPIFACE_SOCKET:
      fprintf(stderr, "  interface type: SOCKET\n");
      break;
    default:
      fprintf(stderr, "  interface type: UNKNOWN\n");
      break;
      
    }
  }

  /* print out processor type */

  {
    UINT4 idspType = idsp_type(dsp_c);
    fprintf(stderr, "  DSP type: %d\n", (int) idspType);
  }

  /* Get the pointer to DSP Setup info */
  pDspSetup = DspGetSetupPointer( dsp_c ) ;
  
  /* Check for correct byte order
   */

  fprintf(stderr, "--->> DspResetFifo\n");

  iStatus = DspResetFifo( dsp_c );
  fprintf(stderr, "--->> DspCheckByteOrder\n");
  if (iStatus==SS_NORMAL) iStatus = DspCheckByteOrder( dsp_c ) ;
  if( iStatus != SS_NORMAL ) {
    sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
  }

  /* Set up angle synchronization mode
   */
  
  fprintf(stderr, "--->> dspw_syn_mode\n");

  iStatus = dspw_syn_mode
    ( dsp_c, CLP.lAngleSync ? ANGSYN_DYNAMIC : ANGSYN_NONE, 0 ) ;
  if( iStatus != SS_NORMAL ) {
    sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
  }

  /* Set the task name
   */

  fprintf(stderr, "--->> DspWriteTaskId\n");

  iStatus = DspWriteTaskId( dsp_c, "pd", 0, 0, TASK_SCAN_PPIFULL ) ;
  if( iStatus != SS_NORMAL ) {
    sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
  }


  fprintf(stderr, "--->> dspw_playbackTSOpts\n");

  iStatus = dspw_playbackTSOpts ( dsp_c, MMTS_PHASEMOD ) ;
  if( iStatus != SS_NORMAL ) {
    sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
  }

  /* Setup the Target Simulator if requested */

  if (CLP.lSimTarget == TRUE) {

    struct dspPhysTargSim PTSim;
    
    PTSim.iFlags = DPTS_SIMCOPOL;
    PTSim.iStartKM10 = 1450;
    PTSim.iLengthKM10 = 300;
    PTSim.iPowerDB10 = -150;
    PTSim.iPowerSpanDB10 = -5;
    PTSim.iDopplerHZ = -100;
    PTSim.iDopplerSpanHZ = 200;
    

    fprintf(stderr, "--->> dspw_targSimEnable\n");
    fprintf(stderr, "--->> dspw_targSimDefine\n");

    dspw_targSimEnable( dsp_c, TRUE );
    dspw_targSimDefine( dsp_c, 0, &PTSim );
    
  }

  /* initialize proc arguments */
  /* PrfTicks = 111702 ;  */
  /* PHBinCount = 1865 ; */
  /* iMajorMode = PMODE_USER4 ;  */
  /* CLP.iPhaseCode = PHSEQ_FIXED ;  */

  ParamChg = TRUE ;

  /* Main driving loop for PD - this checks for setup changes, reads data
   * and will eventually write data
   */

  for (;;) {

    UINT1 debugPrint = FALSE;
    
    if ( CLP.lDrive == TRUE ) {
      printf("RadialCount = %d\n", RadialCount);
      if ( ++RadialCount > 370 ) {
	printf("RadialCount = %d\n", RadialCount);
	CLP.lChange = TRUE;
	CLP.lDrive = FALSE;
	ParamChg = TRUE;
      }
    }

    if ( ParamChg == TRUE ) {

      SetupProcParams ( &CLP );
      
      if (CLP.lShowTS == TRUE) {
	iWordsToRead = 2 * CLP.iSampSize * CLP.PHBinCount + HDRSIZE ; 
      } else {
	iMomCount = MomentCount ( &CLP ) ; 
	printf ("iMomCount = %d, BinCount = %d, HDRSIZE = %d\n",
		iMomCount, CLP.PHBinCount, HDRSIZE);
	iWordsToRead = iMomCount * CLP.PHBinCount + HDRSIZE ;
      }
      
      printf ("iWordsToRead=%d\n", iWordsToRead );
      ParamChg = FALSE ; 
      debugPrint = TRUE ;

    }

    iStatus = dspr_gparm( dsp_c, &Gparm, GPARM_LSTATUS_CLEAR ) ;
    if( iStatus != SS_NORMAL ) {
      sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
    }

    if (debugPrint) printf ("Issuing first PROC after Param Change \n");
    debugPrint = FALSE ;

    if (CLP.lShowTS == TRUE) {
      iProcMode = DSP_PROC_TIME_SERIES;
    } else {
      iProcMode = DSP_PROC_SYNCHRONOUS;
    }
     
    iStatus = dspw_proc( dsp_c, 
			 FALSE,  /* we do NOT want the archive bit set,
				  * use 16 bit data */
			 &DataMask,
			 /* TSOUT_SPEC, */
			 TSOUT_HISNR,
			 iProcMode ) ;
    
    if( iStatus != SS_NORMAL ) {
      sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
    }
    
    getbtime( &iTime ) ; iEndTime = iStartTime = iTime ;

    {
      int somethingRead = 0;
      
      do {

	UINT2 istat;
        
	sig_microSleep( 12500 );

	iStatus = dspr_status( dsp_c, &istat ) ;
	if( iStatus != SS_NORMAL ) {
	  sig_signal( iStatus ) ; 
	  exit(EXIT_FAILURE) ;
	}
	
	iStatus = DspReadAvail( dsp_c, iWordBuf, iWordsToRead, 2048,
				&iActualCount ) ;
	somethingRead += iActualCount;
	if (iStatus != SS_NORMAL) {
	  printf( "iActualCount = %d\n", iActualCount );
	  sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
	}

      }
      while ( ( somethingRead == 0 ) || ( iActualCount != 0 ) );
    }

    Elem.RHeader.azimuth_start = (BIN2) iWordBuf[ BEGAZIN ] ;
    Elem.RHeader.azimuth = (BIN2) iWordBuf[ ENDAZIN ] ;
    Elem.RHeader.elevation = (BIN2) iWordBuf[ ENDELIN ] ;
    /* Elem.RHeader.itime += (UINT2) iWordBuf[ TSINDEX ] ; */
    Elem.iSequenceNum = SeqNum++ ;
    Elem.RHeader.iBinCount = CLP.PHBinCount ;

    /* if ( iWordBuf[ MMINDEX ] != 0 ) */
    if (( iWordBuf[ MMINDEX ] & ~MMTS_PHASEMOD ) != 0 )	{
      printf ("Mismatch happens: iWordBuf[ MMINDEX ] = %8.8x\n",
	      iWordBuf[ MMINDEX ] );
      ParamChg = TRUE ;
    }

    if ( (CLP.lProdA2 == TRUE) || (CLP.lShowAng == TRUE) ) {
      ProduceA2 (iWordBuf, &CLP, iMomCount) ;
    }

    /*
     * after inserting a ray that is possibly the
     * first in the volume, reset the flag
     * indicating the start of a volume */

    Elem.VHeader.iFlags = 0 ;
    
  } /* end of looping through process */

  fprintf(stderr, "Closing: --->> DspClose\n");

  iStatus = DspClose( dsp_c );
  if( iStatus != SS_NORMAL ) {
    sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
  }

  exit(EXIT_SUCCESS) ;

}

/* ------------------------------
 * Give correct usage message, and error exit.
 */
static void usage()
{

  fprintf( stderr,
	   "PROC Driver \n"
	   "-------------------------------------\n"
	   "         -angsyn : Run with dynamic angle syncing\n"
	   "       -archive2 : Write Moments to a file (RnD)\n"
	   "       -bins <N> : Setup a range mask with N-bins\n"
	   "         -cf <N> : Define which clutter filter to use\n"
	   "                 :  0-7 as defined in SIGMET setup\n"
	   "   -dfltsamp <N> : Default sample size\n"
	   "    -dopsamp <N> : Doppler sample size\n"
	   "           -help : Print this menu\n"
	   "      -mmt <xxx> : Choose moments to be processed\n"
	   "                 : mmt options:  dbz dbt vel wid sqi flags\n"
	   "                 :               dbz2 dbt2 vel2 wid2 sqi2 flags2\n"
	   "     -mode <xxx> : Define the Major Mode for processing\n"
	   "                 : mode options:  ppp rph fft batch u1 u2 u3 u4\n"
	   "      -phase <N> : Select phase encoding (ignored and set to match data) \n"
	   "                 : 0-fixed, 1-random, 2-custom, 3-sz8/64 \n"
	   "      -polar <N> : Select polarization\n"
	   "                 : 0-H, 1-V, 2-alternating, 3-simultaneous\n"
	   "        -prf <N> : Select pulse repetition frequency (ignored and set to match data) \n"
	   "        -showang : Write Elevation and Azimuth Angles to a file (RnD)\n"
	   "   -survsamp <N> : Surveillance sample size\n"
	   ) ;
}

/* ------------------------------
 * Clean up the signal processor if it is still trying to output more
 * words to us.
 */
static void exit_handler( void )
{
  MESSAGE iStatus;

  /* MESSAGE istatus = SS_NORMAL ; */
  fprintf(stderr, "Exiting: DspResetFifo\n");
  if( dsp_c ) DspResetFifo( dsp_c ); 

  fprintf(stderr, "Closing: --->> DspClose\n");
  iStatus = DspClose( dsp_c );
  if( iStatus != SS_NORMAL ) {
    sig_signal( iStatus );
  }

}

static void ProcCmdLine ( int argc_a, char *argv_a[],
			  struct CmdLineParms *pCLP )
{

  SINT4 ii, jj;
  FILE *param_fd;
  char *newln;
  
  for( ii=1 ; ii < argc_a ; ii++ ) {

    int ilen = strlen( argv_a[ii] ) - 1 ;
    if( (argv_a[ii][0] == '-' || argv_a[ii][0] == '/') && (ilen>0) ) {
      
      if( 0 == strncasecmp( &argv_a[ii][1], "help", ilen ) ) {
        usage() ; exit(EXIT_SUCCESS) ;
      }

      if( 0 == strncasecmp( &argv_a[ii][1], "file", ilen ) ) {
        if( ++ii < argc_a ) {
          if ( ! ( NULL == ( param_fd = fopen( argv_a[ii], "r" ) ) ) ) {
	    printf ("Opened file %s\n", argv_a[ii]);
	    jj = 1;
	    while ( ! ( NULL == (fgets ( pCLP->ptrCP[jj], 20, param_fd)) ) )
	      {
                if (! ( NULL == ( newln = strstr(pCLP->ptrCP[jj],"\n") ) ) )
		  *newln = '\0' ;
                printf ( "param = %d, %s\n", jj, pCLP->ptrCP[jj] );
                jj++;
	      }
            
	    pCLP->icp = jj;
	    fclose ( param_fd );
	    continue ;
	  }
	  printf( "Invalid file argument: %s\n", argv_a[ii] ) ; exit(EXIT_FAILURE) ;
        } else {
          printf( "Missing file argument\n" ) ; exit(EXIT_FAILURE) ;
        }
        continue ;
      }
            
      if( 0 == strncasecmp( &argv_a[ii][1], "mode", ilen ) ) {
        if( ++ii < argc_a ) {
          if ( ! ( NULL == strstr( "ppp fft rph batch u1 u2 u3 u4", argv_a[ii] ) ) ) {
            ilen = strlen( argv_a[ii] ) ;
            if ( 0 == strncasecmp( argv_a[ii], "ppp", ilen ) ) {
              pCLP->iMajorMode = PMODE_PPP ;
              continue ;
            }
            if ( 0 == strncasecmp( argv_a[ii], "fft", ilen ) ) {
              pCLP->iMajorMode = PMODE_FFT ;
              continue ;
            }
            if ( 0 == strncasecmp( argv_a[ii], "rph", ilen ) ) {
              pCLP->iMajorMode = PMODE_RPH ;
              continue ;
            }
            if ( 0 == strncasecmp( argv_a[ii], "batch", ilen ) ) {
              pCLP->iMajorMode = PMODE_BATCH ;
              continue ;
            }
            if ( 0 == strncasecmp( argv_a[ii], "u1", ilen ) ) {
              pCLP->iMajorMode = PMODE_USER1 ;
              continue ;
            }
            if ( 0 == strncasecmp( argv_a[ii], "u2", ilen ) ) {
              pCLP->iMajorMode = PMODE_USER2 ;
              continue ;
            }
            if ( 0 == strncasecmp( argv_a[ii], "u3", ilen ) ) {
              pCLP->iMajorMode = PMODE_USER3 ;
              continue ;
            }
            if ( 0 == strncasecmp( argv_a[ii], "u4", ilen ) ) {
              pCLP->iMajorMode = PMODE_USER4 ;
              continue ;
            }
            printf( "Invalid mode argument: %s\n", argv_a[ii] ) ; exit(EXIT_FAILURE) ;
          } else 
            printf( "Invalid mode argument: %s\n", argv_a[ii] ) ; exit(EXIT_FAILURE) ;
        } else {
          printf( "Missing mode argument\n" ) ; exit(EXIT_FAILURE) ;
        }
        continue ;
      }
      if( 0 == strncasecmp( &argv_a[ii][1], "drive", ilen ) ) {
        pCLP->lDrive = TRUE ; continue ;
      }
      if( 0 == strncasecmp( &argv_a[ii][1], "target", ilen ) ) {
        pCLP->lSimTarget = TRUE ; continue ;
      }
      if( 0 == strncasecmp( &argv_a[ii][1], "angsyn", ilen ) ) {
        pCLP->lAngleSync = TRUE ; continue ;
      }
      if( 0 == strncasecmp( &argv_a[ii][1], "showang", ilen ) ) {
        pCLP->lShowAng = TRUE ; continue ;
      }
      if( 0 == strncasecmp( &argv_a[ii][1], "archive2", ilen ) ) {
        pCLP->lProdA2 = TRUE ; continue ;
      }
      if( 0 == strncasecmp( &argv_a[ii][1], "showts", ilen ) ) {
        pCLP->lShowTS = TRUE ; continue ;
      }
      if( 0 == strncasecmp( &argv_a[ii][1], "dfltsamp" , ilen ) ) {
        if( ++ii < argc_a ) {
          pCLP->iDfltSampSize = atoi( argv_a[ii] ) ;
          if( (pCLP->iDfltSampSize < 1) || (pCLP->iDfltSampSize > RVP8_MAXPULSES) ) {
            printf( "Invalid default samples argument: %d\n", pCLP->iDfltSampSize ) ; exit(EXIT_FAILURE) ;
          }
        } else {
          printf( "Missing default samples argument\n" ) ; exit(EXIT_FAILURE) ;
        }
        continue ;
      }
      if( 0 == strncasecmp( &argv_a[ii][1], "dopsamp" , ilen ) ) {
        if( ++ii < argc_a ) {
          pCLP->iDopSampSize = atoi( argv_a[ii] ) ;
          if( (pCLP->iDopSampSize < 1) || (pCLP->iDopSampSize > RVP8_MAXPULSES) ) {
            printf( "Invalid doppler samples argument: %d\n", pCLP->iDopSampSize ) ; exit(EXIT_FAILURE) ;
          }
        } else {
          printf( "Missing doppler samples argument\n" ) ; exit(EXIT_FAILURE) ;
        }
        continue ;
      }
      if( 0 == strncasecmp( &argv_a[ii][1], "survsamp" , ilen ) ) {
        if( ++ii < argc_a ) {
          pCLP->iSurvSampSize = atoi( argv_a[ii] ) ;
          if( (pCLP->iSurvSampSize < 1) || (pCLP->iSurvSampSize > RVP8_MAXPULSES) ) {
            printf( "Invalid surveillance samples argument: %d\n", pCLP->iSurvSampSize ) ; exit(EXIT_FAILURE) ;
          }
        } else {
          printf( "Missing surveillance samples argument\n" ) ; exit(EXIT_FAILURE) ;
        }
        continue ;
      }
      if( 0 == strncasecmp( &argv_a[ii][1], "phase" , ilen ) ) {
        if( ++ii < argc_a ) {
          pCLP->iPhaseCode = atoi( argv_a[ii] ) ;
          if (pCLP->iPhaseCode > PHSEQ_SZ8_64) {
            printf( "Invalid phase argument: %d\n", pCLP->iPhaseCode ) ; exit(EXIT_FAILURE) ;
          }
        } else {
          printf( "Missing phase argument\n" ) ; exit(EXIT_FAILURE) ;
        }
        continue ;
      }
      if( 0 == strncasecmp( &argv_a[ii][1], "polar" , ilen ) ) {
        if( ++ii < argc_a ) {
          pCLP->iPolarization = atoi( argv_a[ii] ) ;
          if (pCLP->iPolarization > POL_SIMULTANEOUS) {
            printf( "Invalid polarization: %d\n", pCLP->iPolarization ) ; exit(EXIT_FAILURE) ;
          }
        } else {
          printf( "Missing phase argument\n" ) ; exit(EXIT_FAILURE) ;
        }
        continue ;
      }
      if( 0 == strncasecmp( &argv_a[ii][1], "prf" , ilen ) ) {
        if( ++ii < argc_a ) {
          pCLP->fPrf = atof( argv_a[ii] ) ;
          if( (pCLP->fPrf <= 320.0) || (pCLP->fPrf > 1322.0) ) {
            printf( "Invalid prf: %d\n", pCLP->fPrf ) ; exit(EXIT_FAILURE) ;
          }
        } else {
          printf( "Missing prf\n" ) ; exit(EXIT_FAILURE) ;
        }
        continue ;
      }
      if( 0 == strncasecmp( &argv_a[ii][1], "bins" , ilen ) ) {
        if( ++ii < argc_a ) {
          pCLP->PHBinCount = atol( argv_a[ii] ) ;
          if( (pCLP->PHBinCount <= 0) || (pCLP->PHBinCount > 3072) ) {
            printf( "Invalid bin count: %d\n", pCLP->PHBinCount ) ; exit(EXIT_FAILURE) ;
          }
        } else {
          printf( "Missing bin count\n" ) ; exit(EXIT_FAILURE) ;
        }
	printf( "Setting bin count: %d\n", pCLP->PHBinCount ) ;
        continue ;
      }
      if( 0 == strncasecmp( &argv_a[ii][1], "cf" , ilen ) ) {
        if( ++ii < argc_a ) {
          pCLP->iClutFilt = atol( argv_a[ii] ) ;
          if( (pCLP->iClutFilt < 0) || (pCLP->iClutFilt >= 8) ) {
            printf( "Invalid clutter filter slot: %d\n", pCLP->iClutFilt ) ; exit(EXIT_FAILURE) ;
          }
        } else {
          printf( "Missing clutter filter slot\n" ) ; exit(EXIT_FAILURE) ;
        }
        continue ;
      }
      if( 0 == strncasecmp( &argv_a[ii][1], "mmt", ilen ) ) {
        if( ++ii < argc_a ) {
          if ( ! ( NULL == strstr( "dbz dbt vel wid sqi flags dbz2 dbt2 vel2 wid2 sqi2 flags2", argv_a[ii] ) ) ) {
            ilen = strlen( argv_a[ii] ) ;
            if ( 0 == strncasecmp( argv_a[ii], "dbz", ilen ) ) {
              DspMaskSet ( pCLP->pDataMask, DB_DBZ );
              continue ;
            }
            if ( 0 == strncasecmp( argv_a[ii], "dbt", ilen ) ) {
              DspMaskSet ( pCLP->pDataMask, DB_DBT );
              continue ;
            }
            if ( 0 == strncasecmp( argv_a[ii], "vel", ilen ) ) {
              DspMaskSet ( pCLP->pDataMask, DB_VEL );
              continue ;
            }
            if ( 0 == strncasecmp( argv_a[ii], "wid", ilen ) ) {
              DspMaskSet ( pCLP->pDataMask, DB_WIDTH );
              continue ;
            }
            if ( 0 == strncasecmp( argv_a[ii], "sqi", ilen ) ) {
              DspMaskSet ( pCLP->pDataMask, DB_SQI2 );
              continue ;
            }
            if ( 0 == strncasecmp( argv_a[ii], "flags", ilen ) ) {
              DspMaskSet ( pCLP->pDataMask, DB_FLAGS2 );
              continue ;
            }
            if ( 0 == strncasecmp( argv_a[ii], "dbz2", ilen ) ) {
              DspMaskSet ( pCLP->pDataMask, DB_DBZ2 );
              continue ;
            }
            if ( 0 == strncasecmp( argv_a[ii], "dbt2", ilen ) ) {
              DspMaskSet ( pCLP->pDataMask, DB_DBT2 );
              continue ;
            }
            if ( 0 == strncasecmp( argv_a[ii], "vel2", ilen ) ) {
              DspMaskSet ( pCLP->pDataMask, DB_VEL2 );
              continue ;
            }
            if ( 0 == strncasecmp( argv_a[ii], "wid2", ilen ) ) {
              DspMaskSet ( pCLP->pDataMask, DB_WIDTH2 );
              continue ;
            }
            if ( 0 == strncasecmp( argv_a[ii], "sqi2", ilen ) ) {
              DspMaskSet ( pCLP->pDataMask, DB_SQI2 );
              continue ;
            }
            if ( 0 == strncasecmp( argv_a[ii], "flags2", ilen ) ) {
              DspMaskSet ( pCLP->pDataMask, DB_FLAGS2 );
              continue ;
            }
            printf( "Invalid moment argument: %s\n", argv_a[ii] ) ; exit(EXIT_FAILURE) ;
          } else 
            printf( "Invalid moment argument: %s\n", argv_a[ii] ) ; exit(EXIT_FAILURE) ;
        } else {
          printf( "Missing moment argument\n" ) ; exit(EXIT_FAILURE) ;
        }
        continue ;
      }

      printf( "Unrecognized arg: '%s'\n", &argv_a[ii][0] ) ;
      usage() ; exit(EXIT_FAILURE) ;

    } else {
      printf( "Unrecognized arg: '%s'\n", &argv_a[ii][0] ) ;
      usage() ; exit(EXIT_FAILURE) ;
    }
  }
}

static void SetupProcParams ( struct CmdLineParms *pCLP )
{

  /* struct gparm Gparm;
     struct dspExParm ExParm; */

  MESSAGE iStatus ;
  UINT2 filtmask[RVP8_MAXBINS] = {0} ;
  int i;
   
  fprintf(stderr, "SetupProcParams -->>> DspResetFifo\n");

  DspResetFifo( dsp_c );

  fprintf(stderr, "SetupProcParams -->>> dspr_gparm\n");

  iStatus = dspr_gparm( dsp_c, &Gparm, GPARM_LSTATUS_CLEAR ) ;
  if( iStatus != SS_NORMAL ) {
    sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
  }

  /* Initialize bin count.  Important! */
/*   pCLP->PHBinCount  = Gparm.ibin_out_num ; */
/*   pCLP->PHBinCount  = 1000 ; */
/*   fprintf(stderr, "11111111, setting bin count to ibin_out_num: %d\n", */
/* 	  pCLP->PHBinCount); */
  
  fprintf(stderr, "SetupProcParams -->>> dspr_exParm\n");

  iStatus = dspr_exParm ( dsp_c, &ExParm );
  if( iStatus != SS_NORMAL ) {
    sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
  }

  if (FALSE) {

    printf ("iMismatchTS = %d\n", ExParm.iMismatchTS);
    printf ("  iRayMajorMode = %d\n", ExParm.iRayMajorMode);
    printf ("  iRayBinCount = %d\n", ExParm.iRayBinCount);
    printf ("  iRayUmode = %d\n", ExParm.iRayUmode);
    printf ("  iRayPWidth = %d\n", ExParm.iRayPWidth);
    printf ("  iRayPhaseMod = %d\n", ExParm.iRayPhaseMod);
    printf ("  iRayPolar = %d\n", ExParm.iRayPolar);
    printf ("  fRayNomPRF = %10.6f\n", ExParm.fRayNomPRF);
    
    printf ("GPARM = \n");
    printf (" ibin_out_num = %d\n", Gparm.ibin_out_num );
    printf (" iaqbins = %d\n", Gparm.iaqbins );
    printf (" iprbins = %d\n", Gparm.iprbins );
  }
  printf ("iMismatchTS = %d, fRayNomPRF = %10.6f\n",
	  ExParm.iMismatchTS, ExParm.fRayNomPRF);
  printf ("Check PW:  Gparm.ipw_now = %8.8x\n", Gparm.ipw_now);
  printf ("Check PRT: Gparm.iprt_start = %d, Gparm.iprt_end = %d\n",
	  Gparm.iprt_start, Gparm.iprt_end);
  printf ("           Gparm.iprt_mes = %d\n", Gparm.iprt_mes);
  printf ("                      prf = %10.1f\n",
	  (1.0/ (Gparm.iprt_mes * 0.833333))*1E6);

  /* use the newer values from the mismatch information */
  if ( ExParm.iMismatchTS != 0 ) {
    pCLP->fPrf = ExParm.fRayNomPRF ;
    pCLP->iMajorMode = ExParm.iRayMajorMode ;
    pCLP->PHBinCount = ExParm.iRayBinCount ;
    fprintf(stderr, "222222, setting bin count to ExParm.iRayBinCount: %d\n",
	    pCLP->PHBinCount);
    pCLP->iPhaseCode = ExParm.iRayPhaseMod ;
  }
  
  /* Set up Surveillance scan for first time through */   
  if (( pCLP->lDrive == TRUE) && ( pCLP->lChange == FALSE )) {
    pCLP->fPrf = 322 ;
    pCLP->PHBinCount = 1865 ;
    pCLP->iPhaseCode = PHSEQ_FIXED ;
  }
  
  if ( pCLP->lChange == TRUE ) {
    pCLP->fPrf = 857 ;
    pCLP->PHBinCount = 1401 ;
    pCLP->iPhaseCode = PHSEQ_SZ8_64 ;
    pCLP->lChange = FALSE ;
  }
  
  if ( pCLP->iMajorMode == PMODE_USER4 ) {
    /* 35.9682e6 sample clock freq/ 90000 ticks to determine
     * if in Long PRT scan */
    if ( pCLP->fPrf <= 400.0 ) {
      pCLP->iSampSize = pCLP->iSurvSampSize;
    } else {/* short PRT scan */
      pCLP->iSampSize = pCLP->iDopSampSize;
    }
  }

  /* If a specific number of bins has been requested, then reload the
   * range mask with a uniform mask starting from range zero.
   */

  {

    UINT2 imask[512] ;

    /* LOOK at this and see if we want to set the bin spacing from the data */

    fprintf(stderr, "SetupProcParams -->>> range_mask_gen\n");

    range_mask_gen( dsp_c,
		    imask,
		    0.0,
		    pCLP->PHBinCount,
		    0.250,
		    0 ) ;

    fprintf(stderr, "SetupProcParams -->>> dspw_range_mask\n");

    iStatus = dspw_range_mask( dsp_c,
			       imask,
			       0 ) ;
    
    if( iStatus != SS_NORMAL ) {
      sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
    }
  }

  fprintf(stderr, "SetupProcParams -->>> dspw_options\n");

  iStatus = dspw_options( dsp_c,
                          pCLP->iMajorMode,
                          pCLP->iSampSize,
                          pCLP->iProcFlags,
                          0,
                          0,
                          PRF_FIXED,
                          pCLP->iWindowType,
                          pCLP->iRangeSmooth );

  if( iStatus != SS_NORMAL ) {
    sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
  }
  printf ("proc options: iMajorMode = %d, iSampSize = %d\n",
	  pCLP->iMajorMode, pCLP->iSampSize ) ;
  
  /* Set the header configuration
   */

  fprintf(stderr, "SetupProcParams -->>> dspw_header_cfg\n");

  iStatus = dspw_header_cfg( dsp_c, HDRBITS );
  if( iStatus != SS_NORMAL ) {
    sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
  }

  /* Set the PRF */
  printf ("Before set PRF, pCLP->fPrf = %10.6f\n", pCLP->fPrf);

  fprintf(stderr, "SetupProcParams -->>> dsp_set_prf\n");

  iStatus = dsp_set_prf( dsp_c,
                         pCLP->fPrf,
                         0,
                         TRUE ) ;
  if( iStatus != SS_NORMAL ) {
    sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
  }
  
  /* Set the Phase Modulation */

  fprintf(stderr, "SetupProcParams -->>> dspw_phaseSeq\n");

  iStatus = dspw_phaseSeq( dsp_c,
			   pCLP->iPhaseCode,
			   0,
			   0 ) ;

  if( iStatus != SS_NORMAL ) {
    sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
  }

  /* set the polarization */

  /* if (pCLP->iPolarization != POL_HORIZ_FIX) { */

    fprintf(stderr, "SetupProcParams -->>> dsp_set_polarization\n");

    iStatus = dsp_set_polarization(dsp_c,
                                   pCLP->iPolarization,
                                   TRUE);
    if( iStatus != SS_NORMAL ) {
      sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
    }

    /* } */
  
  /*
   * Turn on Clutter filtering in all range bins if clutter
   * filtering is turned on in the 
   * command line
   */

  if (pCLP->iClutFilt >= 0) {
    for (i = 0; i < RVP8_MAXBINS; i++) {
      filtmask[i] = pCLP->iClutFilt ;
    }
    /* filtmask[i] = 5; */
    printf ("filtmask[100] = %d\n", filtmask[100] );
    
    fprintf(stderr, "SetupProcParams -->>> dspw_filt\n");

    iStatus = dspw_filt( dsp_c, &filtmask[0] ) ;

    if( iStatus != SS_NORMAL ) {
      sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
    } 
  }

  if ( lDspMaskEmpty (pCLP->pDataMask) ) {
    DspMaskSet ( pCLP->pDataMask, DB_DBZ2 );
    DspMaskSet ( pCLP->pDataMask, DB_VEL2 );
    DspMaskSet ( pCLP->pDataMask, DB_WIDTH2 );
    DspMaskSet ( pCLP->pDataMask, DB_FLAGS2 );
  }
  
}


int MomentCount ( struct CmdLineParms *pCLP )
{

  int i;
  int iMomentCnt=0;
  
  for (i = 1; i < NUM_DEFINED_DATA; i++) {
    if( lDspMaskTest( pCLP->pDataMask, i ) ) {
      iMomentCnt++;
    }
  }
  printf ("iMomentCnt=%d\n",iMomentCnt);
  return ( iMomentCnt );
}

static void ProduceA2 ( UINT2 *pWordBuf,
			struct CmdLineParms *pCLP, UINT2 iMomCount ) 
{

  FILE *of ;
  int k;
  
  of = fopen ("RnD", "a"); 
  
  if (pCLP->lShowAng == TRUE ) {
    fprintf ( of, "WordBuf:  Az = %4.4x, %4.4x\n", pWordBuf[BEGAZIN], pWordBuf[ENDAZIN] );
    fprintf ( of, "WordBuf:  El = %4.4x, %4.4x\n", pWordBuf[BEGELIN], pWordBuf[ENDELIN] );
    fprintf ( of, "WBGP:  Az El = %4.4x, %4.4x\n", pWordBuf[GPINDEX+3], pWordBuf[GPINDEX+4] );
    fprintf ( of, "Gparm: Az El = %4.4x, %4.4x\n", Gparm.itaga, Gparm.itagb );
  }
  if (pCLP->lProdA2 == TRUE ) {
    fprintf ( of, "%d ", iMomCount );
    fprintf ( of, "%d ", pCLP->PHBinCount );
    fprintf ( of, "HEADER\n" );
    for ( k=0; k < HDRSIZE; k++) {
      fprintf ( of, "%c", ((UINT1 *)&pWordBuf)[k] );
    }
    fprintf ( of, "EXPARM\n" );
    for ( k=0; k < (int) sizeof(struct dspExParm); k++) {
      fprintf ( of, "%c", ((UINT1 *)&ExParm)[k] );
    }
    fprintf ( of, "ELEM\n" );
    fprintf ( of, "ENDELEM\n" );
  }
  fprintf ( of, "\n" );
  fclose ( of ) ;
}
