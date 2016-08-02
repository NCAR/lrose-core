/*
 * NexRadStnHdlr implementation
 */

#ifdef sgi
#define _SGI_MP_SOURCE
#include <sys/bsd_types.h>
#include "bool.h"
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
//#include "uiftovrc.h"
#include <libgen.h>
#include <unistd.h>
#include "linkedlisttemplate.h"
#include <stdio.h>
//#include <baseData.h>
#include <values.h>
#include <siteinfo.h>
//#include <dobson.h>
#include <rdrscan.h>
#include "NexRadMgr.h"
#include <time.h>
//SD 14/2/05
#include "rdrInvent.h"

static char const cvsid[] = "$Id: NexRadStnHdlr.C,v 1.2 2008/02/29 04:44:54 dixon Exp $";


NexRadStnHdlr::NexRadStnHdlr(int station,
			       char *path,
			       NexRadOutpDesc **outputdesc,
			       int outputcount,
			       e_scan_type radarType,
			       int packet_size,
			       scan_pattern* scan_patt_arr,
			       int scan_patt_count,
			       char *vcp_locn,
			       int delay,
			       int mintimepertilt, 
			       int start_local_port,
			       bool debug,
			       bool LoadFromDB,
			       bool NexRadFlag,
			       bool writeSimRapic,
			       bool completeVol,
			       bool ignoreFutureData,
			       bool noTempFile,
			       bool filter_Vel,
			       bool filter_Refl,
			       bool filter_SpWdth,
			       int refl_Rngres_reduce_factor,
			       int vel_Rngres_reduce_factor) : scan_client()
{
  //debug, print params
  if (debug) printf("NexRadStnHdlr::NexRadStnHdlr contructor:\n"
	 " 	station %d\n"
	 "	path %s\n"
	 "	outputcount %d\n"
	 "	radarType %d\n"
	 "	packet_size %d\n"
	 "	pattern count%d\n"
	 "	vcp location %s\n"
	 "	delay %d\n"
	 "	mintimepertilt %d\n"
	 "	start_local_port %d\n"
	 "	debug %d\n"
	 "	filt vel %d\n"
	 "	filt refl %d\n"
	 "	filt spwdth %d\n"
	 "	refl_rngres_reduce_factor %d\n"
	 "	vel_rngres_reduce_factor %d\n"
	 "	LoadFromDB %d\n"
	 "	NexRadFlag %d\n"
	 "	writeSimRapic %d\n"
	 "	completeVol %d\n"
	 "	ignoreFutureData %d\n"
	 "	noTempFlag %d\n",
	  	station,
	        path,
	 	outputcount,
	 	radarType,
	 	packet_size,
	 	scan_patt_count,
	        vcp_locn,
	 	delay,
		mintimepertilt, 
		start_local_port,
	 	debug,
	 	filter_Vel,
	 	filter_Refl,
	 	filter_SpWdth,
		refl_Rngres_reduce_factor,
		vel_Rngres_reduce_factor,
	 	LoadFromDB,
		NexRadFlag,
		writeSimRapic,
		completeVol,
		ignoreFutureData,
		noTempFile);
  this->NexRadFlag               = NexRadFlag;
  this->writeSimRapic            = writeSimRapic;
  this->completeVol              = completeVol;
  this->ignoreFutureData         = ignoreFutureData;
  this->LoadFromDB               = LoadFromDB;
  this->station                  = station;
  strcpy(this->path,path);
  this->outputcount              = outputcount;  
  //this->outputdesc               = outputdesc;
  for (int i=0;i<outputcount;i++)
    this->outputdesc[i] = outputdesc[i];
  this->radarType                = radarType;
  this->delay_pac                = delay;  //10 millisec delay in packet send
  min_time_per_tilt		 = mintimepertilt;
  this->debug                    = debug;
  this->filter_Vel               = filter_Vel;
  this->filter_Refl              = filter_Refl;
  this->filter_SpWdth            = filter_SpWdth;
  this->start_local_port         = start_local_port;
  //this->pattern                  = pattern;  SD alt 22/8/00
  this->scan_patt_arr            = scan_patt_arr;
  this->scan_patt_count          = scan_patt_count;
  strcpy(this->vcp_locn,vcp_locn);
  this->noTempFile               = noTempFile;
  
  this->packet_size              = packet_size;
  seq_num                  = 0;
  fr_seq                   = 0;
  refl_rngres_reduce_factor     = refl_Rngres_reduce_factor;
  vel_rngres_reduce_factor     = vel_Rngres_reduce_factor;
  
  ClientType               = SC_NEXRADSTNHDLR;
  lock                     = new spinlock("NexRadStnHndlr->lock", 3000); // 30secs
  NexRadDataListLock       = new spinlock("NexRadStnHndlr->NexRadDataListLock", 3000); // 30secs
  if (completeVol) {             //SD 1may07
     AcceptNewScans        = FALSE;
     AcceptFinishedScans   = TRUE;
  }
  else {
     AcceptNewScans        = TRUE;
     AcceptFinishedScans   = FALSE;
  }
  
  AcceptDuplicates         = FALSE;
  AllowReplayMode          = true;  // allow replay mode
  AllowDBReviewMode        = true;  // allow DBReview mode
  newscans_last            = 0;
  checkedscans_last        = 0;
  //local_port               = NEXRAD_START_PORT; Now a param SD 29/10/99
  strcpy(threadName, "NexRadStnHdlr");
  printf("NexRadStnHdlr::start\n");  
  workProcTimeout = 600;    // default is 10 minutes
  if (!NexRadFlag) return;

  //check vcp_locn exists
  if (!DirExists(vcp_locn)) {
    //cannot find VCP file location
    fprintf(stderr, "NexRadStnHdlr ERROR, cannot find VCP directory %s, ignoring it. \n",vcp_locn);
    strcpy(this->vcp_locn,"");
  }
  
  

  //SD addition for rdrInvent.C  14/2/5
  if (writeSimRapic) {
    char *filename = new char[1024];
    sprintf(filename,"rapic_%d_%d",station,(int)time(0));
    s1fd = open(filename, O_RDWR+O_CREAT, 0664);
    if (s1fd < 0) {
      fprintf(stderr, "NexRadStnHdlr FAILED opening %s \n",filename);
      perror(0);
    }
    else fprintf(stdout, "NexRadStnHdlr opening %s for simulated rapic file\n",filename);
  }

  startThread();  
}	

NexRadStnHdlr::~NexRadStnHdlr() 
{
  //  int waittime = 30;
  
  //  fprintf(stderr, "NexRadStnHdlr::~NexRadStnHdlr Stopping thread....Thread_id = %d\n", thread_id);
  stopThread();
  
  //  rdr_scan_node	*nextnode;
  if (lock) 
    {
      lock->rel_lock();
      delete lock;        
    }
  if (NexRadDataListLock) 
    {
      NexRadDataListLock->rel_lock();
      delete NexRadDataListLock;        
    }

  //SD addition for rdrInvent.C  14/2/5
  if (writeSimRapic) close(s1fd);
}

void NexRadStnHdlr::GetListLock()
{
    NexRadDataListLock->get_lock();
}

void NexRadStnHdlr::RelListLock()
{
    NexRadDataListLock->rel_lock();
}


void NexRadStnHdlr::threadInit() 
{

  //readInitFile();
  nice(15);   //back off scheduler priority a bit
}

void NexRadStnHdlr::workProc() 
{
  ThreadObj::workProc();  // perform base class operations
    CheckNewScans();
    ProcessCheckedScans();
}

void NexRadStnHdlr::threadExit()
{
  ThreadObj::threadExit();
  //  fprintf(stderr, "NexRadStnHdlr::runLoop EXITING\n");
}


int  NexRadStnHdlr::NewDataAvail(rdr_scan *newscan) {
  //move new radar scan onto linked list of current volumes to consider
  rdr_scan_node   *newscannode;	
  int result = 0;
  int lock_ok = 0;
  if (!NexRadFlag) return 1;

  if (lock && !(lock_ok = lock->get_lock()))
    //    fprintf(stderr,"NexRadStnHdlr::NewDataAvail (pid=%d): get_lock FAILED\n", thread_id);	
    cerr << "NexRadStnHdlr::NewDataAvail (pid=" << thread_id << "): get_lock FAILED\n";
  
  if (newscan && AcceptNewScans && ScanSourceOK(newscan)) {
/*  This occurs before the header has been decoded, rdrscan data will not be valid
    if (debug) printf("NexRadStnHdlr::NewDataAvail: adding scan %s %02d/%d/%d %02d:%02d:%02d\n",
	   stn_name(newscan->station), 
	   newscan->year,newscan->month,newscan->day,newscan->hour,
	   newscan->min,newscan->sec);
*/
    newscannode = new rdr_scan_node(this,"NexRadStnHdlr",newscan);  //
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

int  NexRadStnHdlr::NewDataAvail(CData *newdata) 
{
  //not interested in Cdata
    return 0;
}

// Don't add scan via both New and FInished methods
// If AcceptNewScans defined, don't use FinishedDataAvail
int  NexRadStnHdlr::FinishedDataAvail(rdr_scan *finishedscan) 
{
  rdr_scan_node   *newscannode;	
  int result = 0;
  int lock_ok = 0;
  if (!NexRadFlag) return 1;

  if (lock && !(lock_ok = lock->get_lock()))
    cerr << "NexRadStnHdlr::FinishedDataAvail (pid=" << thread_id << "): get_lock FAILED\n";
  if (finishedscan && !AcceptNewScans && AcceptFinishedScans &&
		ScanSourceOK(finishedscan)) {
     if (finishedscan->rootScan()->Complete()) {
      if (debug) printf(
	   "NexRadStnHdlr::FinishedDataAvail: adding scan %02d/%d/%d %02d:%02d:%02d station %d\n",
	   finishedscan->year,finishedscan->month,finishedscan->day,
	   finishedscan->hour,finishedscan->min,finishedscan->sec,
	   finishedscan->rootScan()->station);
      newscannode = new rdr_scan_node(this,"NexRadStnHdlr",finishedscan);
      incScanNodeCount(); // SD 1may7
      newscannode->prev = 0;
      newscannode->next = newscans;
      if (newscans) newscans->prev = newscannode;  //SD add prev links so dbly linked list
      if (!newscans) newscans_last = newscannode;  //if empty set last ptr to single node
      newscans = newscannode;
      if (!newscans_last) newscans_last = newscannode;
      result = 1;
    }
    
  }
  if (lock_ok) 
    lock->rel_lock();
  return result;
}


// Don't add scan via both New and FInished methods
// If AcceptNewScans defined, don't use FinishedDataAvail
int  NexRadStnHdlr::FinishedDataAvail(CData *finisheddata) {
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
 *   links     -  |               /                |           -----> NexRad
 *        next |  |              /                 |          /
 *             v  |             /                  |         /
 *                |            /                   |        /
 *  newscans_last o >---------/  checkedscans_last o >-----/ 
 * 
 */

void  NexRadStnHdlr::CheckNewScans(bool MoveToChecked, 
			    bool RejectFaulty) {
  //Keep checking the newscans list until a member satisfies the criteria,
  //then move it over to checkedscans list.  Otherwise delete it if it fails
  //some criteria,  or keep it on list for next time
  //move over to checkedscans in chronological order,  last to new

  rdr_scan_node *tempnode, *rootnode;	
  rdr_scan	*rootscan;
  //  rdr_scan	*reflscan, *velscan;
  bool future_scan = FALSE;

  int		lock_ok = 0;
    
	if (!newscans_last)
		return;
	if (lock && !(lock_ok = lock->get_lock()))
	  cerr << "NexRadStnHdlr::CheckNewScans (pid=" << thread_id << "): get_lock FAILED\n";
	//	  fprintf(stderr,"NexRadStnHdlr::CheckNewScans (pid=%d): get_lock FAILED\n", thread_id);	
	
	rootnode = newscans_last;
	//loop thru newscans_last,  oldest to newest
	while (newscans_last) 
	  {
	    //loop  thru current volumes, deleting those irrelevent, moving to 
	    //checkedscans those satisfying certain criteria,  and leaving 
	    //those not yet determined
	    tempnode = (rdr_scan_node *)newscans_last->prev;
	    rootscan = (rdr_scan *)newscans_last->scan->rootScan();
	    future_scan = (ignoreFutureData && (rootscan->scan_time_t - time(0) > futureDataTmWin));;


	    if (!future_scan &&
                !rootscan->Faulty() && 
		rootscan->CheckForValidHeader() &&
		rootscan->station == station && 
		rootscan->scan_type == radarType &&
                (strcmp(vcp_locn,"") || GetScanPattern(rootscan)))  //SD add 22/8/00, and add vcp 4/10/05
	      {
		if (debug) printf(
                  "NexRadStnHdlr::CheckNewScans: moving scan %02d/%d/%d %02d:%02d:%02d\n",
		  newscans_last->scan->year,newscans_last->scan->month,
		  newscans_last->scan->day, newscans_last->scan->hour,
		  newscans_last->scan->min,newscans_last->scan->sec);
		//repair end pointers 
		if (newscans_last == rootnode) rootnode = newscans_last->prev;
		if (newscans_last == newscans) newscans = newscans_last->next;

		if(newscans_last->prev) newscans_last->prev->next = newscans_last->next;
		if(newscans_last->next) newscans_last->next->prev = newscans_last->prev;
		
		//put it into new end of checkedscans
		newscans_last->prev = 0;
		newscans_last->next = checkedscans;
		if (checkedscans) checkedscans->prev = newscans_last;
		if (!checkedscans) checkedscans_last = newscans_last;
		checkedscans = newscans_last;
		//decScanNodeCount();
	      }
	    else if (future_scan ||
                     rootscan->Faulty() ||
		     (rootscan->CheckForValidHeader() && 
		      (rootscan->station != station   ||
		       rootscan->scan_type != radarType)) ||
		       (!strcmp(vcp_locn,"") && !GetScanPattern(rootscan)))  //SD add 22/8/00, and add vcp 4/10/05
	      { //excise link
		if (debug) printf(
		   "NexRadStnHdlr::CheckNewScans: deleting scan " \
		   "%02d/%d/%d %02d:%02d:%02d, radarType = %d, stn = %d\n",
		   newscans_last->scan->year,newscans_last->scan->month,
		   newscans_last->scan->day,newscans_last->scan->hour,
		   newscans_last->scan->min,newscans_last->scan->sec,
		   rootscan->scan_type,rootscan->station);
		//repair end pointers 
		if (newscans_last == rootnode) rootnode = newscans_last->prev;
		if (newscans_last == newscans) newscans = newscans_last->next;

// pjp 222/11/2005
// The rdr_scan_node destructor will fix neighboring links
// The following lines will result in it being done twice - can't be good 	       
// 		if(newscans_last->next) newscans_last->next->prev = newscans_last->prev;
// 		if(newscans_last->prev) newscans_last->prev->next = newscans_last->next;

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

int NexRadStnHdlr::GetScanPattern(rdr_scan* scan)
{
  //see if scan count is listed in scan_patt_arr
  int i;
  
  for (i=0;i< scan_patt_count;i++)  //check vol_scans is correct varble!!!!!
    {
      //debug
      //printf("NexRadStnHdlr::GetScanPattern: pattern %d, arr scans %d, scan->vol_scans %d\n",
      //	   scan_patt_arr[i].pattern,scan_patt_arr[i].scan_count,scan->vol_scans);
      if (scan_patt_arr[i].scan_count == scan->vol_scans) return scan_patt_arr[i].pattern;
    }
  
  return 0;
}

void  NexRadStnHdlr::ProcessCheckedScans(int maxscans) 
{
  //NOTE:  new scans go on one end of LL,  need to read them off the other end
  // so its a queue, not  a stack!! See diagram above.
  rdr_scan_node   *tempscannode,*rootnode;	
  rdr_scan	  *tempscan;
  NexRad	  *NexRad_out;
  //  int             NexRadfd;
  int             outfileflag = 0;
  bool		scanComplete = false,  scanFinished = false;
  bool          pathOK = false;
  bool          vcpFileWritten = false;
  
  
  //int             local_port;
  
  /*
  if (debug) 
    {
      printf(".");
      fflush(stdout);
    }
  */
  
  //process members of checkedscans,  convert to NexRad!
  //if node has ptr to nexrad object, continue trying to process it
  //   until its done
  //if it returns uncompleted,  store ptr to it and continue with next node
  //if it does complete,  delete link in LL
  //if no ptr, construct new one 
  rootnode = checkedscans_last;
  while (checkedscans_last && !stopFlag)
    {
      tempscannode = (rdr_scan_node *)checkedscans_last->prev;
      tempscan = (rdr_scan *)checkedscans_last->scan->rootScan();
      if (debug) printf(
	   "NexRadStnHdlr::ProcessCheckedScans: scan s%02d %02d/%d/%d %02d:%02d:%02d\n",
	   tempscan->station,
	   tempscan->year,tempscan->month,tempscan->day,tempscan->hour,
	   tempscan->min,tempscan->sec);

      //process end of volume, write NexRad
      char NexRadfilename[256],NexRadfilenameNew[256];

      //open output file 
      outfileflag = 0;
      
      if (outputdesc[0]->fs_flag == NEXRAD_FILE)
	{
	  if (!(pathOK = DirExists(path)))
	    {
	      if ((pathOK = DirExists(path, true, true)))
		printf("NexRadStnHdlr::ProcessCheckedScans: - Created directory %s\n", path);
	      else
		printf("NexRadStnHdlr::ProcessCheckedScans: - Unable to create path %s\n", path);
	    }
	  if (pathOK)
	    {
	      sprintf(NexRadfilename,"%s/%02d_%04d%02d%02d_%02d%02d%02d.nexrad",
		      path,
		      tempscan->station,  
		      tempscan->year,tempscan->month,tempscan->day,
		      tempscan->hour,tempscan->min,tempscan->sec);
	      strcpy(NexRadfilenameNew,NexRadfilename);
	      if (!noTempFile)
		strcat(NexRadfilenameNew,".new");
	      outputdesc[0]->open_file(NexRadfilenameNew);
	    }
	  if (outputdesc[0]->fd != -1) outfileflag = 1;
	}
      
      //open all the sockets
      for (int i = 1;i<outputcount;i++)
	  outputdesc[i]->open_socket(local_port++);
	  
      
      printf("NexRadStnHdlr::ProcessCheckedScans:  Writing NEXRAD dataset\n");
      //create new nexrad object,  put in node
      VolumeNo = update_vol_scan_no();

      //find VCP number, depends on wx vcp_locn set or not
      int vcp_num = 0;
      int vol_id = tempscan->volume_id;
      if (vol_id == -1) vol_id = 0;
      //mask vol_id to 2 bits
      vol_id = vol_id&3;
      if (strcmp(vcp_locn,"")) {  //not == so it exists
	if (tempscan->rootScan()->Complete() && !vcpFileWritten) {
	  vcp_num = computeAndWriteVcp(vol_id, tempscan);
	  vcpFileWritten = true;
	}
	else  //use the last volumes computed vcp number
	  vcp_num = vcpNumPerVolumeId[vol_id]; 
      }
      else vcp_num = GetScanPattern(tempscan);
      NexRad_out =  new NexRad(tempscan,
			       outputdesc,
			       outputcount,
			       //pattern,  SD alt 22/8/00.  SD Replace with computed VCP if nec! 30/9/05
			       //GetScanPattern(tempscan),
			       vcp_num,
			       packet_size,
			       debug,
			       VolumeNo,
			       seq_num,
			       fr_seq,
			       delay_pac, 
			       filter_Vel,
			       filter_Refl,
			       filter_SpWdth,
			       refl_rngres_reduce_factor,
			       vel_rngres_reduce_factor,
			       min_time_per_tilt); 
      int count = 0;
      //keep trying to write it out
      while(!NexRad_out->checkWrite() &&
	    count++ < NEXRAD_CHECKWRITE_LIMIT &&
	    !((scanFinished = tempscan->Finished()) && !(scanComplete = tempscan->Complete()))) 
	{ 
	  if (debug) 
	    {
	      printf("*");
	      fflush(stdout);
	    }
	  if (stopFlag) //check obituaries for parent
	    {
	      NexRad_out->writeLast();
	      if (outfileflag && !noTempFile)
		if (rename(NexRadfilenameNew,NexRadfilename) == -1)
		  {
		    /*
		    fprintf(stderr,"\nNexRadStnHdlr::ProcessCheckedScans:  (pid=%d)"
			    "ERROR  Cannot rename output file from %s to %s\n", thread_id,
			    NexRadfilenameNew,NexRadfilename);
		    */
		    cerr << "\nNexRadStnHdlr::ProcessCheckedScans:  (pid=" << thread_id << 
		      ") ERROR  Cannot rename output file from " << NexRadfilenameNew << " to " <<
		      NexRadfilename << "\n";
		    perror("NexRadStnHdlr::ProcessCheckedScans");
		    // exiting is BAD		  exit(-1);
		  }
 	      break; 
	    }
	  sleep(2);  
	}

      if (count >= NEXRAD_CHECKWRITE_LIMIT) 
	   NexRad_out->writeLast(); //given up on scan
      if (tempscan->Finished() && !tempscan->Complete()) 
	   NexRad_out->writeLast();
      //recover packet sequence number from output object
      seq_num = NexRad_out->seq_number;
      fr_seq  = NexRad_out->fr_seq;
      //delete NexRad_out;  //purify find
      local_port = start_local_port;  //reset port to that passed in constr.

      //VCP--------------------
      //have a complete volume here, so if vcp_locn set, and its not already written, 
      //set up elevations for this VOLUMEID,
      //construct VCP number and write out VCP file.
      if (strcmp(vcp_locn,"") && !vcpFileWritten) {     //not == so it exists
	vcp_num = computeAndWriteVcp(vol_id, tempscan);
	vcpFileWritten = true;
      }

      if (debug) printf(
			"NexRadStnHdlr::ProcessCheckedScans: scan s%02d %02d/%d/%d %02d:%02d:%02d NEXRAD TX COMPLETE\n"
			"scanComplete = %d,  scanFinished = %d",
			tempscan->station,
			tempscan->year,tempscan->month,tempscan->day,tempscan->hour,
			tempscan->min,tempscan->sec, 
			scanComplete, scanFinished
			);
      
      //SD addition 14/2/05 for rdrInvent.C
      if (writeSimRapic) {
	int scan_ind = 0;
	rdr_scan *src_scan = 0;
	rdrInvent *rdrInvInst = new rdrInvent(s1fd);
	printf("\nNexRadStnHdlr::ProcessCheckedScans: calling rdrInvent->createScanProduct\n");
	while ((src_scan = NexRad_out->goto_scan(scan_ind++))) {
	  rdrInvInst->createScanProduct(src_scan);
	}
      }
      
      
      //succeeded,  remove this link and nexrad object
      //           & repair end pointers
      if (checkedscans_last == rootnode) rootnode = checkedscans_last->prev;
      if (checkedscans_last == checkedscans) 
	checkedscans = checkedscans_last->next;

// pjp 222/11/2005
// The rdr_scan_node destructor will fix neighboring links
// The following lines will result in it being done twice - can't be good 	       
//       if (checkedscans_last->next) 
// 	checkedscans_last->next->prev = checkedscans_last->prev;
//       if (checkedscans_last->prev) 
// 	checkedscans_last->prev->next = checkedscans_last->next;
      delete checkedscans_last;
      decScanNodeCount();
      
      //fclose(NexRad_out->fd);	// should let destructor do this
      // close(NexRad_out->fd); // should let destructor do this
      if (NexRad_out) {
	delete NexRad_out;
	NexRad_out = 0;	// redundant, but can be good practice
      }
      checkedscans_last = tempscannode;
      //rename output file if its been opened
      if (outfileflag && !noTempFile)
	if (rename(NexRadfilenameNew,NexRadfilename) == -1)
	  {
	    /*
	      fprintf(stderr,"\nNexRadStnHdlr::ProcessCheckedScans:  (pid=%d)"
	      "ERROR  Cannot rename output file from %s to %s\n", thread_id,
	      NexRadfilenameNew,NexRadfilename);
	    */
	    cerr << "\nNexRadStnHdlr::ProcessCheckedScans:  (pid=" << thread_id << 
	      ") ERROR  Cannot rename output file from " << NexRadfilenameNew << " to " <<
	      NexRadfilename << "\n";
	    perror("NexRadStnHdlr::ProcessCheckedScans");
	    // exiting is BAD	  exit(-1);
	  }
    }
  //restore list root
  checkedscans_last = rootnode; 
  //if queue empty,  make both end ptrs consistent. Redundant?
  if (!checkedscans_last) checkedscans = 0;
}


/*read and write a file with vol scan number, st:
  if file does not exist, create it with vs # of 0,
  else if time is 0Z - 1Z and vs # > 20 say then reset vs to 0
  else vs++
  write it back and close
*/



int NexRadStnHdlr::computeAndWriteVcp(int vol_id, rdr_scan* tempscan) 
{
      //SD 21/may/07
      //VCP--------------------
      //have a complete volume here, so if vcp_locn set, set up elevations for this VOLUMEID,
      //construct VCP number and write out VCP file.
      
	rdr_scan* loopscan = tempscan->rootScan();
	rdr_angle last_scan_angle = -1;
	int elevs = 0;
	char vcp_header[1024];
	
	char vcp_str[1024];
	char vcp_line[80];
	
	strcpy(vcp_str,"");
	strcpy(vcp_line,"");
	
	while (loopscan){
	  if (loopscan->set_angle != last_scan_angle) {
	    //new elevation, add angle to elevArray list and elevCount for this volume_id
	    elevs++;
	    sprintf(vcp_line,"  <angle>%4.1f</angle>\n",((float)loopscan->set_angle)/10.0);
	    strcat(vcp_str,vcp_line);
	  }
	  last_scan_angle = loopscan->set_angle;
	  loopscan = tempscan->NextScan(loopscan);
	}
	
	//now we can compute the VCP number
	// First 5 bits = elevation count,  giving max of 31 elevations.
	// Next  2 bits = rapic VOLUMEID, giving 4 possible values.
	// Final 8 bits = station number without country code, max of 255.
	vcpNumPerVolumeId[vol_id] = (tempscan->station&255) + vol_id*256 + elevs*1024;
	//write out VCP file header
	sprintf(vcp_header,
		"<vcp number=\"%d\" isNexrad=\"false\" isClearAir=\"true\" canCache=\"false\">\n"
		"<!-- this is an Australian (RAPIC) volume coverage pattern. -->\n"
		"<!-- %s %d elevs vol_id %d %04d%02d%02d:%02d%02d -->\n",
		vcpNumPerVolumeId[vol_id],stn_name(station),elevs,vol_id,tempscan->year,tempscan->month,
		tempscan->day,tempscan->hour,tempscan->min);
	char *vcp_filename = new char[1024];
	char *vcp_filename_new = new char[1024];
	sprintf(vcp_filename,"%s/vcp%d",vcp_locn,vcpNumPerVolumeId[vol_id]);
	sprintf(vcp_filename_new,"%s/vcp%d.new",vcp_locn,vcpNumPerVolumeId[vol_id]);
	
	FILE *vcp_fd;
	if ((vcp_fd = fopen(vcp_filename_new,"w"))) {
	  fprintf(stdout, "NexRadStnHdlr opening %s\n",vcp_filename_new);
	  fprintf(vcp_fd,"%s%s</vcp>\n",vcp_header,vcp_str);
	  fclose(vcp_fd);
	  //move file to final name
	  if (rename(vcp_filename_new,vcp_filename) == -1)
	    {
	      cerr << "\nNexRadStnHdlr::ProcessCheckedScans: " << 
		"ERROR  Cannot rename VCP output file from " << vcp_filename_new << " to " <<
		vcp_filename << "\n";
	      perror("NexRadStnHdlr::ProcessCheckedScans");
	    }
	}
	else {
	  fprintf(stderr, "NexRadStnHdlr FAILED opening %s \n",vcp_filename);
	  perror(0);
	}
      return (tempscan->station&255) + vol_id*256 + elevs*1024;
      //END VCP--------------------
}


int NexRadStnHdlr::update_vol_scan_no()
{
  int vs = 1;
  time_t curr_time = time(0);
  struct tm *timestr;
  char filename[256];
  
  
  FILE *vsfd;
  sprintf(filename,"nexrad_vsno_%d.dat",station);
  if (!(vsfd = fopen(filename,"r+"))) 
    {
      /*need to create the file*/
      if(!(vsfd = fopen(filename,"w")))
	{
	  /*cant open, arrh*/
	  /*
	    fprintf(stderr,"NexRadStnHdlr::update_vs:  (pid=%d)"
	    "ERROR, Cant open nexrad_volscan.dat, using vs = 1\n", thread_id);
	  */
	  cerr << "NexRadStnHdlr::update_vs:  (pid=" << thread_id << 
	    ") ERROR, Cant open nexrad_volscan.dat, using vs = 1\n";
	  perror("NexRadStnHdlr::update_vs");
	  // exiting is BAD	  exit(-1);
	  return 1;
	}
      fprintf(vsfd,"00001");
      fclose(vsfd);
      return vs;
    }
  
  if (fscanf(vsfd,"%d",&vs) != 1)
    {
      /*cant read, arrh*/
      /*
      fprintf(stderr,"NexRadStnHdlr::update_vs:  (pid=%d)"
	      "ERROR, Cant read nexrad_volscan.dat, using vs = 1\n", thread_id);
      */
      cerr << "NexRadStnHdlr::update_vs:  (pid=" << thread_id << 
	") ERROR, Cant read nexrad_volscan.dat, using vs = 1\n";
      perror("NexRadStnHdlr::update_vs");
// exiting is BAD      exit(-1);
      fclose(vsfd);
	return 1;
    }
  timestr =  gmtime(&curr_time);
  if (timestr->tm_hour == 0 && vs > NEX_VS_THRESH)
    {
      /*it is now sometime within an hour of 0Z, and vs needs resetting*/
      vs = 1;
    }
  else if (vs > 999) vs = 1;  //WDSS overflows if greater
  else vs++;
  rewind(vsfd);
  if (fprintf(vsfd,"%05d     \n",vs) < 0)
    {
      /*cant write, arrh*/
      /*
      fprintf(stderr,"NexRadStnHdlr::update_vs:  (pid=%d) Cant write nexrad_volscan.dat\n", thread_id);
      */
      cerr << "NexRadStnHdlr::update_vs:  (pid=" << thread_id <<
	") Cant write nexrad_volscan.dat\n";
      perror("NexRadStnHdlr::update_vs");
// exiting is BAD      exit(-1);
    }

  fclose(vsfd);
  return vs;
}
  

