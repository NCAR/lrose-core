/* **********************************************************************
 * *	                                                                *
 * *                  Power-Up For Antenna Operations                   *
 * *                                                                    *
 * **********************************************************************
 * File: libs/antenna/iant_pwrp.c
 *
 *       COPYRIGHT (c) 1989, 1991, 1992, 1993, 1996, 1997, 1999,
 *             2000, 2001, 2002, 2003, 2004, 2005, 2008 BY
 *             VAISALA INC., WESTFORD MASSACHUSETTS, U.S.A.
 *
 * THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED
 * ONLY  IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
 * INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE  OR  ANY OTHER
 * COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
 * OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY
 * TRANSFERED.
 */
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "sigtypes.h"
#include "user_lib.h"
#include "event_flags.h"
#include "setup.h"
#include "dsp.h"
#include "antenna_lib.h"
#include "antenna_int.h"
#include "config_lib.h"

#include "ant_global.h"

static MESSAGE ReadRemoteSetup(void);

static MESSAGE ProcessClientType
( SINT4 iClientType_a,
  UINT1 lCreated_a );

static MESSAGE fork_pair
( const char *pdirectory_a,
  volatile struct io_information *pio_a,
  UINT4 ichannel );
static MESSAGE iant_init_csmasks( void ) ;
static MESSAGE iant_init_first( void ) ;

static MESSAGE ant_global_init( UINT2* lcreated_a );

static void antPedxLock  ( void ) ;
static void antPedxUnLock( void ) ;

static void antShipLock  ( void ) ;
static void antShipUnLock( void ) ;

/* This process has powered up 
 */
static UINT1 lPoweredUp_c = FALSE ;
static SINT4 antenna_size_c;
static SINT4 antenna_id_c;
static UINT1 lIAmSimulator_c ; /* (affects Direction of communication) */

/* ------------------------------
 * Perform initializations necessary for all subsequent calls to
 * antenna library routines
 */
MESSAGE iant_pwrp( void )	/* Old interface */
{
  return AntPowerUpMain( ANT_IAM_CLIENT, 0 ) ;
}

MESSAGE AntPowerUpMain( SINT4 iClientType_a, SINT4 iOptions_a )
{
  MESSAGE istatus ; UINT2 lCreated ; char *psdirectory ;

  /* Allow multiple powerup calls in one process, as long as the type
   * does not change.  Otherwise clear the client information and try
   * to powerup from scratch.
   */
  if( lPoweredUp_c ) {
    if( iClientType_a != antClient_c.iClientType ) return ANT_CLIENT_CHANGED ;
    return SS_NORMAL;
  }
  memset( &antClient_c, 0, sizeof(antClient_c) ) ;

  /* Create/Map the global section
   */     
  istatus = ant_global_init( &lCreated );
  if( istatus != SS_NORMAL ) return istatus;

  istatus = ProcessClientType( iClientType_a, lCreated );
  if (istatus != SS_NORMAL)
    {
      sig_shmem_unmap( antGlobal_c, antenna_size_c, antenna_id_c );
      return istatus;
    }

  antClient_c.iClientType = iClientType_a;
  lPoweredUp_c = TRUE;

  /* Create/Map the semaphores needed by the library, and set client
   * pointers into shared objects.
   */
  istatus = create_sem_set
    ( LIBS_ANTENNA_CLUSTER_NAME, LIBS_ANTENNA_SEM_NUM, LIBS_ANTENNA_EF_NUM,
      LIBS_ANTENNA_CLUSTER ) ;

  if( istatus != SS_NORMAL ) {
    MESSAGE istatus2 = ant_close( );
    if( istatus2 != SS_NORMAL ) sig_signal( istatus2 );
    return istatus;
  }

  rtvecAttach( &antClient_c.pedx, &antGlobal_c->vdat.rtpedx_history.info,
	       antPedxLock, antPedxUnLock ) ;

  rtvecAttach( &antClient_c.ship, &antGlobal_c->vdat.rtship_history.info,
	       antShipLock, antShipUnLock ) ;

  /* If the global section was already there, then verify that all
   * subprocesses that should be present indeed seem to be running,
   * and that the file version of the setup structure matches the
   * running copy.
   */
  if ( !lCreated )
    {
      if( ( iClientType_a == ANT_IAM_CLIENT ) ||
	  ( iClientType_a == ANT_IAM_EXPORT ) ||
	  ( iClientType_a == ANT_IAM_RVP    ) ||
	  ( iClientType_a == ANT_IAM_RCP    ) ||
	  ( iClientType_a == ANT_IAM_CHATTER) )
	{
	  struct ant_manual_setup asetup;
	  if (antGlobal_c->info.io[0].lRcvExpected)
	    if ( !antGlobal_c->info.io[0].iRcvPid ) return ANT_ERR_RCV;
	  if ( antGlobal_c->info.io[0].lXmtExpected)
	    if ( !antGlobal_c->info.io[0].iXmtPid ) return ANT_ERR_XMT;

	  if (antGlobal_c->info.io[1].lRcvExpected)
	    if ( !antGlobal_c->info.io[1].iRcvPid ) return ANT_ERR_RCV;
	  if ( antGlobal_c->info.io[1].lXmtExpected)
	    if ( !antGlobal_c->info.io[1].iXmtPid ) return ANT_ERR_XMT;

	  /* Check for setup changes, and report.
	   */
	  istatus = ant_load_setup(&asetup ) ;
	  if (istatus != SS_NORMAL) return istatus ;
	  
	  if (0 != memcmp( (void *)&antGlobal_c->LocalSetup, &asetup, sizeof( asetup ) )) {
	    /***************************************************
	     * MOD TO SIGMET CODE
	     *
	     * Instead of failing, copy the current antenna setup
	     * into the local setup
	     ***************************************************/
	    fprintf(stderr, "NOTE - AntPowerUpMain in iant_pwrp.c\n");
	    fprintf(stderr, "Copying current ant setup into local struct\n");
	    /* memcpy((void *) &antGlobal_c->LocalSetup, &asetup, sizeof(asetup)); */
	    /* return ANT_SETUP_CHANGED ; */
	  }
	}
      
      if ( (iClientType_a==ANT_IAM_CHATTER) &&
	   (ANTIFACE_ANTEXPORT==antGlobal_c->LocalSetup.iInterfaceType))
	{
	  MESSAGE istatus2 = ant_close( );
	  if( istatus2 != SS_NORMAL ) sig_signal( istatus2 );
	  return ANT_REMOTE_CHATTER;
	}
      return SS_NORMAL ;
    }

  /* If the global section was just created, then load the antenna
   * configuration file and initialize the semaphores.
   */
  istatus = ant_load_setup(&antGlobal_c->LocalSetup );
  if( istatus != SS_NORMAL ) sig_signal( istatus );

  /* If the system has no antenna, then clean up and exit now!
   */
  if (!antGlobal_c->LocalSetup.lAntAvailable)
    {
      istatus = ant_close( );
      if( istatus != SS_NORMAL ) sig_signal( istatus );
      return ANT_NO_ANTENNA;
    }

  if ( (iClientType_a==ANT_IAM_CHATTER) &&
       (ANTIFACE_ANTEXPORT==antGlobal_c->LocalSetup.iInterfaceType))
    {
      MESSAGE istatus2 = ant_close( );
      if( istatus2 != SS_NORMAL ) sig_signal( istatus2 );
      return ANT_REMOTE_CHATTER;
    }

  /* Copy local setup to operational version
   */
  antGlobal_c->setup = antGlobal_c->LocalSetup;

  /* If we are interfacing via AntExport, then attach to the remote
   * system to read the setup files.
   */
  if (ANTIFACE_ANTEXPORT == antGlobal_c->setup.iInterfaceType )
    {
      istatus = ReadRemoteSetup();
      if (istatus!=SS_NORMAL) return istatus;
    }

  /* Check to make sure we are not attempting to chat from a
   * non-controlling host.
  */
  if ( (iClientType_a==ANT_IAM_CHATTER) &&
       (!antGlobal_c->setup.lControllingHost))
    {
      MESSAGE istatus2 = ant_close( );
      if( istatus2 != SS_NORMAL ) sig_signal( istatus2 );
      return ANT_WRONG_CHATTER;
    }

  init_sem_set( LIBS_ANTENNA_CLUSTER ) ;

  /* Check that the antenna communications formats are valid, for
   * serial interface only.
   */
  if ( (ANTIFACE_SERIAL  == antGlobal_c->setup.iInterfaceType) ||
       (ANTIFACE_NETWORK == antGlobal_c->setup.iInterfaceType) )
    {
      if (antGlobal_c->setup.lControllingHost)
	switch (antGlobal_c->setup.icontrol_format)
	  {
	  case ANTFMT_XMT01:
	  case ANTFMT_XMT02:
	  case ANTFMT_XMT04:
	  case ANTFMT_XMT05:
	  case ANTFMT_XMTSA:
	    break;
	    
	  default:
	    return ANT_BAD_CONTROL;
	    break;
	  }

      switch (antGlobal_c->setup.istatus_format)
	{
	case ANTFMT_RCV01:
	case ANTFMT_RCV02:
	case ANTFMT_RCV03:
	case ANTFMT_RCV04:
	case ANTFMT_RCV05:
	case ANTFMT_RCVSA:
	  break;
	  
	default:
	  return ANT_BAD_STATUS;
	  break;
	}
      
      if (antGlobal_c->setup.lControllingHost)
	switch (antGlobal_c->setup.icontrol2_format)
	  {
	  case ANTFMT_XMTVOID:
	  case ANTFMT_XMT01:
	  case ANTFMT_XMT02:
	  case ANTFMT_XMT04:
	  case ANTFMT_XMT05:
	  case ANTFMT_XMTSA:
	    break;
	    
	  default:
	    return ANT_BAD_CONTROL;
	    break;
	  }

      switch (antGlobal_c->setup.istatus2_format)
	{
	case ANTFMT_RCVVOID:
	case ANTFMT_RCV01:
	case ANTFMT_RCV02:
	case ANTFMT_RCV03:
	case ANTFMT_RCV04:
	case ANTFMT_RCV05:
	case ANTFMT_RCVSA:
	  break;
	  
	default:
	  return ANT_BAD_STATUS;
	  break;
	}
    }

  /* - - - - - - - -
   * Do the first-time-only initializations
   */
  istatus = iant_init_csmasks();
  if (istatus != SS_NORMAL) return istatus;

  istatus = iant_init_first();
  if (istatus != SS_NORMAL) return istatus;

  /* Only the first caller needs to specify this
   */
  if (iOptions_a&ANT_OPTIONS_AUTO) antGlobal_c->info.lauto = TRUE;

  /* Start up the background logging process if it's needed.  Do this
   * before forking any other processes so that we can view any error
   * messages they might generate.
   */
  psdirectory = getenv( "IRIS_BIN" );

  if( antGlobal_c->setup.lLogToFile ) {
    char spath[PATHNAME_SIZE] ; SINT4 iCount = 0 ;
    antGlobal_c->info.iLogStatus = 0 ;
    strcpy( spath, psdirectory ) ; strcat( spath, "ant_log" ) ;
    istatus = sig_fork_daemon( spath, "ANT_LOG" ) ;
    if( istatus != SS_NORMAL ) return istatus;
    while( 0 == antGlobal_c->info.iLogStatus ) {
      if( iCount++ > 100 ) return iMessageMod_va( USER_PROCESS_START, 1, "ant_log" );
      sig_microSleep( 50000 ) ;
    }
    if( antGlobal_c->info.iLogStatus != SS_NORMAL )
      return( antGlobal_c->info.iLogStatus ) ;
  }

  /*  - - - - - - - -
   * If there is no interface to the RCP, then we are done.  Otherwise
   * proceed with starting up the support processes.
   */
  if (ANTIFACE_NONE == antGlobal_c->setup.iInterfaceType) return SS_NORMAL;

  /* Optionally start up the simulator package.  Must be done after
   * call to iant_init_first.
   */
  if ( antGlobal_c->setup.lStartSimulator &&
       (ANTIFACE_SERIAL == antGlobal_c->setup.iInterfaceType))
    {
      antGlobal_c->info.lSimRcvExpected = TRUE;
      antGlobal_c->info.lSimXmtExpected = TRUE;
      istatus = iantsim_pwrp_opt( antGlobal_c->info.lauto ) ;
    }

  /* Start up processes as required to handle this antenna.
   */
  if (ANTIFACE_NONE != antGlobal_c->setup.iInterfaceType)
    {
      istatus = fork_pair( psdirectory, &(antGlobal_c->info.io[0]), 0 );
      if (istatus != SS_NORMAL) return istatus;
    }

  if( (ANTIFACE_ANTEXPORT != antGlobal_c->setup.iInterfaceType) &&
      ( (ANTFMT_RCVVOID != antGlobal_c->setup.istatus2_format) ||
	((ANTFMT_XMTVOID != antGlobal_c->setup.icontrol2_format) &&
	 (antGlobal_c->setup.lControllingHost) ) ) )
    {
      istatus = fork_pair( psdirectory, &(antGlobal_c->info.io[1]), 1 );
      if (istatus != SS_NORMAL) return istatus;
    }

  /* Request a transmission from the BITE(s) as soon as processes are
   * there to handle it
   */
  antGlobal_c->binfo.lInterrogateBite  = TRUE;

  return SS_NORMAL;
}

UINT1 lAntSimulator( void )
{
  return lIAmSimulator_c;
}

/* ------------------------------
 * Read the remote setup information, and merge it into our
 * version.
 */
static MESSAGE ReadRemoteSetup(void)
{
  MESSAGE iStatus;
  int iSocket, ierror;
  iStatus = iAntOpenSocket( &iSocket ) ;
  if( iStatus != SS_NORMAL ) return iStatus;

  /* Once we have opened the socket, we must close it */

  /* Ask for the server's setup information */
  iStatus = WriteSocketString( iSocket, "Setup", FALSE );
  if (iStatus==SS_NORMAL)
    {
      struct lim_memory SetupAscii;
      struct ant_manual_setup SetupBinary;
      /* Read back the setup structure */
      SetupAscii.sPointer = ReadSocketPacketTcp( iSocket, &SetupAscii.iSize, FALSE, 0,
					      NULL );
      iStatus = CheckSocketAck( SetupAscii.sPointer, SetupAscii.iSize );
      if (iStatus==SS_NORMAL)
	{
	  struct lim_memory Setup; /* New copy to increment the position */
	  Setup = SetupAscii;
	  Setup.sPointer+=4;  /* Skip the "Ack|" */
	  Setup.iSize   -=4;
	  /* Convert to binary structure */
	  iStatus = lim_loadMemory( &Setup, "ant_manual_setup", &SetupBinary,
				    setup_limits );
	}
      free( SetupAscii.sPointer );
      if (iStatus==SS_NORMAL) 
	{
	  /* Copy into local storage.  We replace the whole setup structure
	   * except the interface specs.
	   */
	  antGlobal_c->setup = SetupBinary;
	  
	  antGlobal_c->setup.iInterfaceType    = antGlobal_c->LocalSetup.iInterfaceType;
	  memcpy( (char *)antGlobal_c->setup.sAntExportHost, 
		  (char *)antGlobal_c->LocalSetup.sAntExportHost, 40 );
	  antGlobal_c->setup.iAntExportPortNum = antGlobal_c->LocalSetup.iAntExportPortNum;
	  antGlobal_c->setup.iNetDelayMs       = antGlobal_c->LocalSetup.iNetDelayMs;
	  antGlobal_c->setup.iAngleSource      = antGlobal_c->LocalSetup.iAngleSource;

	}
    }

  /* Close the socket, if there was an error before returning */
  
  ierror = close( iSocket ) ;
  if (ierror == -1)
    sig_signal( errno_to_message_va( errno, 1, "ReadRemoteSetup()") ) ;

  return iStatus;
}      
/* ------------------------------
 * Processes the specified client type.  Signals errors, and
 * returns zero if there is an error which should prevent the
 * program from running.
 */
static MESSAGE ProcessClientType( SINT4 iClientType_a, UINT1 lCreated_a )
{
  lIAmSimulator_c = FALSE ;
  switch( iClientType_a )
    {
    case ANT_IAM_CLIENT:
    case ANT_IAM_EXPORT:
    case ANT_IAM_RCP:
    case ANT_IAM_RVP:
      break;
    
    case ANT_IAM_SIM_RCV:
      if (lCreated_a) return ANT_SIM_STARTFIRST;
      if (antGlobal_c->info.iSimRcvPid) return ANT_TWO_SIM_RCV;
      else {
	antGlobal_c->info.iSimRcvPid = getpid();
	lIAmSimulator_c = TRUE;
      }
      break;

    case ANT_IAM_SIM_XMT:
      if (lCreated_a) return ANT_SIM_STARTFIRST;
      if (antGlobal_c->info.iSimXmtPid) return ANT_TWO_SIM_XMT;
      else {
	antGlobal_c->info.iSimXmtPid = getpid();
	lIAmSimulator_c = TRUE;
      }
      break;

    case ANT_IAM_ANT_RCV0:
      if (lCreated_a) return ANT_STARTFIRST;
      if (antGlobal_c->info.io[0].iRcvPid) return ANT_TWO_RCV;
      else antGlobal_c->info.io[0].iRcvPid = getpid();
      break;

    case ANT_IAM_ANT_RCV1:
      if (lCreated_a) return ANT_STARTFIRST;
      if (antGlobal_c->info.io[1].iRcvPid) return ANT_TWO_RCV;
      else antGlobal_c->info.io[1].iRcvPid = getpid();
      break;

    case ANT_IAM_ANT_XMT0:
      if (lCreated_a) return ANT_STARTFIRST;
      if (antGlobal_c->info.io[0].iXmtPid) return ANT_TWO_XMT;
      else antGlobal_c->info.io[0].iXmtPid = getpid();
      break;

    case ANT_IAM_ANT_XMT1:
      if (lCreated_a) return ANT_STARTFIRST;
      if (antGlobal_c->info.io[1].iXmtPid) return ANT_TWO_XMT;
      else antGlobal_c->info.io[1].iXmtPid = getpid();
      break;

    case ANT_IAM_LOGGER:
      if (lCreated_a) return ANT_STARTFIRST;
      if (antGlobal_c->info.iLogPid) return ANT_TWO_LOG;
      else antGlobal_c->info.iLogPid = getpid();
      break;

    case ANT_IAM_QANT:
      if (lCreated_a) return ANT_STARTFIRST;
      antGlobal_c->info.lShuttingDown = TRUE ;
      break;

    case ANT_IAM_CHATTER:
      if (antGlobal_c->info.iChatPid) return ANT_TWO_CHATTER;
      else {
	antGlobal_c->info.iChatPid = getpid();
	/* Initialize the destination back to wild for each user */
	antGlobal_c->info.iChatDestination = htonl(INADDR_ANY);
      }
      break;

    default:
      return ANT_CLIENT_INVALID;
      break;
    }
  return SS_NORMAL;
}

/* ------------------------------
 * Fork a pair of receive and transmit processes.
 */
static MESSAGE fork_pair
( const char *pdirectory_a,
 volatile struct io_information *pio_a,
 UINT4 iChannel_a )
{
  MESSAGE istatus;
  SINT4 icount, ierror, index ;
  char spath[PATHNAME_SIZE], sname[40] ;
  volatile char *sdevice;
  struct stat stat_buf;

  switch (antGlobal_c->setup.iInterfaceType)
    {
    case ANTIFACE_SERIAL:
      /* Refuse to proceed if the device file cannot be found. A zero
       * length string means to use a shared memory connection.
       */
      if (iChannel_a) sdevice = antGlobal_c->setup.schannel2 ;
      else          sdevice = antGlobal_c->setup.schannel1 ;
      
      icount = strlen( (char *)sdevice );
      if( icount >= sizeof(antGlobal_c->setup.schannel1)) return ANT_BAD_DEVICE;
      for( index=0 ; index < icount ; index++ )
	if( ! isgraph(sdevice[index] ) ) return ANT_BAD_DEVICE;
      
      if( sdevice[0] ) {
	ierror = stat( (char *)sdevice, &stat_buf );
	if (ierror) return ANT_BAD_DEVICE;
      }
      break ;

    case ANTIFACE_NETWORK:
      /* Check for garbage.  Note that channel 1 is always serial
       */
      if (iChannel_a==0) 
	{
	  sdevice = antGlobal_c->setup.schannel1 ;
      
	  icount = strlen( (char *)sdevice );
	  if( icount >= sizeof(antGlobal_c->setup.schannel1)) return ANT_BAD_DEVICE;
	  if( icount < 1) return ANT_BAD_DEVICE;
	  for( index=0 ; index < icount ; index++ )
	    if( ! isgraph(sdevice[index] ) ) return ANT_BAD_DEVICE;
	}
      break ;
    }

  pio_a->lRcvExpected = TRUE;
  if (antGlobal_c->setup.lControllingHost) pio_a->lXmtExpected = TRUE;

  /* Special rules for the aux channel, either direction can be turned off */
  if (iChannel_a)
    {
      if (ANTFMT_RCVVOID == antGlobal_c->setup.istatus2_format) pio_a->lRcvExpected=FALSE;
      if (ANTFMT_XMTVOID == antGlobal_c->setup.icontrol2_format) pio_a->lXmtExpected=FALSE;
    }

  if (pio_a->lRcvExpected)
    {
      /* Startup the receive process and wait up to 5 seconds for it to
       * start.
       */
      pio_a->ircv_status = 0 ;
      strcpy( spath, pdirectory_a ) ; strcat( spath, "ant_rcv" ) ;
      strcpy( sname, "ANT_RCV" ) ; if (iChannel_a) strcat( sname, "2" );
      istatus = sig_fork_daemon( spath, sname ) ;
      if( istatus != SS_NORMAL ) return istatus;
      
      icount = 0;
      while( 0 == pio_a->ircv_status )
	{
	  if( icount++ > 100 ) return iMessageMod_va( USER_PROCESS_START, 1, "ant_rcv" );
	  sig_microSleep( 50000 ) ;
	}
      if ( SS_NORMAL != pio_a->ircv_status )  return pio_a->ircv_status;
    }
  
  if (pio_a->lXmtExpected)
    {
      /* Startup the transmit process and wait up to 5 seconds for it to
       * start.
       */
      pio_a->ixmt_status = 0 ;
      strcpy( spath, pdirectory_a ) ; strcat( spath, "ant_xmt" ) ;
      strcpy( sname, "ANT_XMT" ) ; if (iChannel_a) strcat( sname, "2" );
      istatus = sig_fork_daemon( spath, sname ) ;
      if( istatus != SS_NORMAL ) return istatus;
      
      icount = 0;
      while( 0 == pio_a->ixmt_status ) 
	{
	  if( icount++ > 100 ) return iMessageMod_va( USER_PROCESS_START, 1, "ant_xmt" );
	  sig_microSleep( 50000 ) ;
	}
      if (SS_NORMAL != pio_a->ixmt_status) return pio_a->ixmt_status;
    }

  return SS_NORMAL;
}

/* ------------------------------
 * Routine to initialize control and status bit fields.
 *
 * History:
 *   jmh   18 Oct 1991   Init (split from main routine)
 */
static MESSAGE iant_init_csmasks( void )
{
  UINT4 itemp_mask ; MESSAGE istatus = SS_NORMAL ;

  /* Convert the manual setup structure into a more easily used format
   */
  itemp_mask = 0;
  if ((ANTIFACE_NONE != antGlobal_c->setup.iInterfaceType) &&
      (antGlobal_c->setup.lControllingHost))
    {
      if (antGlobal_c->setup.cont_servo_power.lenabled) itemp_mask |= ACF_SERVO;
      if (antGlobal_c->setup.cont_radiate.lenabled)     itemp_mask |= ACF_RADIATE;
      if (antGlobal_c->setup.cont_tr_power.lenabled)    itemp_mask |= ACF_TRPOWER;
      itemp_mask |= ACF_IRIS;
      if (antGlobal_c->setup.cont_reset.lenabled)       itemp_mask |= ACF_RESET;
      itemp_mask |= ACF_AUTO_RAD;
      if (antGlobal_c->setup.cont_noise.lenabled)       itemp_mask |= ACF_NOISE;
      if (antGlobal_c->setup.cont_siggen.lenabled)      itemp_mask |= ACF_SIGGEN;
      if (antGlobal_c->setup.cont_sigcw.lenabled)       itemp_mask |= ACF_SIGCW;
      if (antGlobal_c->setup.cont_pwbw.lenabled)        itemp_mask |= ACF_PWBW;
      if (antGlobal_c->setup.cont_pw_unset.lenabled)    itemp_mask |= ACF_PW_UNSET;
      if (antGlobal_c->setup.cont_nst.lenabled)         itemp_mask |= ACF_NST;
      if (antGlobal_c->setup.cont_polar.lenabled)       itemp_mask |= ACF_POLAR;
      itemp_mask |= ACF_ANTENNA;
    }
  antGlobal_c->info.ic_mask = itemp_mask;

  itemp_mask = 0;
  if (ANTIFACE_NONE != antGlobal_c->setup.iInterfaceType)
    {
      if (antGlobal_c->setup.status_low_airflow.lenabled)    itemp_mask |= ASF_LOWAIRFLOW;
      if (antGlobal_c->setup.status_low_wgp.lenabled)        itemp_mask |= ASF_LOW_WGP;
      if (antGlobal_c->setup.status_interlock_open.lenabled) itemp_mask |= ASF_INTERLOCK;
      if (antGlobal_c->setup.status_standby.lenabled)        itemp_mask |= ASF_STANDBY;
      if (antGlobal_c->setup.status_radiate.lenabled)        itemp_mask |= ASF_RADIATE;
      if (antGlobal_c->setup.status_mag_current.lenabled)    itemp_mask |= ASF_MAGTFLT;
      if (antGlobal_c->setup.status_local.lenabled)          itemp_mask |= ASF_LOCAL;
      if (antGlobal_c->setup.status_trlocal.lenabled)        itemp_mask |= ASF_TRLOCAL;
      if (antGlobal_c->setup.status_servo_power.lenabled)    itemp_mask |= ASF_SERVO;
      if (antGlobal_c->setup.status_tr_power.lenabled)       itemp_mask |= ASF_TRPOWER;
      if (antGlobal_c->setup.status_shutdown.lenabled)       itemp_mask |= ASF_SHUTDOWN;
      if (antGlobal_c->setup.status_sigfault.lenabled)       itemp_mask |= ASF_SIGFAULT;
      if (antGlobal_c->setup.status_siggen.lenabled)         itemp_mask |= ASF_SIGGEN;
      if (antGlobal_c->setup.status_sigcw.lenabled)          itemp_mask |= ASF_SIGCW;
      if (antGlobal_c->setup.status_pwbw.lenabled)           itemp_mask |= ASF_PWBW;
      if (antGlobal_c->setup.status_polar.lenabled)          itemp_mask |= ASF_POLAR;
      if (antGlobal_c->setup.status_calib_az.lenabled)       itemp_mask |= ASF_CALIB_AZ;
      if (antGlobal_c->setup.status_calib_el.lenabled)       itemp_mask |= ASF_CALIB_EL;
    }
  antGlobal_c->info.is_mask = itemp_mask;

  itemp_mask = 0;
  if (ANTIFACE_NONE != antGlobal_c->setup.iInterfaceType)
    {
      if (antGlobal_c->setup.status_low_airflow.lCritical)    itemp_mask |= ASF_LOWAIRFLOW;
      if (antGlobal_c->setup.status_low_wgp.lCritical)        itemp_mask |= ASF_LOW_WGP;
      if (antGlobal_c->setup.status_interlock_open.lCritical) itemp_mask |= ASF_INTERLOCK;
      if (antGlobal_c->setup.status_standby.lCritical)        itemp_mask |= ASF_STANDBY;
      if (antGlobal_c->setup.status_radiate.lCritical)        itemp_mask |= ASF_RADIATE;
      if (antGlobal_c->setup.status_mag_current.lCritical)    itemp_mask |= ASF_MAGTFLT;
      if (antGlobal_c->setup.status_local.lCritical)          itemp_mask |= ASF_LOCAL;
      if (antGlobal_c->setup.status_trlocal.lCritical)        itemp_mask |= ASF_TRLOCAL;
      if (antGlobal_c->setup.status_servo_power.lCritical)    itemp_mask |= ASF_SERVO;
      if (antGlobal_c->setup.status_tr_power.lCritical)       itemp_mask |= ASF_TRPOWER;
      if (antGlobal_c->setup.status_shutdown.lCritical)       itemp_mask |= ASF_SHUTDOWN;
      if (antGlobal_c->setup.status_sigfault.lCritical)       itemp_mask |= ASF_SIGFAULT;
      if (antGlobal_c->setup.status_siggen.lCritical)         itemp_mask |= ASF_SIGGEN;
      if (antGlobal_c->setup.status_sigcw.lCritical)          itemp_mask |= ASF_SIGCW;
      if (antGlobal_c->setup.status_pwbw.lCritical)           itemp_mask |= ASF_PWBW;
      if (antGlobal_c->setup.status_calib_az.lCritical)       itemp_mask |= ASF_CALIB_AZ;
      if (antGlobal_c->setup.status_calib_el.lCritical)       itemp_mask |= ASF_CALIB_EL;
    }
  /* Only allow critical faults on bits which are faultable
   */
  antGlobal_c->info.iStatusCritical = itemp_mask & ASFLAGS_FAULTABLE;

  return( istatus ) ;
}

/* ------------------------------
 * Routine to initialize things when we are the first call to iant_pwrp.
 */
static MESSAGE iant_init_first( void )
{
  SINT4 ii, iunit ; UINT4 itemp_mask, iBtimeNow ;
  MESSAGE istatus = SS_NORMAL ;
  struct ymds_time YmdsNow;

  getbtime( &iBtimeNow ) ;
  GetTimeYmds( &YmdsNow, TIME_FLG_UTC );

  /* Initialize all of the antenna_info structure.  It was already zeroed,
   * and may have a client PID inserted.
   */
  antGlobal_c->info.iazmode     = antGlobal_c->setup.iazmode_initial;
  antGlobal_c->info.ielmode     = antGlobal_c->setup.ielmode_initial;

  antGlobal_c->info.idaz        = antGlobal_c->setup.iazpos_initial;
  antGlobal_c->info.idel        = antGlobal_c->setup.ielpos_initial;
  antGlobal_c->info.fdazv       = antGlobal_c->setup.fazv_initial;
  antGlobal_c->info.fdelv       = antGlobal_c->setup.felv_initial;
  {
    /* Initialize the pedestal parameters realtime model.
     */
    volatile struct rtvec_info *pedxInfo ;
    UINT1 iModulos[ RTVECMAXDIM ] = { ANTPEDX_MODS } ;

    pedxInfo = &antGlobal_c->vdat.rtpedx_history.info ;
    rtvecInitFirst( pedxInfo, ANTPEDX_SLOTS, ANTPEDX_TERMS ) ;
    pedxInfo->iSlotWidthMS = ANTPEDX_WIDTH ;
    memcpy( (void *)pedxInfo->iModulos, iModulos, RTVECMAXDIM ) ;
  }
  {
    /* Initialize the shipboard parameters realtime model.  Latitude
     * and longitude are initialized only the first time, because they
     * may be overwritten by information from the antenna, and we do
     * not want to have a glitch
     */
    volatile struct rtvec_info *shipInfo ; struct rtship rtship ;
    UINT1 iModulos[ RTVECMAXDIM ] = { ANTSHIP_MODS } ;

    shipInfo = &antGlobal_c->vdat.rtship_history.info ;
    rtvecInitFirst( shipInfo, ANTSHIP_SLOTS, ANTSHIP_TERMS ) ;
    shipInfo->iSlotWidthMS = ANTSHIP_WIDTH ;
    memcpy( (void *)shipInfo->iModulos, iModulos, RTVECMAXDIM ) ;

    for( ii=0 ; ii < ANTSHIP_TERMS ; ii++ ) ((FLT8 *)&rtship)[ii] = 0.0 ;
    rtship.flatitude  = fDegFromBin4(antGlobal_c->setup.ilat);
    rtship.flongitude = fDegFromBin4(antGlobal_c->setup.ilon);
    rtship.faltitude  = (FLT8)(antGlobal_c->setup.ignd_hgt + antGlobal_c->setup.irad_hgt) ;
    rtvecInsertPoint( &antClient_c.ship, 0, (void *)&rtship ) ;
  }
  /* Convert the manual setup structure into a more easily used
   * format.  Note that ic_mask and is_mask are set in the routine
   * iant_init_csmasks().
   */
  itemp_mask = 0;
  if (antGlobal_c->setup.cont_servo_power.linitial_high) itemp_mask |= ACF_SERVO;
  if (antGlobal_c->setup.cont_radiate.linitial_high)     itemp_mask |= ACF_RADIATE;
  if (antGlobal_c->setup.cont_tr_power.linitial_high)    itemp_mask |= ACF_TRPOWER;
  if (antGlobal_c->setup.cont_reset.linitial_high)       itemp_mask |= ACF_RESET;
  if (antGlobal_c->setup.cont_noise.linitial_high)       itemp_mask |= ACF_NOISE;
  if (antGlobal_c->setup.cont_siggen.linitial_high)      itemp_mask |= ACF_SIGGEN;
  if (antGlobal_c->setup.cont_sigcw.linitial_high)       itemp_mask |= ACF_SIGCW;

  antGlobal_c->info.icbits       = itemp_mask;
  antGlobal_c->info.RadiateOnYmds = YmdsNow;
  antGlobal_c->info.isbits       = 0;

  antGlobal_c->info.iActualPolar  = 0;
  antGlobal_c->info.iDesiredPolar = POL_UNCHANGED;
  antGlobal_c->info.iActualPwbw   = 0;
  antGlobal_c->info.iDesiredPwbw  = ANT_PWBW_UNCHANGED;
  antGlobal_c->info.idsiggen_value = antGlobal_c->setup.isiggen_value_initial;
  antGlobal_c->info.scurrent_rst_mode[0] = 0;
  antGlobal_c->info.icurrent_rst_mode    = 0;
  antGlobal_c->info.irequested_rst_mode  = 0;

  if (antGlobal_c->setup.cont_nst.linitial_high == 
      antGlobal_c->setup.cont_nst.lhigh) 
    antGlobal_c->info.icurrent_nst_faults = 0x0f;
  else
    antGlobal_c->info.icurrent_nst_faults = 0;

  antGlobal_c->info.iNstTimeout = 120000 ;
  antGlobal_c->info.iNstUpdateTimes[0] = iBtimeNow ;
  antGlobal_c->info.iNstUpdateTimes[1] = iBtimeNow ;
  antGlobal_c->info.iNstUpdateTimes[2] = iBtimeNow ;
  antGlobal_c->info.iNstUpdateTimes[3] = iBtimeNow ;

  antGlobal_c->info.iBtimePkt  = iBtimeNow ;
  antGlobal_c->info.iBtimeAng  = iBtimeNow ;
  antGlobal_c->info.iBtimeShip = iBtimeNow ;

  antGlobal_c->info.iactive       = 0;
  antGlobal_c->info.ichange_count = 0;

  antGlobal_c->info.lwanted_to_toggle = FALSE;
  antGlobal_c->info.ltime_packet_received = FALSE;
  antGlobal_c->info.ltime_set_from_packet = FALSE ;
  antGlobal_c->info.lset_system_time = FALSE ;
  antGlobal_c->info.iLastTimeChange = 0;
  antGlobal_c->info.itime_error  = 0;
  antGlobal_c->info.itime_btime  = 0;
  antGlobal_c->info.time_in_packet.iyear = 0;
  antGlobal_c->info.time_in_packet.imon  = 0;
  antGlobal_c->info.time_in_packet.iday  = 0;
  antGlobal_c->info.time_in_packet.isec  = 0;

  /* Load the Bite definition information, and set local state.  The
   * file load is not done every time, because it cannot be changed
   * without changing the memory copy
   */
  istatus = bite_load_def() ;
  if( istatus != SS_NORMAL ) {
    /* If there is an error, then set the unit and field counts
     * to zero.
     */
    antGlobal_c->bitex.iunit_cnt  = 0 ;
    antGlobal_c->bitex.ifield_cnt = 0 ;

  } else {
    /* If the load was okay, then set default data states for all
     * fields that are control fields.
     */
    SINT4 ifield ;
      
    for( ifield=0 ; ifield < antGlobal_c->bitex.ifield_cnt ; ifield++ ) {
      volatile struct bitex_field_def *pfdef = &antGlobal_c->bitex.fields[ifield] ;
      if( ( antGlobal_c->bitex.units[ pfdef->iunit ].lcontrol_unit ) &&
	  ( !pfdef->ldisabled) ) {
	if( pfdef->cs.cnt.button_flags & BITE_BUTTON_PULSE ) {
	  /* Pulse buttons always initialize to out */
	  if( !pfdef->cs.cnt.lbutton_in_high )
	    antGlobal_c->binfo.data_mask[ ifield >> 5 ] |= (1 << (ifield & 0x1F)) ;
	} else {
	  /* Toggle buttons initialize based on initial state */
	  if( (0 != (pfdef->cs.cnt.button_flags & BITE_BUTTON_DFLTIN)) ==
	      (0 != (pfdef->cs.cnt.lbutton_in_high                  )) )
	    antGlobal_c->binfo.data_mask[ ifield >> 5 ] |= (1 << (ifield & 0x1F)) ;
	}
      }
    }
  }

  for( iunit=0 ; iunit < BITE_MAX_UNITS ; iunit++ ) {
    antGlobal_c->binfo.Unit[iunit].iin_cnt = 0 ;
    antGlobal_c->binfo.Unit[iunit].iout_cnt = 0 ;
  }
  antGlobal_c->binfo.idiag_size = 0 ;

  /* Initialize the chat-mode byte FIFOs
   */
  bfifo_init( &antGlobal_c->vdat.ChatIn , sizeof(antGlobal_c->vdat.ChatIn ) ) ;
  bfifo_init( &antGlobal_c->vdat.ChatOut, sizeof(antGlobal_c->vdat.ChatOut) ) ;

  /* Initialize the internal simulator link FIFOs
   */
  bfifo_init( &antGlobal_c->vdat.FromSimulator, sizeof(antGlobal_c->vdat.FromSimulator) ) ;
  bfifo_init( &antGlobal_c->vdat.ToSimulator,   sizeof(antGlobal_c->vdat.ToSimulator  ) ) ;

  /* Initialize the file logging FIFO
   */
  bfifo_init( &antGlobal_c->vdat.LogFIFO, sizeof(antGlobal_c->vdat.LogFIFO) ) ;

  /* Set flag indicating whether a dual system RCP is configured
   */
  switch( antGlobal_c->setup.istatus_format ) {
  case ANTFMT_RCV05:
    antGlobal_c->info.drcp.lSetupDual = TRUE  ; break ;
  default:
    antGlobal_c->info.drcp.lSetupDual = FALSE ; break ;
  }

  return( istatus ) ;
}

/* ------------------------------
 * Map the Antenna global section if it has not been done already by this
 * process.  Lcreated_a is set to non-zero if the global section was created.
 */
static MESSAGE ant_global_init
( UINT2 *lcreated_a )
{
  MESSAGE istatus;

  *lcreated_a = FALSE ;

  istatus = sig_shmem_map
    ( sizeof(struct antenna_global), "iris_antenna", -1,
      (void *)&antGlobal_c, &antenna_size_c, &antenna_id_c, lcreated_a ) ;
  
  if( istatus == SS_NORMAL )
    {
      if( *lcreated_a ) memset( (void *)antGlobal_c, 0, antenna_size_c );
    }

  return( istatus ) ;
}

/* ------------------------------
 * UnMap the shared memory, and disconnect this process from the
 * antenna library.  Use this when you no longer need the antenna
 * library.  For multi-threaded programs, call this after all threads
 * are closed, for example in the exit handler.
 */
MESSAGE ant_close( void )
{
  MESSAGE istatus = SS_NORMAL ;

  if( lPoweredUp_c ) {
    switch( antClient_c.iClientType )
      {
      case ANT_IAM_CLIENT:
      case ANT_IAM_EXPORT:
      case ANT_IAM_RCP:
      case ANT_IAM_RVP:
      default:
	break;
    
      case ANT_IAM_SIM_RCV:
	antGlobal_c->info.iSimRcvPid = 0;
	break;
    
      case ANT_IAM_SIM_XMT:
	antGlobal_c->info.iSimXmtPid = 0;
	break;

      case ANT_IAM_ANT_RCV0:
	antGlobal_c->info.io[0].iRcvPid = 0;
	break;

      case ANT_IAM_ANT_RCV1:
	antGlobal_c->info.io[1].iRcvPid = 0;
	break;

      case ANT_IAM_ANT_XMT0:
	antGlobal_c->info.io[0].iXmtPid = 0;
	break;

      case ANT_IAM_ANT_XMT1:
	antGlobal_c->info.io[1].iXmtPid = 0;
	break;

      case ANT_IAM_QANT:
	kill_sem_set( LIBS_ANTENNA_CLUSTER ) ;
	break;

      case ANT_IAM_CHATTER:
	antGlobal_c->info.iChatPid = 0;
	break;
      }
    istatus = sig_shmem_unmap( antGlobal_c, antenna_size_c, antenna_id_c ) ;
    lPoweredUp_c = FALSE ;
  }
  return( istatus ) ; 
}

/* ------------------------------
 * Lock/Unlock routines for shared structures
 */
static void antPedxLock( void ) {
  if( ! antGlobal_c->info.lShuttingDown )
    lock_sem( ANTENNA_PEDX_MODEL_SEM ) ;
}
static void antPedxUnLock( void ) {
  if( ! antGlobal_c->info.lShuttingDown )
    unlock_sem( ANTENNA_PEDX_MODEL_SEM ) ;
}
static void antShipLock( void ) {
  if( ! antGlobal_c->info.lShuttingDown )
    lock_sem( ANTENNA_SHIP_MODEL_SEM ) ;
}
static void antShipUnLock( void ) {
  if( ! antGlobal_c->info.lShuttingDown )
    unlock_sem( ANTENNA_SHIP_MODEL_SEM ) ;
}

/* ------------------------------
 * Determine whether this client is permitted to write antenna angles
 * into the library.
 */
UINT1 lAntCanInsertAngles( void )
{
  UINT1 lOkay = FALSE ;
  switch( antGlobal_c->setup.iAngleSource ) {
  case ANTANGSRC_NORMAL:
    lOkay = ( ( antClient_c.iClientType == ANT_IAM_ANT_RCV0 ) ||
	      ( antClient_c.iClientType == ANT_IAM_ANT_RCV1 ) ) ;
    break ;
  case ANTANGSRC_RVP:
    lOkay = ( antClient_c.iClientType == ANT_IAM_RVP ) ;
    break ;
  case ANTANGSRC_RCP:
    lOkay = ( antClient_c.iClientType == ANT_IAM_RCP ) ;
    break ;
  }
  return( lOkay ) ;
}

/* ------------------------------
 * Return a pointer to the setup information the library has.
 */
const struct ant_manual_setup *AntGetSetupPointer( void )
{
  /* Cast discards volatile because this does not change
   */
  return (const struct ant_manual_setup*) &(antGlobal_c->setup);
}

/* ------------------------------
 * Return a short character string describing a client type.
 */
const char *sAntClientType( SINT4 iClientType_a )
{
  switch( iClientType_a ) {
  case ANT_IAM_CLIENT  : { static const char s[5]={"CLI" } ; return(s) ; }
  case ANT_IAM_EXPORT  : { static const char s[5]={"EXP" } ; return(s) ; }
  case ANT_IAM_RCP     : { static const char s[5]={"RCP" } ; return(s) ; }
  case ANT_IAM_RVP     : { static const char s[5]={"RVP" } ; return(s) ; }
  case ANT_IAM_SIM_RCV : { static const char s[5]={"SIMR"} ; return(s) ; }
  case ANT_IAM_SIM_XMT : { static const char s[5]={"SIMX"} ; return(s) ; }
  case ANT_IAM_ANT_RCV0: { static const char s[5]={"RCV0"} ; return(s) ; }
  case ANT_IAM_ANT_RCV1: { static const char s[5]={"RCV1"} ; return(s) ; }
  case ANT_IAM_ANT_XMT0: { static const char s[5]={"XMT0"} ; return(s) ; }
  case ANT_IAM_ANT_XMT1: { static const char s[5]={"XMT1"} ; return(s) ; }
  case ANT_IAM_QANT    : { static const char s[5]={"QANT"} ; return(s) ; }
  case ANT_IAM_CHATTER : { static const char s[5]={"CHAT"} ; return(s) ; }
  case ANT_IAM_LOGGER  : { static const char s[5]={"LOG" } ; return(s) ; }
  }
  { static const char s[5]={"????"} ; return(s) ; }
}
