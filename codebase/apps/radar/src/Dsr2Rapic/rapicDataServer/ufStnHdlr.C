/*
 * ufStnHdlr implementation
 * Convert rapic data into Universal Format (UF).  Writes data out to file in
 * directory 'path' given in uf.ini file.
 * UF files can be viewed with solo (ftp://ftp.atd.ucar.edu/archive/rdpdist)
 * after conversion by the xltrs program provided by solo.
 * Written by Sandy Dance 2 Nov 2001.
 */

#ifdef SGI
#include <sys/bsd_types.h>
#endif
#ifdef STDCPPHEADERS
#include <iostream>
#else
#include <iostream.h>
#endif

#include <signal.h>
#include <sys/types.h>
// #include <sys/prctl.h>
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
#include "ufStnHdlr.h"
static char const cvsid[] = "$Id: ufStnHdlr.C,v 1.2 2008/02/29 04:44:54 dixon Exp $";
char	rereadflagname[] = "rereaduf.ini"; // flag file to reload new config

ufStnHdlr::ufStnHdlr(int station,
		     char *path,
		     e_scan_type radarType,
		     bool debug,
		     bool ufFlag) : scan_client()
{
  this->station                  = station;
  strcpy(this->path,path);
  this->radarType                = radarType;
  this->debug                    = debug;
  this->ufFlag = ufFlag;
  strcpy(stationName,stn_name(station));
  strcpy(radarTypeStr,get_scan_type_short(radarType));
  ClientType = SC_UFMNG;
  lock = new spinlock("ufStnHdlr::ufStnHdlr", 3000); // 30 secs
  ufDataListLock = new spinlock("ufStnHdlr::ufStnHdlr", 3000); // 30secs
  AcceptNewScans = FALSE;      //was SD 5/1/4
  AcceptFinishedScans = TRUE;  //was SD 5/1/4
  //AcceptNewScans           = TRUE;
  //AcceptFinishedScans      = FALSE;
  AcceptDuplicates         = FALSE;
  AllowReplayMode = true;  // allow replay mode
  AllowDBReviewMode = true;  // allow DBReview mode
  newscans_last = 0;
  checkedscans_last = 0;
  VolumeNo = 0;
  strcpy(threadName, "ufStnHdlr");
  
  printf("ufStnHdlr::start station %d\n",station);  
   
  //why do we need ppivol?  for setUfParams!
  ppivol.scans = (struct scan *)calloc(1,sizeof(struct scan));
  memset((void *)&ppivol.rparams,0,sizeof(struct rapicradar));
  memset((void *)ppivol.scans,0,sizeof(struct scan));
  ppivol.scans->gatedata = (struct gate *) calloc(MAXAZS,sizeof(struct gate));
  seqno        = 0; 
  imageno      = 0;
  workProcTimeout = 60;    // 60 seconds
   if (!ufFlag) return;
    startThread();  
}	

ufStnHdlr::~ufStnHdlr() 
{
 
  if (ScanMng && ufFlag) ScanMng->RemClient(this);
  fprintf(stderr, "ufStnHdlr::~ufStnHdlr Stopping thread....\n");
  stopThread();

  free (ppivol.scans->gatedata);
  free (ppivol.scans);
    
  if (lock) 
    {
      lock->rel_lock();
      delete lock;        
    }
  if (ufDataListLock) 
    {
      ufDataListLock->rel_lock();
      delete ufDataListLock;        
    }
}

void ufStnHdlr::GetListLock()
{
  ufDataListLock->get_lock();
}

void ufStnHdlr::RelListLock()
{
  ufDataListLock->rel_lock();
}

void ufStnHdlr::readInitFile(char *initfilename)
{
	
}

void ufStnHdlr::threadInit() {
  //readInitFile();
  nice(15);   //back off scheduler priority a bit
}

void ufStnHdlr::workProc() {

  //SD debug
  //printf("ufStnHdlr::workProc()\n");
  
  ThreadObj::workProc();  // perform base class operations
  CheckNewScans();
  ProcessCheckedScans();
}


int  ufStnHdlr::NewDataAvail(rdr_scan *newscan) {
  //move new radar scan onto linked list of current volumes to consider
  rdr_scan_node   *newscannode;	
  int result = 0;
  int lock_ok = 0;
  //SD debug
  //printf("ufStnHdlr::NewDataAvail: first\n");
  
  if (!ufFlag) return 1;

  if (lock && !(lock_ok = lock->get_lock()))
    fprintf(stderr,"ufStnHdlr::NewDataAvail: get_lock FAILED\n");	
  if (newscan && AcceptNewScans) {
    printf("ufStnHdlr::NewDataAvail: adding scan %02d/%d/%d %02d:%02d:%02d\n",
	   newscan->year,newscan->month,newscan->day,newscan->hour,
	   newscan->min,newscan->sec);
    newscannode = new rdr_scan_node(this,"ufStnHdlr",newscan);
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

// Don't add scan via both New and FInished methods
// If AcceptNewScans defined, don't use FinishedDataAvail
int  ufStnHdlr::FinishedDataAvail(rdr_scan *finishedscan) 
{
  rdr_scan_node   *newscannode;	
  int	    result = 0;
  int		lock_ok = 0;
  //SD debug
  //printf("ufStnHdlr::FinishedDataAvail: first\n");
  
  if (!ufFlag) return 1;

  if (lock && !(lock_ok = lock->get_lock()))
    fprintf(stderr,"ufStnHdlr::FinishedDataAvail: get_lock FAILED\n");	
  if (finishedscan && !AcceptNewScans && AcceptFinishedScans) {
    if (debug) printf(
		      "ufStnHdlr::FinishedDataAvail: adding scan %02d/%d/%d %02d:%02d:%02d\n",
		      finishedscan->year,finishedscan->month,finishedscan->day,
		      finishedscan->hour,finishedscan->min,finishedscan->sec);
    newscannode = new rdr_scan_node(this,"ufStnHdlr",finishedscan);
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
 *   links     -  |               /                |           -----> Uf
 *        next |  |              /                 |          /
 *             v  |             /                  |         /
 *                |            /                   |        /
 *  newscans_last o >---------/  checkedscans_last o >-----/ 
 * 
 */

void  ufStnHdlr::CheckNewScans(bool MoveToChecked, 
			   bool RejectFaulty) {
  //Keep checking the newscans list until a member satisfies the criteria,
  //then move it over to checkedscans list.  Otherwise delete it if it fails
  //some criteria,  or keep it on list for next time
  //move over to checkedscans in chronological order,  last to new

  rdr_scan_node *tempnode, *rootnode;	
  rdr_scan	*rootscan;
  //  rdr_scan	*reflscan, *velscan;

  int		lock_ok = 0;
    
  if (!newscans_last)
    return;
  if (lock && !(lock_ok = lock->get_lock()))
    fprintf(stderr,"ufStnHdlr::CheckNewScans: get_lock FAILED\n");	
	
  rootnode = newscans_last;
  //loop thru newscans_last,  oldest to newest
  while (newscans_last) 
    {
      //loop  thru current volumes, deleting those irrelevent, moving to 
      //checkedscans those satisfying certain criteria,  and leaving 
      //those not yet determined
      tempnode = (rdr_scan_node *)newscans_last->prev;
      rootscan = (rdr_scan *)newscans_last->scan->rootScan();

      if (!rootscan->Faulty() && 
	  rootscan->CheckForValidHeader() && 
	  rootscan->station == station &&
	  rootscan->scan_type == radarType ) 
	{
	  if (debug) printf(
			    "ufStnHdlr::CheckNewScans: moving scan %02d/%d/%d %02d:%02d:%02d\n",
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
	  decScanNodeCount();
	}
      else if (rootscan->Faulty() || 
	       (rootscan->CheckForValidHeader() && //dont get stn or time if no header
		(rootscan->station != station || //delete if wrong station 
		 rootscan->scan_type != radarType ))) 
	{ //excise link
	  if (debug) printf(
			    "ufStnHdlr::CheckNewScans: deleting scan " \
			    "%02d/%d/%d %02d:%02d:%02d, radarType = %d, stn = %d\n",
			    newscans_last->scan->year,newscans_last->scan->month,
			    newscans_last->scan->day,newscans_last->scan->hour,
			    newscans_last->scan->min,newscans_last->scan->sec,
			    rootscan->scan_type,rootscan->station);
	  if (newscans_last == rootnode) rootnode = newscans_last->prev;
	  if(newscans_last->next) newscans_last->next->prev = newscans_last->prev;
	  if(newscans_last->prev) newscans_last->prev->next = newscans_last->next;
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

void  ufStnHdlr::ProcessCheckedScans(int maxscans) 
{
  //NOTE:  new scans go on one end of LL,  need to read them off the other end
  // so its a queue, not  a stack!!
  /*
    openvolume
    init_ppivol(rootscan)
    lastelev = -1
    for each scan in vol
    if not first & thiselev != lastelev
    setScanData(scan,scanPtr)
    if thiselev != lastdev
    init_scan(scanPtr)
    addScanData(scan,scanPtr) with approp. field set
    lastelev = thiselev

    setScanData(scan,scanPtr)  //write last elevation
    closeVolume
  */
  rdr_scan_node   *tempscannode;	
  rdr_scan	  *tempscan;
  rdr_scan	  *tempScanElev;
  //  uf		  *uf_out;
  char 		   uffilename[256];
  //  FILE		  *uffd;
  struct scan     *scandata;
  int		   lastelevang;
  int              scanngates;
  

  if (debug) 
    {
      printf(".");
      fflush(stdout);
    }
  
  //process members of checkedscans,  send to Uf!
  while (checkedscans_last && !stopFlag)
    {
      tempscannode = (rdr_scan_node *)checkedscans_last->prev;
      tempscan = (rdr_scan *)checkedscans_last->scan->rootScan();
      if (debug) printf(
			"ufStnHdlr::ProcessCheckedScans: scan %02d/%d/%d %02d:%02d:%02d\n",
			tempscan->year,tempscan->month,tempscan->day,tempscan->hour,
			tempscan->min,tempscan->sec);
      
      //open output file in default dir
      sprintf(uffilename,"%s%s%04d%02d%02d_%02d%02d%02d.uf",
	      stationName,radarTypeStr,
	      tempscan->year,tempscan->month,tempscan->day,
	      tempscan->hour,tempscan->min,tempscan->sec);
      printf("ufStnHdlr::ProcessCheckedScans:  Writing UF dataset to %s/%s\n",
	     path,uffilename);
      tempScanElev = tempscan->rootScan();
      ufInst = new uf(debug);

      if (ufInst->openVolume(path,uffilename))
	{
	  //check open OK?
	  if (init_ppivol(tempScanElev) == 0) {
	    
	  scanngates = int(1+(tempScanElev->max_rng*1000-tempScanElev->start_rng)/tempScanElev->rng_res);
	  //ufInst->setUfParams(&ppivol.rparams,&ppivol.scans[0],scanngates);
	  scandata = ppivol.scans;
	  
	  int elevind = 0;
	  int setUfOk = 0;
	  
	  lastelevang = -1;
	  
	  while (tempScanElev)  //and uf write OK?
	    {
	      //write out last elevation's data (if not first)
	      if (elevind && lastelevang != tempScanElev->set_angle)
		ufInst->setScanData(scandata);
	      
	      //add this elev and field to ppivol
	      if (lastelevang != tempScanElev->set_angle)
		{
		  elevind++;
		  init_scan(tempScanElev);
		  if (!setUfOk++) ufInst->setUfParams(&ppivol.rparams,&ppivol.scans[0],scanngates);
		}
	      
	      lastelevang = tempScanElev->set_angle;
	      addRapic2Scan(tempScanElev);
	      
	      tempScanElev = tempScanElev->NextScan(tempScanElev);
	    }
	  //SD debug
	  //dumpBuff();
	  
	  ufInst->setScanData(scandata); //write out last elevation's data, check uf write OK?
	  
	  //SD debug
	  //ufInst->dumpData(&ppivol);
	  
	  ufInst->closeVolume();
	  }
	  
	  else {
	    printf("ufStnHdlr::ProcessCheckedScans: ERROR, NOT Writing UF dataset to %s/%s due to lack of level data\n",
	     path,uffilename);
	    ufInst->closeVolume();
	  }
	  

	  
	}
      //remove old checkedscans.
      if(checkedscans_last->prev) checkedscans_last->prev->next = 0;      
      delete checkedscans_last;
      delete ufInst;
      checkedscans_last = tempscannode;
    }
  checkedscans = 0;  //this queue must be empty here
} 

void ufStnHdlr::addRapic2Scan(rdr_scan *rdrscan)
{
  //update scan data from rdrscan, put in appropriate field
  //ie, refl is field 0, vel field 1, others?
  //assume memory allocated in init_ppivol, memset here if nec
  int gateno,azidx;
  int field=0,index=0;
  char type[10];
  
  s_radl radl;
  short radlpos = 0;
  exp_buff_rd *dbuffrd = new exp_buff_rd();
    
  dbuffrd->open(rdrscan->DataBuff());

  struct scan *scandata = ppivol.scans;
  rdr_scan *rootscan = rdrscan->rootScan();
  int numlevels = rootscan->NumLevels;

  //get field index
  if (rdrscan->data_type == e_refl)    strcpy(type,"REFL");
  if (rdrscan->data_type == e_vel)    	strcpy(type,"VEL");
  if (rdrscan->data_type == e_spectw) 	strcpy(type,"WID");
  if (rdrscan->data_type == e_diffz)  	strcpy(type,"ZDR");
  if (rdrscan->data_type == e_rawrefl) strcpy(type,"UNCORREFL");
  ufInst->setnumFields(type,field,index);
  ppivol.numfields = max((int)field+1,ppivol.numfields);

  ppivol.nelevations    = rdrscan->vol_scans/ppivol.numfields;
  scandata->nfields     = ppivol.numfields;
  if (rdrscan->data_type == e_vel) 
    scandata->nyq         = rdrscan->nyquist;
  //SD debug
  //ufInst->debugFields();

  //loop for this field gates within azimuths
  scandata->nazimuths     = 3600/rdrscan->angle_res;
  for (azidx=0;azidx < scandata->nazimuths; azidx++)
    {
      if (rdrscan->get_radl_angl(&radl,azidx*rdrscan->angle_res, dbuffrd, &radlpos) == -1) continue;
      //SD debug
      //if (az==94) printf("ufStnHdlr::***NumLevels = %d\n",radl.numlevels);
      scandata->fieldlevel[index]    = radl.numlevels;
      scandata->gatedata[azidx].ngates  = radl.data_size; 
      scandata->gatedata[azidx].azimuth = radl.az_f();
      scandata->gatedata[azidx].elev    = radl.el_f();
      scandata->gatedata[azidx].time    = 0; 
      //rdrscan->LastScanTm; //time format? Is it incr sec since last scan

      for (gateno=0;gateno<radl.data_size;gateno++)
	{
	  //for each bin
	  if (radl.data[gateno] < numlevels)
	    scandata->gatedata[azidx].data[index][gateno] = radl.data[gateno]; //char
	}
    }
  if (dbuffrd) delete dbuffrd;
}


void ufStnHdlr::init_scan(rdr_scan *rdrscan)
{
  int ii;
  struct gate *ptr;
  //  struct tm *gmt_tm;
  //  time_t scan_time;
  struct scan *scandata = ppivol.scans;
 
  ptr = scandata->gatedata; //preserve allocated mem for gatedata
  memset((void *)scandata,0,sizeof(struct scan));
  scandata->gatedata = ptr;
  for ( ii = 0; ii < MAXAZS; ii++ ) {
    ptr[ii].azimuth = ii;
    ptr[ii].ngates  = 0;
  }

  scandata->numdbmlvls    = rdrscan->NumLevels;
  scandata->nazimuths     = 0;
  scandata->startrange    = int(rdrscan->start_rng); 
  //scandata->nfields     = ppivol.numfields; ??fill in later?
  scandata->seqno         = seqno;
  scandata->imageno       = imageno++;
  //  int  	nscans;
  //  long  	imagesize;
  //  long  	thisscan;
  //  int  	eoi;
  strcpy(scandata->station,stn_name(rdrscan->station));  
  scandata->stnid         = rdrscan->station;
  scandata->year          = rdrscan->year;
  scandata->month         = rdrscan->month;
  scandata->day           = rdrscan->day;
  scandata->hour          = rdrscan->hour;
  scandata->min           = rdrscan->min;
  scandata->sec           = rdrscan->sec;
  scandata->timed         = 1;   //??
  scandata->unix_time     = rdrscan->scan_time_t;
  scandata->vers          = 10.01; //rapic doesnt store this!
  scandata->startrange    = int(rdrscan->start_rng);
  scandata->endrange      = int(rdrscan->max_rng*1000);  //in meters
  scandata->rngres        = int(rdrscan->rng_res);
  scandata->angres        = rdrscan->angle_res/10.0; //deg
  scandata->vidres        = rdrscan->NumLevels; //I assume
  //pass ??
  if (rdrscan->scan_type == PPI)       strcpy(scandata->product,"PPI");
  if (rdrscan->scan_type == RHI)       strcpy(scandata->product,"RHI");
  if (rdrscan->scan_type == CompPPI)   strcpy(scandata->product,"CompPPI");
  if (rdrscan->scan_type == IMAGE)     strcpy(scandata->product,"IMAGE");
  if (rdrscan->scan_type == VOL)       strcpy(scandata->product,"VOL");
  if (rdrscan->scan_type == RHISet)    strcpy(scandata->product,"RHISet");
  if (rdrscan->scan_type == MERGE)     strcpy(scandata->product,"MERGE");
  if (rdrscan->scan_type == SCANERROR) strcpy(scandata->product,"SCANERROR");
  if (rdrscan->scan_type == CAPPI)     strcpy(scandata->product,"CAPPI");
  //  char   	imgfmt[15];??
  scandata->elev          = rdrscan->set_angle/10.0;
  scandata->dbzcor        = 0; //applied already by rapic check_data()
  if (rdrscan->LvlTbls(e_refl))
    {
      for (ii=0;ii<rdrscan->NumLevels;ii++)
	//scandata->dbmlvl[ii]  = rdrscan->rootScan()->LvlTbls[e_refl]->Levels[ii];  SD 30/11/4
	scandata->dbmlvl[ii]  = rdrscan->LvlTbls(e_refl)->val(ii);
    }
  //SD debug 8/1/4
  /*
  for (ii=0;ii<rdrscan->NumLevels;ii++) {
    rdr_scan *tempscan;
    tempscan = rdrscan->rootScan();
    LevelTable *tempLvlTbl = tempscan->LvlTbls[e_refl];
    if (tempLvlTbl == 0) {
      printf("ufStnHdlr::ERROR LvlTbls null\n");
      fflush(stdout);
    }
    float *levelPtr = tempLvlTbl->Levels;
    if (levelPtr == 0) {
      printf("ufStnHdlr::ERROR levelPtr null\n");
      fflush(stdout);
    }
    
    float level = levelPtr[ii];
    scandata->dbmlvl[ii]  = level;    
  }
  */
  scandata->numdbmlvls    = rdrscan->NumLevels; 
  scandata->nazimuths     = 3600/rdrscan->angle_res;
  scandata->nfields       = 1; //will need to update this as its discovered!
  scandata->scantype      = rdrscan->scan_type;
  if (rdrscan->data_type == e_vel) 
    scandata->nyq         = rdrscan->nyquist;
  scandata->field         = (RAPICFIELDS)max(int(scandata->field),int(rdrscan->data_type));
  //  int        lassenfield;
  scandata->numgates      = int(rdrscan->max_rng*1000/rdrscan->rng_res);
  sprintf(scandata->timestamp,"%02d%02d%02d%02d%02d%02d",
	  rdrscan->year,rdrscan->month,rdrscan->day,rdrscan->hour,rdrscan->min,rdrscan->sec);
  scandata->latitude      = StnRec[rdrscan->station].Lat();
  scandata->longitude     = StnRec[rdrscan->station].Lng();
  scandata->height        = StnRec[rdrscan->station].Ht();
  scandata->frequency     = rdrscan->frequency*1000000.0 ;  //in hertz
  scandata->prf           = rdrscan->prf;
  //  float      pulselength; Not decoded by rapic
  //  float      anglerate;   Not decoded by rapic
  //  char       unfolding[20]; no
}

int ufStnHdlr::init_ppivol(rdr_scan *rdrscan)
{
  //set up memory for ppivol struct components
  //  int ii;
  //struct scan *scandata;
  struct gate *ptr;

  //Test excise SD 30/11/4      rdrscan->NumLevels
  //if (rdrscan->rootScan()->LvlTbls[e_refl]->Levels == 0) {  //Levels is the array of float dbz values SD 30/11/4
  //  printf("ufStnHdlr::init_ppivol: ERROR no levels in scan, skipping.\n");
  //  return 1;
  //}
  if (rdrscan->rootScan()->NumLevels == 0) {  //Levels is the array of float dbz values SD 30/11/4
    printf("ufStnHdlr::init_ppivol: ERROR no levels in scan, skipping.\n");
    return 1;
  }
  
  seqno++;

  //why do we need ppivol?  for setUfParams!
  memset((void *)&ppivol.rparams,0,sizeof(struct rapicradar));
  ptr = ppivol.scans->gatedata; //preserve allocated mem for gatedata
  memset((void *)ppivol.scans,0,sizeof(struct scan));
  ppivol.scans->gatedata = ptr;
  //
  //update params
  //Get station info
  strcpy(ppivol.rparams.stationname,stn_name(rdrscan->station));
  strcpy(ppivol.rparams.radarname,  rdrscan->rdr_params.name);
  ppivol.rparams.latitude    = StnRec[rdrscan->station].Lat();
  ppivol.rparams.longitude   = StnRec[rdrscan->station].Lng();
  ppivol.rparams.altitude    = int(StnRec[rdrscan->station].Ht());
  ppivol.rparams.stnid       = rdrscan->station;
  ppivol.rparams.prf         = rdrscan->prf;
  //type?
  ppivol.rparams.wavelength  = int((LIGHTVEL)/(rdrscan->frequency*1000000.0)); //mm
  //npulses?
  //pulsewidth?
  //update numfields and nelevations!! later?
  //ppivol.nelevations         = rdrscan->completed_tilts; I got a '1'!
  ppivol.numfields           = 0; //max found later
  return 0;
  
}

void ufStnHdlr::dumpBuff()
{

  //dump contents of buffer sent to ufInst
  printf("ppivol = \n");
  printf("   numfields %d nelevations %d\n",ppivol.numfields,ppivol.nelevations);
  printf("rparams = \n");
  printf("latitude;  %f     \n"
	 " longitude; %f     \n"
	 " stationname %s    \n"
	 " radarname %s	    \n"
	 " stnid;   %d       \n"
	 " altitude; %d      \n"
	 " prf;    %d        \n"
	 " type;   %d        \n"
	 " wavelength; %d    \n"
	 " npulses;  %d      \n"
	 " pulsewidth; %d\n",    
	 ppivol.rparams.latitude,       
	 ppivol.rparams.longitude,      
	 ppivol.rparams.stationname,
	 ppivol.rparams.radarname,  
	 ppivol.rparams.stnid,          
	 ppivol.rparams.altitude,       
	 ppivol.rparams.prf,            
	 ppivol.rparams.type,           
	 ppivol.rparams.wavelength,     
	 ppivol.rparams.npulses,        
	 ppivol.rparams.pulsewidth);
  printf("scan = \n");
  printf("     seqno;           %d \n"              
	 "     imageno;         %d \n"       
	 "     nscans;          %d \n"        
	 "     imagesize;       %d \n"       
	 "     thisscan;        %d \n"       
	 "     eoi;             %d \n"        
	 "     station[25];     %s \n"       
	 "     stnid;           %d \n"        
	 "     timed;           %d \n"        
	 "     unix_time;       %d \n"     
	 "     vers;            %f \n"      
	 "     startrange;      %d \n"        
	 "     endrange;        %d \n"       
	 "     rngres;          %d \n"       
	 "     angres;          %f \n"      
	 "     vidres;          %d \n"        
	 "     pass;            %d \n"        
	 "     product;         %s \n"       
	 "     imgfmt[15];      %s \n"       
	 "     elev;            %f \n"      
	 "     dbzcor;          %f \n"      
	 "     numdbmlvls;      %d \n"        
	 "     nazimuths;       %d \n"        
	 "     nfields;         %d \n"        
	 "     scantype;        %d \n"        
	 "     nyq;             %f \n"      
	 "    	field;          %d \n"
	 "     lassenfield;     %d \n"        
	 "     numgates;        %d \n"        
	 "     timestamp[25];   %s \n"       
	 "     latitude;        %f \n"      
	 "     longitude;       %f \n"      
	 "     height;          %f \n"      
	 "     frequency;       %f \n"      
	 "     prf;             %f \n"      
	 "     pulselength;     %f \n"      
	 "     anglerate;       %f \n"      
	 "     unfolding[20];   %s \n",
	 ppivol.scans->seqno,         
	 int(ppivol.scans->imageno),       
	 ppivol.scans->nscans,        
	 int(ppivol.scans->imagesize),     
	 int(ppivol.scans->thisscan),      
	 ppivol.scans->eoi,           
	 ppivol.scans->station,   
	 ppivol.scans->stnid,         
	 ppivol.scans->timed,         
	 int(ppivol.scans->unix_time),     
	 ppivol.scans->vers,          
	 ppivol.scans->startrange,    
	 int(ppivol.scans->endrange),      
	 int(ppivol.scans->rngres),        
	 ppivol.scans->angres,        
	 ppivol.scans->vidres,        
	 ppivol.scans->pass,          
	 ppivol.scans->product,   
	 ppivol.scans->imgfmt,    
	 ppivol.scans->elev,          
	 ppivol.scans->dbzcor,        
	 ppivol.scans->numdbmlvls,    
	 ppivol.scans->nazimuths,     
	 ppivol.scans->nfields,       
	 ppivol.scans->scantype,      
	 ppivol.scans->nyq,           
	 ppivol.scans-> field,        
	 ppivol.scans->lassenfield,   
	 ppivol.scans->numgates,      
	 ppivol.scans->timestamp, 
	 ppivol.scans->latitude,      
	 ppivol.scans->longitude,     
	 ppivol.scans->height,        
	 ppivol.scans->frequency,     
	 ppivol.scans->prf,           
	 ppivol.scans->pulselength,   
	 ppivol.scans->anglerate,     
	 ppivol.scans->unfolding);
}

