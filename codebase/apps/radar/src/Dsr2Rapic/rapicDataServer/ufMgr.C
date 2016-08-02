/*
 * ufMgr implementation
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
#include "ufMgr.h"
#include "ufStnHdlr.h"
#include <time.h>
static char const cvsid[] = "$Id: ufMgr.C,v 1.2 2008/02/29 04:44:54 dixon Exp $";

ufMgr *ufManager = 0;

//send output from several radar stations to file for each,  as controlled by uf.ini
//(analogous to NexRad)

ufMgr::ufMgr() : scan_client()
{
  //read init file uf.ini
  Init_uf();
  AcceptNewScans = FALSE;      //was SD 5/1/4
  AcceptFinishedScans = TRUE;  //was SD 5/1/4
  AllowReplayMode = true;  // allow replay mode
  AllowDBReviewMode = true;  // allow DBReview mode
    if (!ufFlag) return;

  ClientType               = SC_UFMNG;
  lock                     = new spinlock("ufMgr->lock", 3000);    // 30 secs
  printf("ufMgr::start\n");  
  if (ScanMng && ufFlag) ScanMng->AddClient(this);
}	

ufMgr::~ufMgr() 
{
  //  int waittime = 30;
  
  if (ScanMng && ufFlag) ScanMng->RemClient(this);
  fprintf(stderr, "ufMgr::~ufMgr Stopping ufStnHdlrs....\n");
    
  //stop each ufStnHdlr, go thru linked list of instances
  for (int i=0;i<stn_count;i++)
    stn_init[i].ufstn->stopThread();
  if (lock) 
    {
      lock->rel_lock();
      delete lock;
      lock = 0;
    }
}

void ufMgr::StartUfStnHdlrs() 
{
  if (!ufFlag) return;
  //start each ufStnHdlr in turn, as defined by uf.ini
  for (int i=0;i<stn_count;i++)
    {
      stn_init[i].ufstn = new ufStnHdlr( stn_init[i].station,
					 stn_init[i].path,
					 stn_init[i].radarType,
					 debug,
					 ufFlag);
    }
}

int  ufMgr::NewDataAvail(rdr_scan *newscan) 
{
  if (!ufFlag) return 1;
  if (newscan && AcceptNewScans && ScanSourceOK(newscan)) 
    {
      for (int i=0;i<stn_count;i++)
	stn_init[i].ufstn->NewDataAvail(newscan);
    }
  return 1;
}


int  ufMgr::NewDataAvail(CData *newdata) 
{
  //not interested in Cdata
  return 0;
}

// Don't add scan via both New and FInished methods
// If AcceptNewScans defined, don't use FinishedDataAvail
int  ufMgr::FinishedDataAvail(rdr_scan *finishedscan) 
{
  //pass this call onto each ufStnHdlr in turn
  //  rdr_scan_node   *newscannode;	
  int	    result = 0;
  if (!ufFlag) return 1;
  if (finishedscan && !AcceptNewScans && AcceptFinishedScans &&  
      ScanSourceOK(finishedscan)) {
     for (int i=0;i<stn_count;i++)
      {
	stn_init[i].ufstn->FinishedDataAvail(finishedscan);
      }
    
    result = 1;
  }
  return result;
}

void ufMgr::SetDataMode(e_scan_client_mode newmode)	// try to set data mode
{
  scan_client::SetDataMode(newmode);
  for (int i=0;i<stn_count;i++)
    {
      stn_init[i].ufstn->SetDataMode(newmode);
    }
    
}

// Don't add scan via both New and FInished methods
// If AcceptNewScans defined, don't use FinishedDataAvail
int  ufMgr::FinishedDataAvail(CData *finisheddata) {
  return 0;
}

void ufMgr::Init_uf()
{
  //read uf.ini file for  uf flag and uf parameters
  /*  Example uf.ini file:
      #rapic to uf Data Manager - parameter file

      #ufFlag turns UF server code on or off
      ufFlag=TRUE
      #ufFlag=FALSE

      debug=TRUE
      #debug=FALSE

      #station number to select
      #Kurnell
      station=54 \
      \ 
      #output path. If not set writes to $PWD
      path=/radar1/sandy/radar_387/radar/rapic/bin/ \
      \ 
      #radar type selects type to convert
      radarType=VOL 
      #radarType=CompPPI
      #End of station data
      #repeat station data below if nec.     
  */
  FILE *uf_ini;
  char line[UF_LINELEN], templine[256], *index;
  char *cursor,*firstchar,*lastchar,keyvalue[256],value[256];
  char path[256];
  char         radarTypeStr[20];
  int          station; 
  e_scan_type  radarType;

  //defaults before init file read
  ufFlag               = FALSE;
  debug                    = FALSE;
  station                  = -1;
  strcpy(path,".");	//initially path set to pwd directory
  //path[0]                  = '\0';  //empty path initially
  radarType		   = VOL;
  

  stn_count=0;
  if (!(uf_ini = fopen("uf.ini","r")) )
    {
      //fprintf(stderr,"ERROR: ufMgr: Cannot open uf.ini\n");
      return;
    }
  fgets(line,256,uf_ini);
  while (!feof(uf_ini))
    {
      int cont=0;
      
      if ((index = strchr(line,'#'))) //ignore comments
	{
	  fgets(line,256,uf_ini);
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
	  
	  fgets(templine,256,uf_ini);
	  while ((index = strchr(templine,'#'))) //ignore comments
	    fgets(templine,256,uf_ini);
	  if (strlen(line) + strlen(templine) > UF_LINELEN)
	    {
	      fprintf(stderr,"ufMgr::Init_uf: ERROR Line too long - EXITING\n");
	      //	      exit(-1);
	      fclose(uf_ini);
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
      station = -1;
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
	  if (sscanf(keyvalue,"ufFlag=%s",value))
	    {
	      if (!strcmp(value,"TRUE"))  ufFlag = TRUE;
	      else ufFlag = FALSE;
	    }
	  if (sscanf(keyvalue,"debug=%s",value))
	    {
	      if (!strcmp(value,"TRUE"))  debug = TRUE;
	      else debug = FALSE;
	    }
	  if (sscanf(keyvalue,"path=%s",value))
	    {
	      strcpy(path,value);
	    }
	  if (sscanf(keyvalue,"radarType=%s",value))
	    {
	      strcpy(radarTypeStr,value);
	      if (!strcmp(value,"VOL"))  radarType = VOL;
	      else if  (!strcmp(value,"CompPPI"))  radarType = CompPPI;
	      else     
		{
		  fprintf(stderr,"ufMgr::Init_uf: ERROR "
			  "Wrong radar type in uf.ini\n"
			  "  Must be one of: VOL, CompPPI.  Got %s\n",value );
		  //		  exit(-1);
		  fclose(uf_ini);
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
	  stn_init[stn_count].station     = station;
	  stn_init[stn_count].radarType   = radarType;
	  if (++stn_count >= UF_MAX_STATIONS)
	    {
	      fprintf(stderr,"ufMgr::Init_uf: ERROR "
		      "Too many stations in init file, ignoring %s\n", line);
	      fclose(uf_ini);
	      return;
	    }
	  //reset local values 
	  station            = -1;
	  strcpy(path,".");
	  radarType          = VOL;
	}
      
      fgets(line,256,uf_ini);
    }
  fclose(uf_ini);
  
  //check no repeated stations in init file
  for (int y=1;y<stn_count;y++)
    for (int k=0;k<y;k++)
      {
	if ( stn_init[y].station == stn_init[k].station)
	  {
	    fprintf(stderr,"ufMgr::Init_uf: ERROR Duplicated stations %d\n"\
		    ,stn_init[k].station );
	    //	    exit(-1);
	    return;
	  }
      }
}

