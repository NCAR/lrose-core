#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "sigtypes.h"
#include "dsp.h"
#include "rvp8.h"
#include "user_lib.h"
#include "setup.h"
#include "dsp_lib.h"
#include "antenna_lib.h"
#include "intelipp_lib.h"
#include "rdasubs_lib.h"
#include "sig_rtdisp.h"
#include "rtq_lib.h"

/* define what information the proc command puts in the
 * header before the ray data */

/* we're currently requesting:
 *       DSP_HDR_TAGS   4 words: begin/end az and begin/end el
 *       DSP_HDR_TSTAMP 1 word:  time series time stamp (milliseconds)
 *       DSP_HDR_MMTS   1 word:  bits showing a mismatch between the
 *                               data and processing setup
 *       DSP_HDR_GPARM  64 words */

#define HDRBITS  DSP_HDR_TAGS | DSP_HDR_TSTAMP | DSP_HDR_MMTS | DSP_HDR_GPARM
#define NWORDS_HDR  70
#define NGATES 500

struct gparm Gparm;
struct dspExParm ExParm;

static void exit_handler( void ) ;
static void SetupDsp ( void ) ;
int MomentCount ( struct dsp_data_mask *dataMask ) ;
static Dsp *dsp_c = NULL ;
SINT2 iSubr[ RTQ_MAX_MOMENTS ] ;

/* ------------------------------
 * This program interfaces with the dsp library to drive processing by
 * various major modes and displays the resulting moments on a SIGMET
 * real time display data stream.
 */

int main( int argc_a, char *argv_a[] ) {

#define MAXREADWORDS 500000

  UINT1 i ;
  MESSAGE iStatus ;
  struct dsp_data_mask DataMask = {0};
  SINT4 nRead, iWordsToRead ;
  UINT2 iWordBuf[ MAXREADWORDS ] ;
  UINT2 iMomCount ;
     
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

  /*
   * set up data field mask
   */

  DspMaskSet ( &DataMask, DB_DBZ2 );
  DspMaskSet ( &DataMask, DB_VEL2 );
  DspMaskSet ( &DataMask, DB_WIDTH2 );
  DspMaskSet ( &DataMask, DB_FLAGS2 );

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

  /* Open the DSP */

  printf( "Opening DSP\n" ) ;
  dsp_c = DspOpenForIo( &iStatus ) ;
  if( NULL == dsp_c ) {
    sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
  } else {
    sig_signal( iStatus ) ;
  }
  
  fprintf(stderr, "--->> DspResetFifo\n");
  iStatus = DspResetFifo( dsp_c );

  /* Check for correct byte order */

  fprintf(stderr, "--->> DspCheckByteOrder\n");
  if (iStatus==SS_NORMAL) iStatus = DspCheckByteOrder( dsp_c ) ;
  if( iStatus != SS_NORMAL ) {
    sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
  }

  /* Set up angle synchronization mode
   */
  
  fprintf(stderr, "--->> dspw_syn_mode\n");
  
  iStatus = dspw_syn_mode ( dsp_c, ANGSYN_DYNAMIC, 0 ) ;
  if( iStatus != SS_NORMAL ) {
    sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
  }

  /* Set the task name */

  fprintf(stderr, "--->> DspWriteTaskId\n");
  
  iStatus = DspWriteTaskId( dsp_c, "pd", 0, 0, SCAN_PPI ) ;
  if( iStatus != SS_NORMAL ) {
    sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
  }
  

  /*
   * set up dsp
   */

  SetupDsp ();

  iMomCount = MomentCount ( &DataMask ) ; 
  printf ("iMomCount = %d, BinCount = %d, NWORDS_HDR = %d\n",
	  iMomCount, NGATES, NWORDS_HDR);
  iWordsToRead = iMomCount * NGATES + NWORDS_HDR ;
      
  /* Main loop */

  for (;;) {

    iStatus = dspw_proc( dsp_c, 
			 FALSE,  /* we do NOT want the archive bit set,
				  * use 16 bit data */
			 &DataMask,
			 TSOUT_HISNR,
			 DSP_PROC_SYNCHRONOUS ) ;
    
    if( iStatus != SS_NORMAL ) {
      sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
    }
    
    {
      int somethingRead = 0;
      
      do {

	UINT2 istat;
        
	sig_microSleep( 10000 );
	
	iStatus = dspr_status( dsp_c, &istat ) ;
	if( iStatus != SS_NORMAL ) {
	  sig_signal( iStatus ) ; 
	  exit(EXIT_FAILURE) ;
	}
	
	iStatus = DspReadAvail( dsp_c, iWordBuf, iWordsToRead, 2048,
				&nRead ) ;

	if (nRead > 0) {
	  fprintf(stderr, "-->> read nwords: %d\n", nRead);
	}
	
	somethingRead += nRead;
	if (iStatus != SS_NORMAL) {
	  printf( "nRead = %d\n", nRead );
	  sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
	}

      }
      while ( ( somethingRead == 0 ) || ( nRead != 0 ) );
    }

  } /* end of looping through process */
  
  fprintf(stderr, "Closing: --->> DspClose\n");

  iStatus = DspClose( dsp_c );
  if( iStatus != SS_NORMAL ) {
    sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
  }

  exit(EXIT_SUCCESS) ;

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

/* -----------------------------------
 * set up params on dsp
 */

static void SetupDsp ( void )
{

  MESSAGE iStatus ;
  UINT2 filtmask[RVP8MAXBINS] = {0} ;
  UINT2 imask[512] ;
  int i;
   
  DspResetFifo( dsp_c );

  fprintf(stderr, "SetupDsp -->>> dspw_range_mask\n");
  
  range_mask_gen( dsp_c,
		  imask,
		  0.0,
		  NGATES,
		  0.250,
		  0 ) ;
  
  iStatus = dspw_range_mask( dsp_c,
			     imask,
			     0 ) ;
  
  if( iStatus != SS_NORMAL ) {
    sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
  }

  fprintf(stderr, "SetupDsp -->>> dspw_options\n");
  
  iStatus = dspw_options( dsp_c,
                          PMODE_FFT,
                          64,
                          OPF_RNV,
                          0,
                          0,
                          PRF_FIXED,
                          WIN_BLACKMAN,
                          4 );
  
  if( iStatus != SS_NORMAL ) {
    sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
  }

  fprintf(stderr, "SetupDsp -->>> dspw_header_cfg\n");
  
  iStatus = dspw_header_cfg( dsp_c, HDRBITS );
  if( iStatus != SS_NORMAL ) {
    sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
  }
  
  /* Set the PRF */

  fprintf(stderr, "SetupDsp -->>> dsp_set_prf\n");

  iStatus = dsp_set_prf( dsp_c,
                         1000,
                         0,
                         TRUE ) ;

  if( iStatus != SS_NORMAL ) {
    sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
  }
  
  /* Set the Phase Modulation */

  fprintf(stderr, "SetupDsp -->>> dspw_phaseSeq\n");

  iStatus = dspw_phaseSeq( dsp_c,
			   PHSEQ_FIXED,
			   0,
			   0 ) ;

  if( iStatus != SS_NORMAL ) {
    sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
  }

  /* set the polarization */
  
  fprintf(stderr, "SetupDsp -->>> dsp_set_polarization\n");
  
  iStatus = dsp_set_polarization(dsp_c,
				 POL_HORIZ_FIX,
				 TRUE);
  if( iStatus != SS_NORMAL ) {
    sig_signal( iStatus ) ; exit(EXIT_FAILURE) ;
  }
  
}


int MomentCount (  struct dsp_data_mask *dataMask )
{

  int i;
  int iMomentCnt=0;
  
  for (i = 1; i < NUM_DEFINED_DATA; i++) {
    if( lDspMaskTest( dataMask, i ) ) {
      iMomentCnt++;
    }
  }
  printf ("iMomentCnt=%d\n",iMomentCnt);
  return ( iMomentCnt );
}
