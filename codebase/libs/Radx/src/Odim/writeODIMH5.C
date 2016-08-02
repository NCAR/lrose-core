/*
 * writeODIMH5 implementation
 */

#include <iostream>
#include <signal.h>
#include "writeODIMH5.h"
#include <errno.h>
#include <string.h>
#include "utils.h"
#include <unistd.h>
#include "linkedlisttemplate.h"
#include <stdio.h>
#include <values.h>
#include <siteinfo.h>

static char const cvsid[] = "$Id: writeODIMH5.C,v 1.1 2014/08/18 14:32:17 dixon Exp $";

writeODIMH5 *ODIMH5Writer = NULL;


writeODIMH5::writeODIMH5() : scan_client()
{
  //read init file
  odimh5Suffix = ".h5";
  rapicODIMH5FileWriter = NULL;
  Init_ODIMH5();

  Thread_pid = Thread_ppid = 0;

  //varbs for threadobj type start
  thread_id = parent_id = 0;
  thread_pid = parent_pid = 0;
  quiet = false;
  strcpy(threadName, "writeODIMH5");
  
  ClientType = SC_WRITEODIMH5;
  lock = new spinlock("writeODIMH5::ctor",3000);	// 30 secs  
  ODIMH5DataListLock = new spinlock("writeODIMH5ListLock",3000); // 30 secs
  StopThreadFlag = 0;
  AcceptNewScans = FALSE;
  AcceptFinishedScans = TRUE;
  newscans_last = 0;
  checkedscans_last = 0;
  
  printf("writeODIMH5::start\n");  
   
  if (ScanMng) 
    ScanMng->AddClient(this);
}	

writeODIMH5::~writeODIMH5() 
{
  fprintf(stderr, "writeODIMH5::~writeODIMH5 Stopping thread....\n");
  stopThread();
  if (!Thread_pid)
    fprintf(stderr, "writeODIMH5::~writeODIMH5 Stopping thread....OK\n");
  else 
    fprintf(stderr, "writeODIMH5::~writeODIMH5 Stopping thread....FAILED\n");

  if (lock) 
    {
      lock->rel_lock();
      delete lock;        
    }
  if (ODIMH5DataListLock) 
    {
      ODIMH5DataListLock->rel_lock();
      delete ODIMH5DataListLock;        
    }
  if (rapicODIMH5FileWriter)
    delete rapicODIMH5FileWriter;
}

void writeODIMH5::GetListLock(char *lockholder)
{
  ODIMH5DataListLock->get_lock(lockholder);
}

void writeODIMH5::RelListLock()
{
  ODIMH5DataListLock->rel_lock();
}

void writeODIMH5::readInitFile(char *initfilename)
{
  Init_ODIMH5(initfilename);
}

void writeODIMH5::runLoop() {

    nice(15);   //back off scheduler priority a bit
    
    while (!StopThreadFlag) {
      CheckNewScans();
      ProcessCheckedScans();
      if (kill(Thread_ppid, 0) < 0) {
	fprintf(stderr, "\nwriteODIMH5::runLoop: Parent process not responding,  exiting\n");
	StopThreadFlag = 1;	    // if parent dies
      }
      if (!StopThreadFlag) sec_delay(1.0);	// wait 1 sec, then loop
    }
    fprintf(stderr, "writeODIMH5::runLoop: pid=%ld, EXITING\n", Thread_pid);
    Thread_pid = 0;
}


int  writeODIMH5::NewDataAvail(rdr_scan *newscan) {
  //move new radar scan onto linked list of current volumes to consider
  rdr_scan_node   *newscannode;	
  int result = 0;
  int lock_ok = 0;

  if (lock && !(lock_ok = lock->get_lock("writeODIMH5::NewDataAvail")))
    fprintf(stderr,"writeODIMH5::NewDataAvail: get_lock FAILED\n");	
  if (newscan && AcceptNewScans) {
    printf("writeODIMH5::NewDataAvail: adding scan %02d/%d/%d %02d:%02d:%02d\n",
	   newscan->year,newscan->month,newscan->day,newscan->hour,
	   newscan->min,newscan->sec);
    newscannode = new rdr_scan_node(this,"writeODIMH5",newscan);
    //_scan_node_count++;  //add _ SD 25jun05
    incScanNodeCount();
    
    //newscannode->PreLink(newscans); //newscans defined in rdrscan.h
    newscannode->prev = 0;		  
    newscannode->next = newscans;
    if (newscans) newscans->prev = newscannode;  //SD add prev links so dbly linked list
    if (!newscans) newscans_last = newscannode;  //if empty set last ptr to single node
    newscans = newscannode;
    if (!newscans_last) newscans_last = newscannode;
    result = 1;
  }
  if (lock_ok) 
    lock->rel_lock();
  return result;
}

int  writeODIMH5::NewDataAvail(CData *newdata) 
{
  //not interested in Cdata
    return 0;
}

// Don't add scan via both New and FInished methods
// If AcceptNewScans defined, don't use FinishedDataAvail
int  writeODIMH5::FinishedDataAvail(rdr_scan *finishedscan) 
{
  rdr_scan_node   *newscannode;	
  int	    result = 0;
  int		lock_ok = 0;

  if (lock && !(lock_ok = lock->get_lock("writeODIMH5::FinishedDataAvail")))
    fprintf(stderr,"writeODIMH5::FinishedDataAvail: get_lock FAILED\n");	
  if (finishedscan && !AcceptNewScans && AcceptFinishedScans &&
      (finishedscan->dataValid() && 
	  ODIMH5stnSet.isSet(finishedscan->station) &&
// 	  rootscan->station == station &&
// 	  rootscan->scan_type == radarType) 
       isVolType(finishedscan->scan_type))) {
    if (debug) printf(
	   "writeODIMH5::FinishedDataAvail: adding scan %02d/%d/%d %02d:%02d:%02d\n",
	   finishedscan->year,finishedscan->month,finishedscan->day,
	   finishedscan->hour,finishedscan->min,finishedscan->sec);
    newscannode = new rdr_scan_node(this,"writeODIMH5",finishedscan);
    newscannode->prev = 0;
    newscannode->next = newscans;
    if (newscans) newscans->prev = newscannode;  //SD add prev links so dbly linked list
    if (!newscans) newscans_last = newscannode;  //if empty set last ptr to single node
    newscans = newscannode;
    if (!newscans_last) newscans_last = newscannode;
    incScanNodeCount();
    result = 1;
  }
  if (lock_ok) 
    lock->rel_lock();
  return result;
}


// Don't add scan via both New and FInished methods
// If AcceptNewScans defined, don't use FinishedDataAvail
int  writeODIMH5::FinishedDataAvail(CData *finisheddata) {
    return 0;
}


/*
 * IMPORTANT NOTE: AS THE NEWSCANS QUEUE IS OFTEN ACCESSED BY SEPARATE
 * THREADS THE NEWDataAvail,  FINISHEDDataAvail AND CHECKNEWSCANS
 * SHOULD ALL BE "THREAD SAFE" I.E. SERIALIZED BY MUTEXES
 * PROCESSCHECKEDSCANS DOES NOT NEED TO BE THREAD SAFE
 */

/*  How the queues (linked lists) newscans and checkedscans are used:
 *
 *   new scan --> o newscans          -----------> o checkedscans     
 *                |                  /             |
 *             ^  |                 /              |
 *        prev |  |                /               |
 *   links     -  |               /                |           -----> ODIMH5
 *        next |  |              /                 |          /
 *             v  |             /                  |         /
 *                |            /                   |        /
 *  newscans_last o >---------/  checkedscans_last o >-----/ 
 * 
 */

void  writeODIMH5::CheckNewScans(bool MoveToChecked, 
				 bool RejectFaulty) {
  //Keep checking the newscans list until a member satisfies the criteria,
  //then move it over to checkedscans list.  Otherwise delete it if it fails
  //some criteria,  or keep it on list for next time
  //move over to checkedscans in chronological order,  last to new

  rdr_scan_node *tempnode, *rootnode;	
  rdr_scan	*rootscan;
//   rdr_scan	*reflscan, *velscan;

  int		lock_ok = 0;
    
  if (!newscans_last)
    return;
  if (lock && !(lock_ok = lock->get_lock("writeODIMH5::CheckNewScans")))
    fprintf(stderr,"writeODIMH5::CheckNewScans: get_lock FAILED\n");	
	
  rootnode = newscans_last;
  //loop thru newscans_last,  oldest to newest
  while (newscans_last) 
    {
      //loop  thru current volumes, deleting those irrelevent, moving to 
      //checkedscans those satisfying certain criteria,  and leaving 
      //those not yet determined
      tempnode = (rdr_scan_node *)newscans_last->prev;
      rootscan = (rdr_scan *)newscans_last->scan->rootScan();

      if (rootscan->dataValid() && 
	  ODIMH5stnSet.isSet(rootscan->station) &&
// 	  rootscan->station == station &&
// 	  rootscan->scan_type == radarType) 
	  isVolType(rootscan->scan_type)) 
	{
	  if (debug) printf(
			    "writeODIMH5::CheckNewScans: moving scan %02d/%d/%d %02d:%02d:%02d\n",
			    newscans_last->scan->year,newscans_last->scan->month,
			    newscans_last->scan->day, newscans_last->scan->hour,
			    newscans_last->scan->min,newscans_last->scan->sec);
	  if (newscans_last == rootnode) rootnode = newscans_last->prev;
	  if(newscans_last->prev) newscans_last->prev->next = newscans_last->next;
	  if(newscans_last->next) newscans_last->next->prev = newscans_last->prev;
		
	  //put it into new end of checkedscans
	  newscans_last->prev = 0;
	  newscans_last->next = checkedscans;
	  if (checkedscans) checkedscans->prev = newscans_last;
	  if (!checkedscans) checkedscans_last = newscans_last;
	  checkedscans = newscans_last;
	  //_scan_node_count--;  //add _ SD 25jun05
	  decScanNodeCount();
		
	}
      else if (rootscan->Faulty() ||
	       (rootscan->HeaderValid() && //dont get stn or time if no header
		(!ODIMH5stnSet.isSet(rootscan->station) ||
		 !isVolType(rootscan->scan_type))))
// 		 (rootscan->station != station || //delete if wrong station 
// 		rootscan->scan_type != radarType ))) 
	{ //excise link
	  if (debug) printf("writeODIMH5::CheckNewScans: deleting scan "
			    "%02d/%d/%d %02d:%02d:%02d, radarType = %d,"
			    " stn = %d\n",
			    newscans_last->scan->year,
			    newscans_last->scan->month,
			    newscans_last->scan->day,newscans_last->scan->hour,
			    newscans_last->scan->min,newscans_last->scan->sec,
			    rootscan->scan_type,rootscan->station);
	  if (newscans_last == rootnode) rootnode = newscans_last->prev;
	  if(newscans_last->next) 
	    newscans_last->next->prev = newscans_last->prev;
	  if(newscans_last->prev) 
	    newscans_last->prev->next = newscans_last->next;
	  delete newscans_last;
	  decScanNodeCount();
		
	}

      newscans_last = tempnode;
    }
  //restore list root
  newscans_last = rootnode; 
  if (!newscans_last) newscans = 0; //if queue empty,  make both end ptrs consistent
	
  if (lock_ok)
    lock->rel_lock();
}

void  writeODIMH5::ProcessCheckedScans(int maxscans) 
{
  //NOTE:  new scans go on one end of LL,  need to read them off the other end
  // so its a queue, not  a stack!!
  rdr_scan_node   *tempscannode;	
  rdr_scan	  *tempscan;
  char            scanstring[256];

  if (debug) 
    {
      printf(".");
      fflush(stdout);
    }
  
  //process members of checkedscans,  send to ODIMH5!
  while (checkedscans_last)
    {
      tempscannode = (rdr_scan_node *)checkedscans_last->prev;
      tempscan = (rdr_scan *)checkedscans_last->scan->rootScan();
      if (debug) 
	printf("writeODIMH5::ProcessCheckedScans: scan %02d/%d/%d"
	       "%02d:%02d:%02d\n",
	       tempscan->year,tempscan->month,tempscan->day,tempscan->hour,
	       tempscan->min,tempscan->sec);

      //process end of volume, write ODIMH5
      string hdfname(path);
      hdfname += tempscan->scanFilename(scanstring);
      if (rapicODIMH5FileWriter->writeFloat)
	hdfname += "_f";
      hdfname += odimh5Suffix;
      rapicODIMH5FileWriter->writeFile((char *)hdfname.c_str(), tempscan);
      
      //remove old checkedscans.
      if(checkedscans_last->prev) checkedscans_last->prev->next = 0;      
      delete checkedscans_last;
      checkedscans_last = tempscannode;
    } 
  checkedscans = NULL;  //this queue must be empty here
  
}

void writeODIMH5::Init_ODIMH5(char *initfilename)
{
  //read ODIMH5.ini file for station id, ODIMH5 flag and ODIMH5 parameters
  /*  Example ODIMH5.ini file:
        #rapic to ODIMH5 Data Manager - parameter file
        
        ODIMH5Flag=TRUE
        #station number is station to send to Migfa
        station=54
        debug=FALSE
	radarType=VOL      
	#radarType=CompPPI      
	#output path
	path=/radar1/sandy/radar_385/radar/bin/rapic/
  */
  
  FILE *ODIMH5_ini;
  char line[256], value[100];
  char *tempstrptr;
  debug = FALSE;
  strcpy(path,"");
  string odimIniFName("ODIMH5.ini");
  
  if (initfilename && strlen(initfilename))
    odimIniFName = initfilename;
  if (!(ODIMH5_ini = fopen(odimIniFName.c_str(),"r")) )
    {
      char strbuff[256];
      fprintf(stderr,"ERROR: writeODIMH5: Cannot open %s - %s\n",
	      odimIniFName.c_str(), strerror_r(errno, strbuff, 256));
      return;
    }
  if (!rapicODIMH5FileWriter)
    rapicODIMH5FileWriter = new odimH5FileWriter();
  fgets(line,256,ODIMH5_ini);
  while (!feof(ODIMH5_ini))
    {
      if (line[0] == '#') //ignore comments
	{
	  fgets(line,256,ODIMH5_ini);
	  continue;
	}
      if ((tempstrptr = strstr(line, "stations=")))
	  {
	    tempstrptr += strlen("stations=");
	    ODIMH5stnSet.set(tempstrptr, true);
	  }
      if (sscanf(line,"debug=%s",value))
	{
	  if (!strcmp(value,"TRUE"))  debug = TRUE;
	  else debug = FALSE;
	}
      if (sscanf(line,"path=%s",value))
	{
	  strcpy(path,value);
	}
      if ((tempstrptr = strstr(line, "suffix=")))
	{
	  tempstrptr += strlen("suffix=");
	  odimh5Suffix = tempstrptr;
	}
      if (strstr(line, "writeFloatData"))
	rapicODIMH5FileWriter->writeFloat = true;
      if ((tempstrptr = strstr(line, "compressFactor=")))
	{
	  int compress;
	  if (rapicODIMH5FileWriter &&
	      (sscanf(tempstrptr, "compressFactor=%d", &compress)
	       == 1))
	    rapicODIMH5FileWriter->compressFactor = compress;
	}
      if ((tempstrptr = strstr(line, "compressChunkSize=")))
	{
	  int cdim0, cdim1;
	  if (sscanf(tempstrptr, "compressChunkSize=%d,%d", &cdim0, &cdim1)
	      == 2)
	    {
	      rapicODIMH5FileWriter->compressChunkSize[0] = cdim0;
	      rapicODIMH5FileWriter->compressChunkSize[1] = cdim1;
	    }
	}
      
//       if (sscanf(line,"radarType=%s",value))
// 	{
// 	  strcpy(radarTypeStr,value);
// 	  if (!strcmp(value,"VOL"))  radarType = VOL;
// 	  else if  (!strcmp(value,"CompPPI"))  radarType = CompPPI;
// 	  else     
// 	    {
// 	      fprintf(stderr,"ERROR: writeODIMH5::Init_ODIMH5: Wrong radar type in ODIMH5.ini\n" 
// 		      "        Must be one of: VOL, CompPPI.  Got %s\n",value );
// 	      exit(-1);
// 	    }
// 	}
      fgets(line,256,ODIMH5_ini);
    }
  fclose(ODIMH5_ini);
}

