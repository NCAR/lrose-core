/*
 * NexRadMgr implementation
 */

#ifdef sgi
#define _SGI_MP_SOURCE
#include <sys/bsd_types.h>
#endif


#ifdef STDCPPHEADERS
#include <iostream>
#else
#include <iostream.h>
#endif

#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include "utils.h"
#include <libgen.h>
#include <unistd.h>
#include "linkedlisttemplate.h"
#include <stdio.h>
#include <values.h>
#include <siteinfo.h>
#include <rdrscan.h>
#include "NexRadMgr.h"
#include <time.h>
static char const cvsid[] = "$Id: NexRadMgr.C,v 1.2 2008/02/29 04:44:54 dixon Exp $";

NexRadMgr *NexRadManager = 0;

//replace arrays of NexRadStnHdlr with linked list later
//similarly array of output sockets
//check array bounds in the meantime!!
//check deletion heirarchies

NexRadMgr::NexRadMgr() : scan_client()
{
  //read init file nexrad.ini
  Init_NexRad();
  if (!NexRadFlag) return;

  ClientType               = SC_NEXRADMNG;
  lock                     = new spinlock("NexRadMgr->lock", 3000);    // 30 secs
  if (completeVol) {             //SD 3may07
     AcceptNewScans        = FALSE;
     AcceptFinishedScans   = TRUE;
  }
  else {
     AcceptNewScans        = TRUE;
     AcceptFinishedScans   = FALSE;
  }
  
  //AcceptNewScans           = TRUE;  //remove these!!
  //AcceptFinishedScans      = FALSE;
  AllowReplayMode = true;  // allow replay mode
  AllowDBReviewMode = true;  // allow DBReview mode
  printf("NexRadMgr::start\n");  
  if (ScanMng && NexRadFlag) ScanMng->AddClient(this);
}	

NexRadMgr::~NexRadMgr() 
{
  //  int waittime = 30;
  
  if (ScanMng && NexRadFlag) ScanMng->RemClient(this);
  fprintf(stderr, "NexRadMgr::~NexRadMgr Stopping NexRadStnHdlrs....\n");
    
  //stop each NexRadStnHdlr, go thru linked list of instances
  for (int i=0;i<stn_count;i++)
    stn_init[i].nexradstn->stopThread();
  if (lock) 
    {
      lock->rel_lock();
      delete lock;
      lock = 0;
    }
}

void NexRadMgr::StartNexRadStnHdlrs() 
{
  if (!NexRadFlag) return;
  //start each NexRadStnHdlr in turn, as defined by nexrad.ini
	  

  for (int i=0;i<stn_count;i++)
    {
	  //debug SD 3/10/05
	  //fprintf(stderr,"NexRadMgr::StartNexRadStnHdlrs:DEBUG stn_init[i].vcp_locn = %s\n",stn_init[i].vcp_locn);
      stn_init[i].nexradstn = new NexRadStnHdlr( stn_init[i].station,
						 stn_init[i].path,
						 stn_init[i].outputdesc,
						 stn_init[i].outputcount,
						 stn_init[i].radarType,
						 stn_init[i].packet_size,
						 stn_init[i].scan_patt_arr, //SD rem pattern, inst this 22/8/00
						 stn_init[i].scan_patt_count, 
						 stn_init[i].vcp_locn, 
						 stn_init[i].delay,
						 stn_init[i].min_time_per_tilt, 
						 stn_init[i].start_local_port,
						 debug,
						 LoadFromDB,
						 NexRadFlag,
						 writeSimRapic,
						 completeVol,
						 ignoreFutureData,
						 stn_init[i].noTempFile,
						 stn_init[i].filter_Vel,
						 stn_init[i].filter_Refl,
						 stn_init[i].filter_SpWdth,
						 stn_init[i].refl_rngres_reduce_factor,
						 stn_init[i].vel_rngres_reduce_factor);
    }
  
}

int  NexRadMgr::NewDataAvail(rdr_scan *newscan) 
{
  //pass this call onto each NexRadStnHdlr in turn

  if (!NexRadFlag) return 1;
  if (newscan && AcceptNewScans && ScanSourceOK(newscan)) 
    {
      for (int i=0;i<stn_count;i++)
	stn_init[i].nexradstn->NewDataAvail(newscan);
    }
  return 1;
}


int  NexRadMgr::NewDataAvail(CData *newdata) 
{
  //not interested in Cdata
  return 0;
}

// Don't add scan via both New and FInished methods
// If AcceptNewScans defined, don't use FinishedDataAvail
int  NexRadMgr::FinishedDataAvail(rdr_scan *finishedscan) 
{
  //pass this call onto each NexRadStnHdlr in turn
  //  rdr_scan_node   *newscannode;	
  int	    result = 0;
  if (!NexRadFlag) return 1;
  if (finishedscan && !AcceptNewScans && AcceptFinishedScans &&
      ScanSourceOK(finishedscan)) {
    for (int i=0;i<stn_count;i++)
      {
	stn_init[i].nexradstn->FinishedDataAvail(finishedscan);
      }
    
    result = 1;
  }
  return result;
}

void NexRadMgr::SetDataMode(e_scan_client_mode newmode)	// try to set data mode
{
  scan_client::SetDataMode(newmode);
  for (int i=0;i<stn_count;i++)
    {
      stn_init[i].nexradstn->SetDataMode(newmode);
    }
    
}

// Don't add scan via both New and FInished methods
// If AcceptNewScans defined, don't use FinishedDataAvail
int  NexRadMgr::FinishedDataAvail(CData *finisheddata) {
  return 0;
}

void NexRadMgr::Init_NexRad()
{
  //read nexrad.ini file for  NexRad flag and NexRad parameters
  /*  Example nexrad.ini file:
      #rapic to nexrad Data Manager - parameter file

      #NexRadFlag turns NEXRAD server code on or off
      NexRadFlag=TRUE
      #NexRadFlag=FALSE

      #writeSimRapic causes rdrInvent to be called which writes out new RAPIC data file(s) with
      #different characteristic, depending on how rdrInvent.C has been modified.  Only used
      #for short term hack at this stage.  SD 11/4/05
      writeSimRapic=FALSE

      #completeVol true means to only process a volume after finished and the scan is complete
      #else if false process data as soon as availalbe
      completeVol=FALSE

      #if TRUE then filter out radar data more than parent->futureDataTmWin (=30min) seconds into future
      ignoreFutureData=TRUE

      LoadFromDB=TRUE
      #LoadFromDB=FALSE

      debug=TRUE
      #debug=FALSE

      #station number to select
      #Kurnell
      station=54 \
      \ 
      #output path. If set dumps packets to file.
      #Note final slash, necessary!
      path=/radar1/sandy/radar_387/radar/rapic/bin/ \
      \ 
      #UDP address
      #hargraves output for migfa
      udp_output=134.178.4.82:8800 \
      udp_output=134.178.4.82:8803 \
      #trump output for ridds2mom
      #udp_output=134.178.4.215:3280
      #trump output to udp_relay
      #udp_output=134.178.4.215:15533
      \
      #radar type selects type to convert
      radarType=VOL \
      #radarType=CompPPI
      \
      #packet size in bytes
      packet_size=2432 \
      \
      #volume scanning pattern
      pattern=145:scancount=34 \
      pattern=146:scancount=30 \
      \
      #VCP file location, usually $W2_CONFIG_LOCATION/vcp
      #if not defined, will not write VCP file
      #else will ignore the pattern= key and write a vcp file
      #and place a computed vcp number in the nexrad data.
      vcp_locn=/blah/blah/blah
      \
      #filtering switches
      filter_Vel=TRUE \
      filter_Refl=FALSE \
      \
      #delay between packets in milliseconds
      delay=4  \
      #min time between tilt in seconds
      mintimepertilt=5 \
      #if defined data written straight to final file name
      no_tempfile \
      #Write Archive2 header at start of file only
      archive2file 
      #End of station data
      #repeat station data below if nec.     
  */
  FILE *NexRad_ini;
  char line[NEX_LINELEN], templine[256], *index,address[100];
  char *cursor,*firstchar,*lastchar,keyvalue[256],value[256];
  int  ivalue,add1,add2,add3,add4;
  char path[256];
  char vcp_locn[256];
  
  char         radarTypeStr[20];
  int	       pattern;
  int          packet_size;
  int          delay_pac;
  int          mintimepertilt;
  int          station; 
  int 	       start_local_port;
  int          outputcount;
  e_scan_type  radarType;
  scan_pattern* scan_patt_arr;
  int           scan_patt_count;
  int           scancount;
  bool       filter_Vel    = FALSE;
  bool       filter_Refl   = FALSE;
  bool       filter_SpWdth = FALSE;
  bool       arch2header   = false;
  bool       noTempFile    = false;
  int        refl_rngres_reduce_factor = 1;
  int        vel_rngres_reduce_factor = 1;

  //defaults before init file read
  NexRadOutpDesc *outputdesc[MAXOUTPUT];
  NexRadFlag               = FALSE;
  writeSimRapic            = FALSE;
  completeVol              = FALSE;
  ignoreFutureData         = FALSE;
  LoadFromDB               = FALSE;
  debug                    = FALSE;
  station                  = -1;
  path[0]                  = '\0';  //empty path initially
  vcp_locn[0]              = '\0';  //empty vcp initially
  outputcount              = 1;  //0 is reserved for the file, if any
  outputdesc[0]            = new NexRadOutpDesc(); 
  delay_pac                = 0;  //4 millisec delay in packet send
  mintimepertilt	   = 0;	// 5 second minimum time per tilt
  pattern                  = 144;
  packet_size              = 2432;
  radarType		   = VOL;
  start_local_port	   = 0;
  

  stn_count=0;
  if (!(NexRad_ini = fopen("nexrad.ini","r")) )
    {
      //fprintf(stderr,"ERROR: NexRadMgr: Cannot open NexRad.ini\n");
      return;
    }
  fgets(line,256,NexRad_ini);
  while (!feof(NexRad_ini))
    {
      int cont=0;
      
      if ((index = strchr(line,'#'))) //ignore comments
	{
	  fgets(line,256,NexRad_ini);
	  continue;
	}
      lastchar = line;
      if (strlen(line))
	lastchar = line + strlen(line) - 1;	// last character
      while ((lastchar > line) && isspace(*lastchar))
	lastchar--;				// strip trailing whitespace
      if (*lastchar != '\\') cont = 0;
      else cont = 1;
      
      while (strlen(line) && cont)
	// is last non-white is \, continuation
	{
	  *lastchar = ' ';     // remove \, replace with blank and null
	  lastchar++;
	  *lastchar = '\0';
	  
	  fgets(templine,256,NexRad_ini);
	  while ((index = strchr(templine,'#'))) //ignore comments
	    fgets(templine,256,NexRad_ini);
	  if (strlen(line) + strlen(templine) > NEX_LINELEN)
	    {
	      fprintf(stderr,"NexRadMgr::Init_NexRad: ERROR Line too long - EXITING\n");
	      //	      exit(-1);
	      fclose(NexRad_ini);
	      return;
	    }

	  strcat(line,templine);
	  lastchar = line + strlen(line) - 1;	// last character
	  while ((lastchar > line) && isspace(*lastchar))
	    lastchar--;				// strip trailing
						// whitespace
	  if (*lastchar != '\\') cont = 0;
	  
	}
      //line is now one continuous string without \s
      //loop thru line looking for <key>=<value> pairs
      cursor = line;
      outputcount = 1;
      station = -1;
      //create new array for scan_pattern
      if (!(scan_patt_arr = (scan_pattern*) malloc(NEX_MAX_PATTS*sizeof(scan_pattern))))
	{
	  fprintf(stderr,"NexRadMgr::Init_NexRad: ERROR, cannot malloc scan_pattern, exitting\n");
	  //	exit(-1);
	  fclose(NexRad_ini);
	  return;
	}
      scan_patt_count=0;
      
      //goto nonblank
      for (cursor = line; isspace(*cursor) && *cursor != '\0';cursor++);
      firstchar = cursor;
      //goto next blank after string
      for (;!isspace(*cursor) && *cursor != '\0';cursor++);
      while (cursor-firstchar > 3)
	//not yet at eol, string in keyvalue
	{
	  strncpy(keyvalue,firstchar,cursor-firstchar);
	  keyvalue[cursor-firstchar] = '\0'; //null terminate
	  
	  if (sscanf(keyvalue,"station=%d",&station));
	  if (sscanf(keyvalue,"local_port=%d",&start_local_port));
	  if (sscanf(keyvalue,"delay=%d",&delay_pac));
	  if (sscanf(keyvalue,"mintimepertilt=%d",&mintimepertilt));
	  if (sscanf(keyvalue,"rngres_reduce_factor=%d",&refl_rngres_reduce_factor));
	  if (sscanf(keyvalue,"refl_rngres_reduce_factor=%d",&refl_rngres_reduce_factor));
	  if (sscanf(keyvalue,"vel_rngres_reduce_factor=%d",&vel_rngres_reduce_factor));
	  if (sscanf(keyvalue,"pattern=%d:scancount=%d",&pattern,&scancount))
	    {
	      scan_patt_arr[scan_patt_count].pattern = pattern;
	      scan_patt_arr[scan_patt_count].scan_count = scancount;
	      if (scan_patt_count++ >= NEX_MAX_PATTS)
		{
		  fprintf(stderr,"NexRadMgr::Init_NexRad: ERROR, scan_patt_arr exceeded size %d, exitting\n",
			  scan_patt_count);
		  //		  exit(-1);
		  fclose(NexRad_ini);
		  return;
		}
	    }
	  if (sscanf(keyvalue,"packet_size=%d",&packet_size));
	  if (sscanf(keyvalue,"NexRadFlag=%s",value))
	    {
	      if (!strcmp(value,"TRUE"))  NexRadFlag = TRUE;
	      else NexRadFlag = FALSE;
	    }
          if (sscanf(keyvalue,"ignoreFutureData=%s",value))
            {
              if (!strcmp(value,"TRUE"))  ignoreFutureData = TRUE;
              else ignoreFutureData = FALSE;
            }


          if (sscanf(keyvalue,"writeSimRapic=%s",value))
            {
              if (!strcmp(value,"TRUE"))  writeSimRapic = TRUE;
              else writeSimRapic = FALSE;
            }
          if (sscanf(keyvalue,"completeVol=%s",value))
            {
              if (!strcmp(value,"TRUE"))  completeVol = TRUE;
              else completeVol = FALSE;
            }
	  if (sscanf(keyvalue,"LoadFromDB=%s",value))
	    {
	      if (!strcmp(value,"TRUE"))  LoadFromDB = TRUE;
	      else LoadFromDB = FALSE;
	    }
	  if (sscanf(keyvalue,"debug=%s",value))
	    {
	      if (!strcmp(value,"TRUE"))  debug = TRUE;
	      else debug = FALSE;
	    }
	  if (sscanf(keyvalue,"filter_Vel=%s",value))
	    {
	      if (!strcmp(value,"TRUE")) filter_Vel  = TRUE;
	      else filter_Vel = FALSE;
	    }
	  if (sscanf(keyvalue,"filter_Refl=%s",value))
	    {
	      if (!strcmp(value,"TRUE")) filter_Refl  = TRUE;
	      else filter_Refl = FALSE;
	    }
	  if (sscanf(keyvalue,"filter_SpWdth=%s",value))
	    {
	      if (!strcmp(value,"TRUE")) filter_SpWdth  = TRUE;
	      else filter_SpWdth = FALSE;
	    }
	  if (sscanf(keyvalue,"path=%s",value))
	    {
	      strcpy(path,value);
	      outputdesc[0]->fs_flag = NEXRAD_FILE;
	    }
	  if (sscanf(keyvalue,"vcp_locn=%s",value))
	    {
	      strcpy(vcp_locn,value);
	    }
	  if (strstr(keyvalue,"archive2file"))
	    {
	      outputdesc[0]->archive2file = true;
	    }
	  if (strstr(keyvalue,"no_tempfile"))
	    {
	      noTempFile = true;
	    }
	  if (sscanf(keyvalue,"udp_output=%d.%d.%d.%d:%d",
		     &add1,&add2,&add3,&add4,&ivalue))
	    {
	      //create new entry in NexRadOutpDesc array
	      sprintf(address,"%d.%d.%d.%d",add1,add2,add3,add4);
	      outputdesc[outputcount++] = new NexRadOutpDesc(address,ivalue);
	      if (outputcount > MAXOUTPUT)
		{
		  fprintf(stderr,"NexRadMgr::Init_NexRad: ERROR"
			  " Too many output sockets\n");
		  //		  exit(-1);
		  fclose(NexRad_ini);
		  return;
		}
	      //debug
	      printf("NexRadMgr::Init_NexRad: udp_output=%s:%d\n",
		     address,ivalue);
	    }
	  if (sscanf(keyvalue,"radarType=%s",value))
	    {
	      strcpy(radarTypeStr,value);
	      if (!strcmp(value,"VOL"))  radarType = VOL;
	      else if  (!strcmp(value,"CompPPI"))  radarType = CompPPI;
	      else     
		{
		  fprintf(stderr,"NexRadMgr::Init_NexRad: ERROR "
			  "Wrong radar type in nexrad.ini\n"
			  "  Must be one of: VOL, CompPPI.  Got %s\n",value );
		  //		  exit(-1);
		  fclose(NexRad_ini);
		  return;
		}
	    }
	  
	  //skip to nonblank
	  for (; isspace(*cursor) && *cursor != '\0';cursor++);
	  firstchar = cursor;
	  //goto next blank after string
	  for (;!isspace(*cursor) && *cursor != '\0';cursor++);
	}
      //end of line processing, may have been a station line or simple line
      if (station >= 0)
	{
	  //station line,  move station data over to array
	  strcpy(stn_init[stn_count].path,path);
	  strcpy(stn_init[stn_count].vcp_locn,vcp_locn);

	  //debug SD 3/10/05
	  //fprintf(stderr,"NexRadMgr::Init_NexRad:DEBUG stn_init[stn_count].vcp_locn = %s\n",stn_init[stn_count].vcp_locn);
	  
	  stn_init[stn_count].station     = station;
	  stn_init[stn_count].radarType   = radarType;
	  stn_init[stn_count].packet_size = packet_size;
	  stn_init[stn_count].scan_patt_arr   = scan_patt_arr; //SD add 22/8/00
	  stn_init[stn_count].scan_patt_count   = scan_patt_count;
	  
	  stn_init[stn_count].delay       = delay_pac;
	  stn_init[stn_count].min_time_per_tilt = mintimepertilt;
	  stn_init[stn_count].start_local_port = start_local_port;
	  stn_init[stn_count].filter_Vel        = filter_Vel;
	  stn_init[stn_count].filter_Refl       = filter_Refl;
	  stn_init[stn_count].filter_SpWdth     = filter_SpWdth;
	  stn_init[stn_count].refl_rngres_reduce_factor= refl_rngres_reduce_factor;
	  stn_init[stn_count].vel_rngres_reduce_factor= vel_rngres_reduce_factor;
	  stn_init[stn_count].noTempFile        = noTempFile;
	  //copy over outputdesc array
	  stn_init[stn_count].outputcount = outputcount;
	  for (int i = 0;i<outputcount;i++)
	    stn_init[stn_count].outputdesc[i] = outputdesc[i];
	  if (++stn_count >= NEX_MAX_STATIONS)
	    {
	      fprintf(stderr,"NexRadMgr::Init_NexRad: ERROR "
		      "Too many stations in init file, ignoring %s\n", line);
	      fclose(NexRad_ini);
	      return;
	    }
	  //reset local values 
	  station            = -1;
	  start_local_port   = 0;
	  strcpy(path,"");
	  strcpy(vcp_locn,"");
	  radarType          = VOL;
	  packet_size        = 2432;
	  pattern            = 144;
	  delay_pac          = 4;
	  outputdesc[0]      = new NexRadOutpDesc(); 
	  outputcount        = 1;
	  filter_Vel         = FALSE;
	  filter_Refl        = FALSE;
	  filter_SpWdth      = FALSE;
	  refl_rngres_reduce_factor= 1;
	  vel_rngres_reduce_factor= 1;
	  arch2header        = false;
	  noTempFile         = false;
	}
      
      fgets(line,256,NexRad_ini);
    }
  fclose(NexRad_ini);
  //last outputdesc[0] = NexRadOutpDesc is not used, so delete it
  delete outputdesc[0];  //didnt like this, seg fault
  
  //check no repeated stations in init file
  for (int y=1;y<stn_count;y++)
    for (int k=0;k<y;k++)
      {
	if ( stn_init[y].station == stn_init[k].station)
	  {
	    fprintf(stderr,"NexRadMgr::Init_NexRad: ERROR Duplicated stations %d\n"\
		    ,stn_init[k].station );
	    //	    exit(-1);
	    return;
	  }
      }


  
}


