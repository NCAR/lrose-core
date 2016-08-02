#undef DEBUGMAIN
// #define DEBUGMAIN

/*

db.c - 3d-Rapic database code

The term IMAGE should more correctly be SCANSET, it has no connection with
rdr_scan, which is a convenience for encapsulating scan sets for display.

The database will consist of data files of non circular raw radar
data streams, with a separate ISAM index file which contains
IMAGE parameters, which include references to the main data file.

Due to the requirement of data files being continuously available for
writing, and data files being periodically backed-up, the db system
will utilise a leap-frogging write file system, with a working file
available for writing,  and a read-only archive file available for
writing to tape and recent data access.

A global index file will be maintained which references both the 
working and archive data files, as well as individual index files 
for each database.

The working data file will grow to a given size (e.g. 50MB) before
being switched to the archive role, with a new working database being
created. The index of the working file could be copied to become the
new global index.
	
*** POTENTIAL PROBLEMS WITH THIS APPROACH AND MULTI USER ?????
SLAVE DISPLAYS COULD OPEN/CLOSE DB ON EACH ACCESS, PERIODICALLY
REFRESHING THEIR STATE.


Radar image database
IMAGE Parameters of relevence:
Central stnid (2),		**lsbyte stnid,  msbyte cntryid
Image DateTime time_t (4),
Scan Type (2), Set Angle (2), Data Type (2), Data Format (2),
Short Filename (16), Offset (4), Size (4),
Spare space (12)

Each image in the data file will be consist of an image header which contains
image and scan details and offsets/sizes.

@5 min. scans, approx 300 images/day
ie 20k image index per day
A scan index file on a regional basis could consist of scans from up to
16 radars, every 5 mins
ie approx 5000 scans/day, or 340k of index data/day
This will be < 10% of the actual data

Alternatively, a scan summary could be stored at the start of each image
in the main data file.

SCAN Parameters of relevence:
StnID (2/3), DateTime (4/9), Scan Type (1), Set Angle/NumScans (2/4),
Data Type (1/1)
Store per scan data in 10/18 bytes


An image will be stored with a header which details the no. of scans,
the component scan's params, their file ofs/sz and the total scan size.
These details should be designed so as to not disturb the normal 
Rapic data readers.
In this case, these files could be read as data streams straight 
into the volumetric display, without the database infrastructure.
A unique prefix would satisfy this requirement, let's say "/"
eg
/NUMSCANS: 23
/SCAN 1: stnid yymmddhhmm scantype aaa.a vidtype offset size
/SCAN......
/TOTALSIZE: 123456
/NEXTIMG: 12345678
STNID:
*/

/*
  LOCKING POLICY -
  The Rapic isam database will only add records.
  ie - NO UPDATES
  HOWEVER - Write locks will be used to allow multiple
  Rapics to add to the database.
  THIS REQUIRES EACH RAPIC TO LOCK THE DATA FILE FIRST,
  ADD THE NEW DATA, THEN LOCK THE ISAM, ADD THE RECORD, UNLOCK THE
  ISAM, THEN UNLOCK THE DATA FILE.
  IF ALL RAPICS USE THIS APPROACH, THE FIRST TO GET THE LOCK
  ON THE DATA FILE WILL LOCK ALL OTHERS OUT UNTIL FINISHED.
  TRANSACTIONS WILL BE ATOMIC.
*/

/*
  scan_rec is a structure for representing image/scan records in
  CISAM databases
  The CISAM database is used to maintain an index into
  the actual scan data which is stored in separate files.
  Sets of scans will be stored,	but individual scans are also supported.
*/


#include "rpdb.h"
#ifndef NO_XWIN_GUI
#include "displmng.h"
#include "busy.h"
#endif
#include "rdrutils.h"
#include "utils.h"
#include "siteinfo.h"
#ifdef TITAN
#include "rapictotitan.h"
#include "titanclient.h"
#endif
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "log.h"
#include <libgen.h>

#ifdef aix
#define abs fabsf
#endif

DBMng *DBMngr = NULL;

// put stn part of database name at least significant end
// Also put _ before stnid to allow easy script testing for new style
bool RPDBNAME_STN_LS = true;


int RpIsamStateCount = 0;

char rapic_data_path[256] = "./";
char latestDataInfoFilename[] = "_latest_data_info";
char fileListName[] = "_file_list";
char fileListTime[] = "_file_list_time";
char volFileSuffix[64] = ".VOL";
char rpdbinifile1[64] = "rpdb.ini";
char rpdbinifile2[64] = "rpdb_path";


int DBLocalTime = 0;

// the following default values may be overriden inn rpdb.ini by
// maxLoadRealTimeHours=24
// maxLoadRealTimeGap=2
// maxLoadDBGap=2
int maxLoadRealTimeSecs = 12 * 3600;  // limit load realtime to 12hours
int maxLoadRealTimeGap = 2 * 3600;    // limit gap in load realtime data to 2 hours
int maxLoadDBGap = 2 * 3600;          // limit gap in load database to 2 hours

time_t CorrectDBLocalTime(int localtime = -1) {
  if (localtime); //dummy
  return 0;
}

/*
  CTREE is used in a multi-user incremental ISAM mode, to allow
  the application to control the database files.
*/

ISEG rp_db_stnseg[6] = {
  { 2, 2, INTSEG },
  { 4, 4, INTSEG },
  { 8, 2, INTSEG },
  {10, 2, INTSEG },
  {12, 2, INTSEG },
  {14, 2, INTSEG }
};

ISEG rp_db_dttmseg[6] = {
  { 4, 4, INTSEG },
  { 2, 2, INTSEG },
  { 8, 2, INTSEG },
  {10, 2, INTSEG },
  {12, 2, INTSEG },
  {14, 2, INTSEG }
};

DATOBJ rp_rec_doda[] = {
  { "DeleteFlag", NULL, CT_INT2U}, 
  { "StnID", NULL, CT_INT2U}, 
  { "DateTime", NULL, CT_INT4}, 
  { "ScanType", NULL, CT_INT2U}, 
  { "ScanAngle", NULL, CT_INT2}, 
  { "DataType", NULL, CT_INT2U}, 
  { "DataFmt", NULL, CT_INT2U}, 
  { "FileName", NULL, CT_STRING, 16}, 
  { "Offset", NULL, CT_INT4U}, 
  { "Size", NULL, CT_INT4U}, 
  { "UNUSED", NULL, CT_STRING, 12}
};  

#define KEYSIZE sizeof(time_t)+(4*sizeof(short))+sizeof(rdr_angle)
IIDX rp_db_idx[2] = {
  { KEYSIZE,        // key length
    12,             // 0-fixed len, 4-ld ch comp, 8 trl ch comp, 12 both comp
    0,              // no dup. keys
    0,              // no null key checking
    32,             // null char
    6,              // no of key segments
    rp_db_stnseg,   // stn/date/time/angle key seg data
    "stn_key"},     // index name
  { KEYSIZE,        // key length
    12,             // 0-fixed len, 4-ld ch comp, 8 trl ch comp, 12 both comp
    0,              // no dup. keys
    0,              // no null key checking
    32,             // null char
    6,              // no of key segments
    rp_db_dttmseg,  // date/stn/time/angle key seg data
    "dttm_key"}    // index name
};


COUNT rp_db_readonly_filmod = READFIL;
//COUNT rp_db_readonly_filmod = SHARED;

IFIL rp_db_ifil = {
  "",               // filled in later
  -1,               // data file no. - let OPEN/CREATE assign it
  sizeof(scan_rec),  // data record length
  4096,             // data extension size (incremental growth step)
  //    SHARED | PERMANENT | WRITETHRU, // default file mode
  SHARED | PERMANENT | WRITETHRU, // default file mode
  2,                // no. of indices
  4096,             // index extension size (incremental growth step)
  //    SHARED | PERMANENT | WRITETHRU, // default index mode
  SHARED | PERMANENT | WRITETHRU, // default index mode
  rp_db_idx,        // IIDX DATA
  "",               // r-tree data
  ""                // r-tree data
  // null: tfilno
};

/* set user and group read write permission, others read only */
LONG IFILEPERMISSIONS = OPF_ALL | GPF_WRITE | WPF_READ;
mode_t DFILEPERMISSIONS = 0664;

scan_rec::scan_rec(rdr_scan *scan) {
  memset(this, 0, sizeof(scan_rec));
  if (scan) SetScanRec(scan);
}

/*
  void scan_rec::SetScanRec(rdr_scan *scan) {
  rdr_scan *tmpscan;
    
  if (!scan) return;
  if (scan->FirstScan()) stnid = scan->FirstScan()->station;
  datetime = scan->FirstTm;
  scantype = IMAGE;
  setangle = scan->scanSetCount();
  tmpscan = scan->FirstScan();
  if (tmpscan) {
  data_type = tmpscan->data_type;
  data_fmt = tmpscan->data_fmt;
  }
  }

  void scan_rec::GetScanRec(rdr_scan *scan) {
  //	scan->station = stnid;
  scan->FirstTm = datetime;
  scan->num_scans = setangle;
  }
*/

void scan_rec::SetScanRec(rdr_scan *scan) {
    
  if (!scan) return;
  stnid = scan->station;
  datetime = scan->FirstTime();
  scan_type = scan->scan_type;
  if (scantype() == VOL) setangle = scan->scanSetCount();
  else setangle = scan->set_angle;
  data_type = scan->data_type;
  data_fmt = scan->data_fmt;
  size = scan->scanSetSize();
}

void scan_rec::GetScanRec(rdr_scan *scan) {
  scan->station = stnid;
  scan->scan_time_t = datetime;
  scan->set_angle = setangle;
  scan->scan_type = scantype();
  scan->data_type = e_data_type(data_type);
  scan->data_fmt = e_data_fmt(data_fmt);
  scan->setDataSize(size);
}

void scan_rec::PrintRec(FILE *file) {
  char	temp1[32],temp2[32];
    
  if (!file) return;
  UnixTime2DateTimeStr(datetime,DBLocalTime, temp1,temp2);
  fprintf(file,"%s(%d) %s %s\n", 
	  StnRec[stnid].Name, stnid,temp1,temp2);
  fprintf(file,"type=%s, ",scan_type_text[scan_type]);
  if ((scantype() == IMAGE) || (scantype() == VOL))
    fprintf(file,"scans=%d",setangle);
  else 
    fprintf(file,"angle=%1.1f", setangle/10.);
  fprintf(file," data=%s, format=%s, size=%d ofs=%ld\n",get_data_type_text((e_data_type)data_type),
	  data_fmt_text[data_fmt], size, offset);
  //    fprintf(file,"%s, ofs=%d, size=%d\n",filename,offset,size);
}

void scan_rec::PrintRecShort(FILE *file, 
			     bool sizefirst) {
  char	temp1[32],temp2[32];
    
  UnixTime2DateTimeStr(datetime,DBLocalTime, temp1,temp2);
  if (sizefirst)
    fprintf(file,"%d, ",size);
  fprintf(file,"%s, %s %s, ", 
	  StnRec[stnid].Name, temp1,temp2);
  fprintf(file,"%s, ",scan_type_text[scan_type]);
  if ((scantype() == IMAGE) || (scantype() == VOL))
    fprintf(file,"%d ",setangle);
  else 
    fprintf(file,"angle=%1.1f", setangle/10.);
  if (!sizefirst)
    fprintf(file,", %d\n",size);
  else
    fprintf(file,"\n");
    
  //    fprintf(file,"%s, ofs=%d, size=%d\n",filename,offset,size);
}

void scan_rec::StnDateTimeString(char *retstr) {
  char	temp1[64], temp2[64];
    
  UnixTime2DateTimeStr(datetime,DBLocalTime, temp1,temp2);
  sprintf(retstr,"%s(%d) %s %s", 
	  stn_name(stnid), stnid, temp1, temp2);
}

void scan_rec::RecString(char *retstr) {
  char	temp1[64], temp2[64], temp3[256];
    
  UnixTime2DateTimeStr(datetime,DBLocalTime, temp1,temp2);
  sprintf(temp3,"%s(%d) %s %s\ntype=%s, ", 
	  StnRec[stnid].Name, stnid, temp1, temp2, scan_type_text[scan_type]);
  if ((scantype() == IMAGE) || (scantype() == VOL))
    sprintf(temp1,"scans=%d",setangle);
  else 
    sprintf(temp1,"angle=%1.1f", setangle/10.);
  sprintf(retstr,"%s%s datatype=%s, dataformat=%s\n",
	  temp3, temp1, 
	  get_data_type_text((e_data_type)data_type),
	  data_fmt_text[data_fmt]);
  //    fprintf(file,"%s, ofs=%d, size=%d\n",filename,offset,size);
}

void scan_rec::CopyRec(scan_rec *destrec)	{	// copy this rec to dest.
  memcpy(destrec,this,sizeof(scan_rec));
}

/*
  #include "ct_include/ctfunp.h"

  void scan_rec::Swap() {
  #ifdef linux
  revbyt(&del_flag, 2);  // swap del_flag, stnid
  revwrd(&datetime, 1);  // word swap datetime
  revbyt(&datetime, 6);  // then byte swap datetime, scantype, setangle, data_type, data_fmt
  revwrd(&offset, 2);    // word swap offset, size
  revbyt(&offset, 4);    // then byte swap them
  #endif
  }
*/

rp_isam_state::rp_isam_state() {
  memset(this, 0, sizeof(rp_isam_state));
  RpIsamStateCount++;
}
    
rp_isam_state::~rp_isam_state() {
  if (next) next->prev = prev;
  if (prev) prev->next = next;
  RpIsamStateCount--;
}

void rp_isam_state::SetState(COUNT currentidx, 
			     scan_rec *currentrec, 
			     rp_key stnkey, 
			     rp_key dttmkey) {
  CurrentIdx = currentidx;
  memcpy(&CurrentRec, currentrec, sizeof(scan_rec));
  memcpy(StnKey, stnkey, sizeof(rp_key));
  memcpy(DttmKey, dttmkey, sizeof(rp_key));
}

void rp_isam_state::GetState(COUNT *currentidx, 
			     scan_rec *currentrec, 
			     rp_key stnkey, 
			     rp_key dttmkey) {
  *currentidx = CurrentIdx;
  memcpy(currentrec, &CurrentRec, sizeof(scan_rec));
  memcpy(stnkey, StnKey, sizeof(rp_key));
  memcpy(dttmkey, DttmKey, sizeof(rp_key));
}

void RemoveDatSuffix(char *instr) {
  int  l, lsuffix;
  StripTrailingWhite(instr);
  // unsafe
  //if (strstr(instr+strlen(instr)-9,".i386.dat")) instr[strlen(instr)-9] = 0;
  //if (strstr(instr+strlen(instr)-4,".dat")) instr[strlen(instr)-4] = 0;
  if (strstr(instr, ".i386.dat")!=NULL) {
    l = strlen(instr);
    lsuffix = strlen(".i386.dat");
    instr[l - lsuffix] = '\0'; // null terminate and strip suffix
  }
  else if (strstr(instr, ".dat")!=NULL) {
    l = strlen(instr);
    lsuffix = strlen(".dat");
    instr[l - lsuffix] = '\0'; // null terminate and strip suffix
  }
}

void rp_isam::PushState() {

  if (!StateStackTop)		    // stack empty, point to first
    StateStackTop = StateStack;
  else if (StateStackTop->next)   // use next stack entry 
    StateStackTop = StateStackTop->next;
  else {			    // no more stack entries, create new
    StateStackTop->next = new rp_isam_state();
    StateStackTop = StateStackTop->next;
  }
  MakeRecStnKey(&CurrentRec);
  MakeRecDttmKey(&CurrentRec);
  StateStackTop->SetState(CurrentIdx, &CurrentRec, 
			  stnkey,dttmkey);
}

void rp_isam::PopState() {
  if (!StateStackTop) {
    fprintf(stderr, "rp_isam::PopState ERROR, StateStackTop = 0\n");
    return; 
  }
  StateStackTop->GetState(&CurrentIdx, &CurrentRec,  
			  stnkey,dttmkey);
  if (CurrentIdx == dttmidx)
    GetRecord(CurrentIdx, dttmkey, &CurrentRec);
  else if (CurrentIdx == stnidx)
    GetRecord(CurrentIdx, stnkey, &CurrentRec);
  StateStackTop = StateStackTop->prev;
}
    
rp_isam::rp_isam(bool rdonly) {
  datafd = dfil = stnidx = dttmidx  = -1;
  CurrentIdx = NOIDX;
  memcpy(&rp_ifil,&rp_db_ifil,sizeof(IFIL));	
  // initial IFIL, refs IIDX etc, no name, dfilno=-1
  memset(fname, 0, sizeof(fname));
  //	tempscan = 0;
  tempscan = 0;
  datasize = 0;
  srcdb = 0;
  destdb = 0;
  useProgressDialog = true;
  lastProgressPercent = -1;
#ifndef NO_XWIN_GUI
  Viewer = 0;
#endif
  first_tm = last_tm = 0;
  USELOCKS = FALSE;
  dfilelocktmout = 20;
  ReadOnly = rdonly;
  StateStack = new rp_isam_state();	// initial State Stack entry
  StateStackTop = 0;
  showDBScansToScanMng = true;
}
    


rp_isam::~rp_isam() {
  Close();
#ifndef NO_XWIN_GUI
  PreviewScan();               // if a Preview set, release it
#endif
  if (tempscan) {
    if (tempscan->ShouldDelete(this, "rp_isam::~rp_isam"))
      delete tempscan;
  }
  while (StateStack) {
    StateStackTop = StateStack->next;
    delete StateStack;
    StateStack = StateStackTop;
  }
  if (srcdb) delete srcdb;
  if (destdb) delete destdb;
};

scan_rec* rp_isam::GetCurrentRec() {
  return &CurrentRec;
}

void rp_isam::setIDXMode(idxtype IdxType) 
{
  if (IdxType == STN) CurrentIdx = stnidx;	
  if (IdxType == DTTM) CurrentIdx = dttmidx;	// default NOIDX=no change
}

bool rp_isam::Open(const char *pathnm, const char *filenm, 
		   bool RdOnly, bool AllowRdOnlyRebuild) {
  char tempstr1[256], tempstr2[256];    

  if (!ReadOnly)          // if ReadOnly already set, keep it
    ReadOnly = RdOnly;
    
  if (filenm)
    strcpy(fname,filenm);
  if (pathnm)
    strcpy(pname,pathnm);
  RemoveDatSuffix(fname);		// file browser uses .dat filter. Remove it if present
  RemoveDatSuffix(pname);		// file browser uses .dat filter. Remove it if present
  strcpy(fullname,pname);
  strcat(fullname,fname);
  strcpy(label_from_to, "");
  // try to open data file
  if (datafd != -1) {
    close(datafd);		// close existing datafd
    datafd = -1;
  }
  if (!ReadOnly)
    datafd = open(fullname,O_RDWR);
  else
    datafd = open(fullname,O_RDONLY);
  if ((datafd == -1) && !ReadOnly) {
    fprintf(stderr,"\nrp_isam::Open Couldn't open datafd %s as Read/Write\n",fullname);
    fprintf(stderr,"rp_isam::Open Attempting to open as Read-Only\n");
    datafd = open(fullname,O_RDONLY);
    if (datafd != -1) {
      ReadOnly = true;
      fprintf(stderr,"rp_isam::Open %s OPENED AS READ-ONLY OK\n",fullname);
    }
  }
  if (datafd == -1) {
    fprintf(stderr,"\nrp_isam::Open Couldn't open datafd %s\n",fullname);
    perror(0);
    return FALSE;			// if file not present, quit
  }
  /*	if (!RdHeader()) {									// RdHeader should set datasize
	fprintf(stderr,"rp_isam::Open Couldn't RdHeader - Writing new header\n"); */
  datasize = lseek(datafd,0,SEEK_END);// seek to end to get size
  /*		if (datasize < sizeof(datafile_header))
		datasize = sizeof(datafile_header);	// min size after WrtHeader
		if (!LockDFileRetry(20)) 							// write valid file size
		WrtHeader();
		LockDFile(FALSE);	
		} */
  if (!OpenIsam(pname,fname, ReadOnly))	{	// if data file opened, not ISAM
    bool result = false;
    if (ReadOnly)
      {
	if (AllowRdOnlyRebuild)
	  {
	    fprintf(stderr,"rp_isam::Open OpenIsam Failed on %s, Disabling ReadOnly and attempting Rebuild\n",fullname);
	    ReadOnly = false; 
	  }
      }
    else
      fprintf(stderr,"rp_isam::OpenIsam Failed on %s, Attempting Rebuild\n",fullname);
    if (!ReadOnly)
      result = Rebuild(BUILDNEWISAM);	// try to reconstruct ISAM
    if (!result) {
      close(datafd);
      datafd = -1;
      fprintf(stderr,"\nrp_isam::OpenIsam and Rebuild Failed on %s\n",fullname);
      return result;
    }
  }
  if (FirstRec(DTTM))
    {
      first_tm = CurrentRec.datetime;
      CurrentRec.StnDateTimeString(tempstr1);
    }
  else
    strcpy(tempstr1,"FirstRec Failed");
  if (LastRec(DTTM))
    {
      last_tm = CurrentRec.datetime;
      CurrentRec.StnDateTimeString(tempstr2);
    }
  else
    strcpy(tempstr2,"LastRec Failed");
  sprintf(label_from_to, "From %s To %s", tempstr1, tempstr2);
  LastRecord(CurrentIdx,&CurrentRec);	// get latest record
  //	lseek(datafd,sizeof(datafile_header),SEEK_SET);	// seek to start of data
  lseek(datafd,0,SEEK_SET);						// seek to start of data
  return TRUE;
}

void rp_isam::addIDXSuffix(char *dbname)
{
#ifdef __i386__
  strcat(dbname,".i386.idx");
#else
  strcat(dbname,".idx");
#endif
}

void rp_isam::addDATSuffix(char *dbname)
{
#ifdef __i386__
  strcat(dbname,".i386.dat");
#else
  strcat(dbname,".dat");
#endif
}

bool rp_isam::OpenIsam(const char *pathnm, const char *filenm, bool RdOnly) {
  char	temp[136];
  int result;
  if (filenm && fname!=filenm)
    strcpy(fname,filenm);
  if (pathnm && pname!=pathnm)
    strcpy(pname,pathnm);
  RemoveDatSuffix(fname); // file browser uses .dat filter. Remove if present
  RemoveDatSuffix(pname); // file browser uses .dat filter. Remove if present
  strcpy(fullname,pname);
  strcat(fullname,fname);
  strcpy(fullname_isam,fullname);
#ifdef __i386__
  strcat(fullname_isam, ".i386"); 
#endif
  // try to open isam
  rp_ifil.pfilnam = fullname_isam;  // point to name
  if (dfil != -1) {
    CloseIFile(&rp_ifil); //close open IFIL
    dfil = stnidx = dttmidx = -1;
  }
  strcpy(temp,fullname);
  addIDXSuffix(temp);

  // NOTE: RdOnly flag is passed by caller, and is treated as a hard
  // read-only directive, 
  // If RdOnly is false, and the file exists but is not writeable it will
  // be set to true to try opening the file read-only, 
  // but cannot be set to false if it has been set true
  //
  // The ReadOnly flag is a class member variable
  // This will be set false if it is true and the file doesn't exist 
  // to allow it to try creating the file

  if (FileExists(temp))
    {
      if (!RdOnly && ((result = chmod(temp, DFILEPERMISSIONS)) == -1))
	{
	  fprintf(stderr, 
		  "rp_isam::OpenIsam WARNING - chmod(%s, %o) failed - ", 
		  temp, DFILEPERMISSIONS); 
	  perror("");	// set permissions
	  if ((result == EPERM) || 
	       (result == EROFS))
	    {
	      fprintf(stderr, 
		      "rp_isam::OpenIsam - No write access to %s"
		      ", trying read-only open\n", 
		      temp);
	      RdOnly = true;
	    }
	}
    }
  else // file doesn't exist
    {
      if (!RdOnly)   
	{
	  if (ReadOnly) // allow attempt to create 
	    {
	      fprintf(stderr, 
		      "rp_isam::OpenIsam - %s doesn't exist and "
		      "ReadOnly is set, turning off ReadOnly to create\n", 
		      temp);
	      ReadOnly = false;  // if index file not present, try to create it
	    }
	}
      else  // don't allow create attempt
	{
	  fprintf(stderr, 
		  "rp_isam::OpenIsam - Failed - %s doesn't exist and "
		  "read-only mode is forced\n", 
		  temp);
	  return false;
	}
    }
  strcpy(temp,fullname);
  addDATSuffix(temp);
  if (FileExists(temp))
    {
      if (!RdOnly && ((result = chmod(temp, DFILEPERMISSIONS)) == -1))
	{
	  fprintf(stderr, 
		  "rp_isam::OpenIsam WARNING - chmod(%s, %o) failed - ", 
		  temp, DFILEPERMISSIONS); 
	  perror("");	// set permissions
	  if (!RdOnly &&
	      ((result == EPERM) || 
	       (result == EROFS)))
	    {
	      fprintf(stderr, 
		      "rp_isam::OpenIsam - No write access to %s"
		      ", trying read-only open\n", 
		      temp);
	      RdOnly = true;
	    }
	}
    }
  else // file doesn't exist
    {
      if (!RdOnly)   
	{
	  if (ReadOnly) // allow attempt to create 
	    {
	      fprintf(stderr, 
		      "rp_isam::OpenIsam - %s doesn't exist and "
		      "ReadOnly is set, turning off ReadOnly to create\n", 
		      temp);
	      ReadOnly = false;  // if index file not present, try to create it
	    }
	}
      else  // don't allow create attempt
	{
	  fprintf(stderr, 
		  "rp_isam::OpenIsam - Failed - %s doesn't exist and "
		  "read-only mode is forced\n", 
		  temp);
	  return false;
	}
    }
  if (RdOnly) 
    rp_ifil.dfilmod = rp_ifil.ifilmod = rp_db_readonly_filmod;
  if (OpenIFile(&rp_ifil)) {
    fprintf(stderr,"\nrp_isam::OpenIsam Couldn't open ifil %s (%d, %d)\n",
	    fullname,isam_err,isam_fil);
    dfil = stnidx = dttmidx  = -1;
    return FALSE;	
  }  // try to reconstruct ISAM
  dfil = rp_ifil.tfilno;      // get file no for data file
  stnidx = dfil+1;	      // 1st index - filno = dfil+1
  dttmidx = dfil+2;	      // 2nd index - filno = dfil+2
  CurrentIdx = dttmidx;	      // default to dttm index
  return TRUE;
}

char headerid_str[] = "datafile_header";

bool rp_isam::WrtHeader() {
  datafile_header temp;
  if (datafd < 0) {
    fprintf(stderr,"rp_isam::WrtHeader ERROR called for datafd = %d\n",datafd);
    return FALSE;
  }
  memset(&temp, 0, sizeof(temp));
  strncpy(temp.headerid,headerid_str,31);
  temp.filesize = datasize;
  temp.lastupdated = time(0);
  if (lseek(datafd,0,SEEK_SET) < 0) {
    perror("rp_isam::WrtHeader Seek to start failed - ");
    return FALSE;
  }
  if (write(datafd,&temp,sizeof(temp)) != sizeof(temp)) {
    perror("rp_isam::WrtHeader Write failed - ");
    return FALSE;
  }
  return TRUE;
}

bool rp_isam::RdHeader(datafile_header *header) {
  datafile_header temp;
  if (datafd < 0) {
    fprintf(stderr,"rp_isam::RdHeader called for datafd = -1\n");
    return FALSE;
  }
  if (lseek(datafd,0,SEEK_SET) < 0) {
    perror("rp_isam::RdHeader Seek to start failed - ");
    return FALSE;
  }
  if (header) {
    if (read(datafd,header,sizeof(temp)) != sizeof(temp)) {
      perror("rp_isam::RdHeader failed to read header - ");
      return FALSE;
    }
    else return (strstr(header->headerid,headerid_str) != 0);
  }
  else {						// if no header specified, just get datasize
    if (read(datafd,&temp,sizeof(temp)) != sizeof(temp)) {
      perror("rp_isam::RdHeader failed to read header - ");
      return FALSE;
    }
    else {
      if (strstr(temp.headerid,headerid_str)) {
	datasize = temp.filesize;
	return TRUE;
      }
      else {
	fprintf(stderr,"rp_isam::RdHeader Did not match headerid\n");
	return FALSE;
      }
    }
  }
}
	
bool rp_isam::Create(const char *pathnm, const char *filenm) {
  int result;
    
  if (filenm)
    strcpy(fname,filenm);
  if (pathnm)
    strcpy(pname,pathnm);
  RemoveDatSuffix(fname);		// file browser uses .dat filter. Remove it if present
  strcpy(fullname,pname);
  strcat(fullname,fname);
  // try to create data file
  if (datafd != -1) close(datafd);    // close existing datafd
  datafd = open(fullname,O_RDWR+O_CREAT+O_EXCL,DFILEPERMISSIONS);
  if ((result = chmod(fullname, DFILEPERMISSIONS)) == -1) {
    fprintf(stderr, "rp_isam::Create chmod(%s, %o) failed - ", fullname, DFILEPERMISSIONS); 
    perror("");	// set permissions
  }
  datasize = 0;		// this will be the init file sz
  if ((datafd == -1) && (errno == EEXIST))
    return Open(pathnm,filenm);				// if already exists, open it
  if (datafd == -1) {
    perror("rp_isam::Create ERROR Creating datafd - ");
    return FALSE;     // if unable to open file, quit
  }
  /*	LockDFileRetry(20);	// lock data file
	datasize = sizeof(datafile_header);		// this will be the init file sz
	WrtHeader();						// write header record to datafd
	LockDFile(FALSE);		// unlock data file */
  return CreateIsam(pathnm,filenm);
}

bool rp_isam::CreateIsam(const char *pathnm, const char *filenm) {
    
  if (filenm)
    strcpy(fname,filenm);
  if (pathnm)
    strcpy(pname,pathnm);
  strcpy(fullname,pname);
  strcat(fullname,fname);
  strcpy(fullname_isam,fullname);
#ifdef __i386__
  strcat(fullname_isam, ".i386"); 
#endif
  // try to create isam
  rp_ifil.pfilnam = fullname_isam;						// point to name
  if (dfil != -1) {
    if (CloseIFile(&rp_ifil))	//close open IFIL
      fprintf(stderr,"rp_db::CreateIsam - ERROR Closing IFile\n");
    dfil = stnidx = dttmidx  = -1;
  }
  if (CreateIFileXtd(&rp_ifil, NULL, NULL, IFILEPERMISSIONS, NULL, NULL)) {
    fprintf(stderr,"rp_isam::Create Couldn't create ifil %s (%d, %d)\n",
	    fullname,isam_err,isam_fil);
    dfil = stnidx = dttmidx  = -1;
    close(datafd);
    return FALSE;
  }
  dfil = rp_ifil.tfilno;	// get file no for data file
  stnidx = dfil+1;		// 1st index - filno = dfil+1
  dttmidx = dfil+2;		// 2nd index - filno = dfil+2
  CurrentIdx = dttmidx;	// default to dttm index
  return TRUE;
}

bool rp_isam::Rebuild(bool ForceNewISAM) {
  char	temp[136];
  bool	DeleteFaulty = FALSE;
  bool	MakeNewISAM = ForceNewISAM;
    
  if (ReadOnly) {
    fprintf(stderr,"rp_db::CreateIsam - ERROR Cannot Rebuild,  Database is READ-ONLY\n");
    return false;
  }
  if (dfil != -1) {
    if (CloseIFile(&rp_ifil))	//close open IFIL
      fprintf(stderr,"rp_db::Rebuild - ERROR Invalid file handle -  Closing IFile\n");
    dfil = stnidx = dttmidx  = -1;
  }
  if (!RebuildIFile(&rp_ifil)) {
    if (OpenIFile(&rp_ifil)) {
      fprintf(stderr,"rp_isam::Rebuild Couldn't re-open ifil %s (%d, %d) after successful rebuild\n",
	      fullname,isam_err,isam_fil);
      dfil = stnidx = dttmidx  = -1;
      return FALSE;
    }																	// try to reconstruct ISAM
    else 
      {
	dfil = rp_ifil.tfilno;	// get file no for data file
	stnidx = dfil+1;		// 1st index - filno = dfil+1
	dttmidx = dfil+2;		// 2nd index - filno = dfil+2
	CurrentIdx = dttmidx;	// default to dttm index
      }
    fprintf(stderr, "rp_isam::Rebuild - OK\n");
    return TRUE;
  }
  else {
    fprintf(stderr, "rp_isam::Rebuild - FAILED error=%d\n", isam_err);
    if (DeleteFaulty) 
      fprintf(stderr,"DELETING DATABASE\n");
    else 
      fprintf(stderr, "Rebuilding ISAM from Data File: %s\n", fullname);
    //		fprintf(stderr, "Closing and de-referencing database: %s\n", fullname);
    if (datafd != -1 && !MakeNewISAM) {
      close(datafd);		// close existing datafd
      datafd = -1;
    }
    if (DeleteFaulty) {
      strcpy(temp,fullname);
      unlink(temp);
    }
    if (dfil != -1) {
      CloseIFile(&rp_ifil);	//close open IFIL
      dfil = stnidx = dttmidx  = -1;
    }
    if (DeleteFaulty || MakeNewISAM) {
      strcpy(temp,fullname);
      addIDXSuffix(temp);
      unlink(temp);
      strcpy(temp,fullname);
      addDATSuffix(temp);
      unlink(temp);
    }
    if (MakeNewISAM) {
      if (!datafd) {
	fprintf(stderr, "rp_isam::Rebuild ERROR Unable to make new ISAM,  Main data file not open\n");
	return FALSE;
      }
      CreateIsam();
      ReadFile(datafd, fullname, ADDISAMONLY);
      return TRUE;
    }
    else
      return FALSE;
  }
}

void rp_isam::Close() {
  if (datafd != -1) close(datafd);
  datafd = -1;
  if (dfil != -1) CloseIFile(&rp_ifil);
  //  memcpy(&rp_ifil,&rp_db_ifil,sizeof(IFIL));	
  // initial IFIL, refs IIDX etc, no name, dfilno=-1
  dfil = stnidx = dttmidx = CurrentIdx = -1;
  // memset(fullname, 0, sizeof(fullname));
  // memset(pname, 0, sizeof(pname));
  // memset(fname, 0, sizeof(fname));
}

bool rp_isam::IsOpen() {
  return (dfil != -1);
}

LONG rp_isam::RecCount() {
  LONG count;
  if (!IsOpen()) return 0;
  count = NbrOfRecords(dfil);
  return count;
}

/*
  LOCKING POLICY -
  The Rapic isam database will only add records.
  ie - NO UPDATES
  HOWEVER - Write locks will be used to allow multiple
  Rapics to add to the database.
  THIS REQUIRES EACH RAPIC TO LOCK THE DATA FILE FIRST,
  ADD THE NEW DATA, THEN LOCK THE ISAM, ADD THE RECORD, UNLOCK THE
  ISAM, THEN UNLOCK THE DATA FILE.
  IF ALL RAPICS USE THIS APPROACH, THE FIRST TO GET THE LOCK
  ON THE DATA FILE WILL LOCK ALL OTHERS OUT UNTIL FINISHED.
  TRANSACTIONS WILL BE ATOMIC.
			
  The functions lock_rp_isam/unlock_rp_isam should be used 
  for ALL locking operations
*/

int rp_isam::lockdfile(short locktype) {
  flock	lock;

  if ((datafd == -1) || !USELOCKS) return 0;
  lock.l_whence = 0;
  lock.l_start = lock.l_len = 0;	// LOCK WHOLE FILE	
  if ((locktype == F_WRLCK) && ReadOnly) locktype = F_RDLCK; // force F_RDLCK
  lock.l_type = locktype;
  if (fcntl(datafd,F_SETLK,&lock) != -1) return 0;
  else return errno;				
}

int rp_isam::unlockdfile() {
  flock	lock;
  if ((datafd == -1) || !USELOCKS) return 0;
  lock.l_whence = 0;
  lock.l_start = lock.l_len = 0;	// UNLOCK WHOLE FILE	
  lock.l_type = F_UNLCK;
  if (fcntl(datafd,F_SETLK,&lock) != -1) return 0;
  else return errno;				
}

// use tenths of sec retries
int rp_isam::lockdfileretry(int retrytenths,  short locktype) {
  int	temp = -1;
    
  if ((datafd == -1) || !USELOCKS) return 0;
  if (retrytenths < 0) retrytenths = dfilelocktmout;	// use default
  if ((locktype == F_WRLCK) && ReadOnly) locktype = F_RDLCK;
  while ((retrytenths > 0) && (temp = lockdfile(locktype))) {
    sec_delay(retrytenths/10);
    retrytenths--;
  }
  if (temp == -1) lockdfile(F_UNLCK);
  return temp;
}
		
/*
 * LOCK THE ISAM AND DATA FILE OF THIS RP_ISAM
 *
 * LOCKISAM WILL EITHER SUCCEED OR FAIL, RETRY NOT REQD.
 * THIS CALL ONLY KEPT FOR CONVENIENCE
 * 
 * This call will cause subsequent write operations to acquire write locks
 * before modifying records
 */
int rp_isam::lock_rp_isam(rpisam_lockmode lockmode) {
  int	temp = -1;
  COUNT isamlockmode = ENABLE;

  if (!USELOCKS) return 0;
  if (lockmode == RP_LOCK_READ) temp = lockdfileretry(-1, F_RDLCK);
  else temp = lockdfileretry(-1, F_WRLCK);
  if (temp) {
    perror("rp_isam::lock_rp_isam UNABLE TO lockdfileretry - ");
    LockISAM(FREE);
    return temp;
  }
	
  if ((lockmode == RP_LOCK_READ) || ReadOnly) isamlockmode = READREC;
  temp = LockISAM(isamlockmode);
  if (temp) {
    fprintf(stderr,"rp_isam::lock_rp_isam LockISAM(mode=%d) FAILED Error=(%d,%d)\n",isamlockmode, isam_err,isam_fil);
    fprintf(stderr,"rp_isam::lock_rp_isam Using LockISAM(FREE) to free locks\n");
    LockISAM(FREE);
    temp = LockISAM(isamlockmode);
    if (temp) {
      fprintf(stderr,"rp_isam::lock_rp_isam LockISAM(mode=%d) FAILED ***AGAIN*** Error=(%d,%d)\n",isamlockmode, isam_err,isam_fil);
      LockISAM(FREE);
      return temp;
    }
  }
  return temp;
}

int rp_isam::unlock_rp_isam() {
  int	temp = 1;
  COUNT lockmode = FREE;

  if (!USELOCKS) return 0;
  temp = LockISAM(lockmode);
  unlockdfile();
  return temp;
}

bool rp_isam::RecPresent(scan_rec *presentrec) {
  scan_rec temprec;
  MakeRecStnKey(presentrec);
  return (GetRecord(stnidx,&stnkey,&temprec) == NO_ERROR);
}

bool rp_isam::ScanPresent(rdr_scan *presentscan) {
  scan_rec temprec;

  MakeScanStnKey(presentscan);
  return (GetRecord(stnidx,&stnkey,&temprec) == NO_ERROR);
}

		
bool rp_isam::AddRec(scan_rec *Rec) {
  COUNT result = 1;
  int	retries = 5;
  scan_rec local_rec = *Rec;
#ifdef SCANREC_BYTESWAP
  local_rec.byteSwapFields();
#endif
    
  while (retries && result) {

    result = AddRecord(dfil,&local_rec);
    if (result == KDUP_ERR) {
      result = 0;
      fprintf(stderr,"rp_isam::AddRec - Duplicate ignored\n");
    } 
    if (result) {
      fprintf(stderr,"rp_isam::AddRec ERROR File=%s Error=(%d,%d)\n",
	      fullname,isam_err,isam_fil);
      if (isam_err == DLOK_ERR) {
	retries--;
	sec_delay(0.02);			// 20ms delay (1/50th of a sec)
      }
      else retries = 0;
    }
  }
  if (result == 0)
    {
      if (Rec->datetime < first_tm)
	first_tm = Rec->datetime;
      if (Rec->datetime > last_tm)
	last_tm = Rec->datetime;
    }
  return result == 0;	
}

bool rp_isam::AddScan(rdr_scan *addscan) {
  if (ReadOnly) return TRUE;
  else return AddScan1(addscan);
}

bool rp_isam::AddScanIsam(rdr_scan *addscan,char *fn, int ofs, int sz) {
  return AddScan1(addscan,TRUE,fn,ofs,sz);
}

bool rp_isam::AddScan1(rdr_scan *addscan, bool IsamOnly,
		       char *fn, int ofs, int sz) {

  scan_rec local_rec;
  char tempstr[256];
    
  if (!fn) fn = fname;
    
  if (!IsamOnly && (datafd == -1)) {
    fprintf(stderr,"rp_isam::AddScan ERROR - DB NOT OPEN\n");
    addscan->StoredInDB = TRUE; //tried to store, unsuccessful
    return FALSE;
  }
  if (!addscan)  {
    fprintf(stderr,"rp_isam::AddScan ERROR - addscan=0\n");
    return TRUE;	// nothing added - as requested
  }
  if (!addscan->HeaderValid() ||
      (addscan->data_source == PROD_ACCUM))  {
    //	fprintf(stderr,"rp_isam::AddScan WARNING - addscan is PROD_ACC,  NOT ADDED TO DB\n");
    addscan->StoredInDB = TRUE; //tried to store
    return TRUE;	// nothing added - as requested
  }
  temprec.SetScanRec(addscan);
  if (lock_rp_isam()){ 	// FAILED - unable to get write lock
    temprec.PrintRec();
    addscan->StoredInDB = TRUE; //tried to store, unsuccessful
    return FALSE;
  }
  MakeScanStnKey(addscan);
  if (GetRecord(stnidx,&stnkey,&temprec) != INOT_ERR) {
    fprintf(stderr,"rp_isam::AddScan - Not added, Duplicate key - %s\n",
	    addscan->ScanString2(tempstr));
    addscan->StoredInDB = TRUE;
    addscan->DBDuplicate = TRUE;
    temprec.PrintRec(stderr);
    unlock_rp_isam();
    return TRUE;
  }
  if (!IsamOnly) {				// add scan set to data file & ISAM
    off_t tempofs = temprec.offset;
    datasize += addscan->append_scan_set(datafd,&tempofs,&temprec.size);
    temprec.offset = long(tempofs);
    if (strlen(fn) > 15)
      fprintf(stderr,"rp_isam::AddScan1 WARNING filename > 15chars - truncating, THIS REC CANNOT BE USED IN MERGED DB.\n");
    strncpy(temprec.filename,fn, 15);
    if (addscan->write_error)
      {
	perror("rp_isam::AddScan1 FAILED\n");
	return FALSE;
      }
  }
  else {					// add image only to ISAM (used by rebuild)
    temprec.offset = ofs;
    temprec.size = sz;
    if (strlen(basename(fn)) > 15)
      fprintf(stderr,"rp_isam::AddScan1 WARNING filename > 15chars - truncating, THIS REC CANNOT BE USED IN MERGED DB.\n");
    strncpy(temprec.filename,basename(fn),15);
  }
  local_rec = temprec;
#ifdef SCANREC_BYTESWAP
  local_rec.byteSwapFields();
#endif
 
  if (!AddRec(&local_rec)) {
    temprec.PrintRec(stderr);
    unlock_rp_isam();
    addscan->StoredInDB = TRUE; //tried to store, unsuccessful
    return FALSE;		    // flag as stored
  };
  fprintf(stderr,"rp_isam::AddScan - Scan added OK - %s\n",
	  addscan->ScanString2(tempstr));
  addscan->StoredInDB = TRUE;
  addscan->DBDuplicate = FALSE;
  temprec.PrintRec();
  //	WrtHeader();
  unlock_rp_isam();
  return TRUE;
}
	
bool rp_isam::FirstRec(idxtype IdxType = NOIDX) {
  if (IdxType == STN) CurrentIdx = stnidx;	
  if (IdxType == DTTM) CurrentIdx = dttmidx;	// default NOIDX=no change
  if (FirstRecord(CurrentIdx,&CurrentRec)) {
    fprintf(stderr,"rp_isam::FirstRec failed (%d, %d)\n",isam_err,isam_fil);
    return FALSE;
  }
#ifdef SCANREC_BYTESWAP
  CurrentRec.byteSwapFields();
#endif
  return TRUE;
}
	
time_t rp_isam::FirstRecTime()
{
  if (FirstRec(DTTM))
    return CurrentRec.datetime;
  else
    return 0;
}

bool rp_isam::LastRec(idxtype IdxType = NOIDX) {
  if (IdxType == STN) CurrentIdx = stnidx;	
  if (IdxType == DTTM) CurrentIdx = dttmidx;	// default NOIDX=no change
  if (LastRecord(CurrentIdx,&CurrentRec)) {
    fprintf(stderr,"rp_isam::LastRec failed (%d, %d)\n",isam_err,isam_fil);
    return FALSE;
  }
#ifdef SCANREC_BYTESWAP
  CurrentRec.byteSwapFields();
#endif
  return TRUE;
}

time_t rp_isam::LastRecTime()
{
  if (LastRec(DTTM))
    return CurrentRec.datetime;
  else
    return 0;
}

bool rp_isam::NextRec(bool quiet) {
  if (NextRecord(CurrentIdx,&CurrentRec)) {
    if ((isam_err != INOT_ERR) && !quiet)
      fprintf(stderr,"rp_isam::NextRec failed (%d, %d)\n",isam_err,isam_fil);
    return FALSE;
  }
#ifdef SCANREC_BYTESWAP
  CurrentRec.byteSwapFields();
#endif
  return TRUE;
}
	
bool rp_isam::PrevRec(bool quiet) {
  if (PreviousRecord(CurrentIdx,&CurrentRec)) {
    if ((isam_err != INOT_ERR) && !quiet)
      fprintf(stderr,"rp_isam::PrevRec failed (%d, %d)\n",isam_err,isam_fil);
    return FALSE;
  }
#ifdef SCANREC_BYTESWAP
  CurrentRec.byteSwapFields();
#endif
  return TRUE;
}

bool rp_isam::NextScan(rdr_scan *nextscan) {
  return (NextRec() && GetScanSet(nextscan));
}
	
bool rp_isam::PrevScan(rdr_scan *nextscan) {
  return (PrevRec() && GetScanSet(nextscan));
}
	
/*
  void rp_isam::MakeScanStnKey(rdr_scan *keyscan) {
  scan_rec temprec;
    
  temprec.SetScanRec(keyscan);
  memcpy(&stnkey[0],&temprec.stnid,14);
  TransformKey(stnidx,stnkey);
  }
*/

void rp_isam::MakeRecStnKey(scan_rec *keyrec) {
    
  memcpy(&stnkey[0],&keyrec->stnid,14);
  TransformKey(stnidx,stnkey);
}

/*
  void rp_isam::MakeScanDttmKey(rdr_scan *keyscan) {
  scan_rec temprec;
    
  temprec.SetScanRec(keyscan);
  memcpy(&dttmkey[0],&temprec.datetime,4);
  memcpy(&dttmkey[4],&temprec.stnid,2);
  memcpy(&dttmkey[4],&temprec.scantype,8);
  TransformKey(dttmidx,dttmkey);
  }
*/
void rp_isam::MakeRecDttmKey(scan_rec *keyrec) {

  scan_rec local_rec = *keyrec;;
#ifdef SCANREC_BYTESWAP
  local_rec.byteSwapFields();
#endif

    
  memcpy(&dttmkey[0],&local_rec.datetime,4);
  memcpy(&dttmkey[4],&local_rec.stnid,2);
  memcpy(&dttmkey[6],&local_rec.scan_type,8);
  TransformKey(dttmidx,dttmkey);
}

void rp_isam::MakeScanStnKey(rdr_scan *keyscan) {
  scan_rec temprec;
    
  temprec.SetScanRec(keyscan);
#ifdef SCANREC_BYTESWAP
  temprec.byteSwapFields();
#endif

  memcpy(&stnkey[0],&temprec.stnid,14);
  TransformKey(stnidx,stnkey);
}

void rp_isam::MakeScanDttmKey(rdr_scan *keyscan) {
  scan_rec temprec;
    
  temprec.SetScanRec(keyscan);
  memcpy(&dttmkey[0],&temprec.datetime,4);
  memcpy(&dttmkey[4],&temprec.stnid,2);
  memcpy(&dttmkey[4],&temprec.scan_type,8);
  TransformKey(dttmidx,dttmkey);
}

void rp_isam::RecAtPercent(COUNT percent, scan_rec *retrec, idxtype IdxType = NOIDX) {
  rp_key tempkey;
  if (IdxType == STN) CurrentIdx = stnidx;	
  if (IdxType == DTTM) CurrentIdx = dttmidx;	// default NOIDX=no change
  ReadData(dfil,KeyAtPercentile(CurrentIdx,tempkey,percent),retrec);
}

void rp_isam::MakeStnKey(short StnId, time_t tm = -1) {
  memset(stnkey, 0, sizeof(stnkey));
  memcpy(&stnkey[0],&StnId,2);
  if (tm >= 0)
    memcpy(&stnkey[2],&tm,4);
  TransformKey(stnidx,stnkey);
}

void rp_isam::MakeDttmKey(time_t dttm) {
  memset(dttmkey, 0, sizeof(dttmkey));
  memcpy(&dttmkey[0],&dttm,4);
  TransformKey(dttmidx,dttmkey);
}

void rp_isam::getStnSet(stnSet* stnset)  // get set of stns in this db, return pointer to stn_set
{
  if (!stnset)
    stnset = &stn_set;
  if (!FirstRec(STN))
    {
      stnset->clear();
    }
  else
    {
      if (CurrentRec.stnid < StnDataMax)
	stnset->set(CurrentRec.stnid);
      while (NextStn())
        if (CurrentRec.stnid < StnDataMax)
	  stnset->set(CurrentRec.stnid);
    }
}

bool rp_isam::NextStn() {
  if (CurrentIdx != stnidx) {
    fprintf(stderr,"rp_isam::NextStn - CurrentIdx != stnidx\n");
    return FALSE;
  }
  MakeStnKey(CurrentRec.stnid+1);
  if (GetGTERecord(stnidx,stnkey,&CurrentRec)) {
    if (isam_err != INOT_ERR) 
      fprintf(stderr,"rp_isam::NextStn failed (%d, %d)\n",isam_err,isam_fil);
    return FALSE;
  }
  else return TRUE;
}

bool rp_isam::PrevStn() {
  if (CurrentIdx != stnidx) {
    fprintf(stderr,"rp_isam::PrevStn - CurrentIdx != stnidx\n");
    return FALSE;
  }
  if (CurrentRec.stnid < 1) {
    fprintf(stderr,"rp_isam::PrevStn - CurrentStn < 1 (%d)\n",CurrentRec.stnid);
    return FALSE;
  }
  MakeStnKey(CurrentRec.stnid-1);
  if (GetGTERecord(stnidx,stnkey,&CurrentRec)) {
    if (isam_err != INOT_ERR) 
      fprintf(stderr,"rp_isam::NextStn failed (%d, %d)\n",isam_err,isam_fil);
    return FALSE;
  }
  else return TRUE;
}

bool rp_isam::NextDay() {	// I suspect a problem here with Local time
  time_t nextday;
  if (CurrentIdx != dttmidx) {
    nextday = ((CurrentRec.datetime-CorrectDBLocalTime())/SecsPerDay) + 1;
    MakeStnKey(CurrentRec.stnid,(nextday*SecsPerDay)+CorrectDBLocalTime());
    if (GetGTERecord(stnidx,stnkey,&CurrentRec)) {
      if (isam_err != INOT_ERR)
	fprintf(stderr,"rp_isam::NextDay failed (%d, %d)\n",isam_err,isam_fil);
      return FALSE;
    }
    else {
      if (CurrentRec.datetime > 0) return TRUE;
      else return FALSE;
    }
  }
  nextday = ((CurrentRec.datetime-CorrectDBLocalTime())/SecsPerDay) + 1;
  MakeDttmKey((nextday*SecsPerDay)+CorrectDBLocalTime());
  if (GetGTERecord(dttmidx,dttmkey,&CurrentRec)) {
    if (isam_err != INOT_ERR)
      fprintf(stderr,"rp_isam::NextDay failed (%d, %d)\n",isam_err,isam_fil);
    return FALSE;
  }
  else {
    if (CurrentRec.datetime > 0) return TRUE;
    else return FALSE;
  }
}

bool rp_isam::PrevDay() {
  time_t prevday;
  if (CurrentIdx != dttmidx) {
    prevday = ((CurrentRec.datetime-CorrectDBLocalTime())/SecsPerDay) - 1;
    MakeStnKey(CurrentRec.stnid,(prevday*SecsPerDay)+CorrectDBLocalTime());
    if (GetGTERecord(stnidx,stnkey,&CurrentRec)) {
      if (isam_err != INOT_ERR)
	fprintf(stderr,"rp_isam::PrevDay failed (%d, %d)\n",isam_err,isam_fil);
      return FALSE;
    }
    else {
      if (CurrentRec.datetime > 0) return TRUE;
      else return FALSE;
    }
  }
  prevday = ((CurrentRec.datetime-CorrectDBLocalTime())/SecsPerDay) - 1;
  MakeDttmKey((prevday*SecsPerDay)+CorrectDBLocalTime());
  if (GetGTERecord(dttmidx,dttmkey,&CurrentRec)) {
    if (isam_err != INOT_ERR)
      fprintf(stderr,"rp_isam::PrevDay failed (%d, %d)\n",isam_err,isam_fil);
    return FALSE;
  }
  else {
    if (CurrentRec.datetime > 0) return TRUE;
    else return FALSE;
  }
}

bool rp_isam::SearchDtTm(time_t tm, bool quiet) {
  CurrentIdx = dttmidx;
  MakeDttmKey(tm);
  if (GetGTERecord(dttmidx,dttmkey,&CurrentRec)) {
    if (!quiet)
      fprintf(stderr,"rp_isam::SearchDtTm failed (%d, %d)\n",isam_err,isam_fil);
    return FALSE;
  }
  else return TRUE;
}
	
bool rp_isam::SearchStn(short stn, time_t tm) {
  if (!stn) return SearchDtTm(tm);	// if stn not defined, use tm
  CurrentIdx = stnidx;
  MakeStnKey(stn,tm);
  if (GetGTERecord(stnidx,stnkey,&CurrentRec)) {
    fprintf(stderr,"rp_isam::SearchStn failed (%d, %d)\n",isam_err,isam_fil);
    return FALSE;
  }
  else return TRUE;
}

bool rp_isam::FirstStnRec(short stn) {
  CurrentIdx = stnidx;
  MakeStnKey(stn,-1);
  if (GetGTERecord(stnidx,stnkey,&CurrentRec)) {
    fprintf(stderr,"rp_isam::FirstStnRec failed (%d, %d)\n",isam_err,isam_fil);
    return FALSE;
  }
  else return CurrentRec.stnid == stn;
}

bool rp_isam::LastStnRec(short stn) {
  CurrentIdx = stnidx;
  MakeStnKey(stn+1,-1);
  if (GetLTERecord(stnidx,stnkey,&CurrentRec)) {
    fprintf(stderr,"rp_isam::LastStnRec failed (%d, %d)\n",isam_err,isam_fil);
    return FALSE;
  }
  if (CurrentRec.stnid > stn) // if EQ next stn at 0 tm use prev rec
    if (PreviousRecord(stnidx,&CurrentRec)) {
      fprintf(stderr,"rp_isam::LastStnRec failed (%d, %d)\n",isam_err,isam_fil);
      return FALSE;
    }
  return CurrentRec.stnid == stn;
}

int rp_isam::ReadScanSet(int fd, unsigned long &offset, unsigned long &endoffset, int &size, 
			 rdr_scan *readscan, int raw = 0) 
{

  if (readscan && 
      readscan->readRapicFile(fd, offset, endoffset, size))
    return 1;
  else
    return -1;

//   char  c;
//   //  bool debug = FALSE;
//   bool done = FALSE;
//   int	  buff_size = 1024;
//   char  *buff = 0, *buff_pos;	// data buffer, & buff pos pointer
//   int   b_size = 0, imgsize = 0;
//   int   b_pos = 0;		// buff pos count
//   rdr_scan_linebuff *linebuff = 0;
//   int		pass1count = 0;
//   //	bool	headerok = FALSE;
    
    
//   int   total_rd = 0;
    
//   if (raw);	// shut up compiler
//   if (fd < 0) return -1;
//   if (!readscan) return 0;
//   buff = new char[buff_size];
//   linebuff = new rdr_scan_linebuff();
//   // if offset < 0, use current pos
//   if (offset >= 0) lseek(fd,offset,SEEK_SET);
//   else offset = long(lseek(fd,0,SEEK_CUR));
//   endoffset = offset;
//   while ((!done) && ((b_size = read(fd,buff,buff_size)) > 0)) {
//     done |= b_size == 0;
//     total_rd += b_size;
//     b_pos = 0;
//     buff_pos = buff;
//     while ((b_pos < b_size) && (!done)) {
//       c = *buff_pos;
//       imgsize++;		// imgsize is file size of this scan
//       endoffset++;
//       linebuff->addchar_parsed(c);
//       if (linebuff->IsEOL())
// 	{
// 	  readscan->AddScanLine(linebuff->line_buff, 
// 				linebuff->lb_size,readscan);
// 	  done |= readscan->Complete();
// 	  done |= readscan->HeaderValid() &&   // only done if a valid header has been read
// 	    (sscanf(linebuff->line_buff,"/IMAGEEND%c",&c) == 1);
// 	  if (sscanf(linebuff->line_buff," PASS: 01 of%c",&c) == 1) {
// 	    pass1count++;
// 	    done |= pass1count > 1;
// 	  }
// 	  linebuff->reset();
// 	}
//       b_pos++;	    // buffer pos count
//       buff_pos++;	    // buffer pos pointer
//     }
//     if (size >= 0) done |= total_rd >= size;
//   }
//   if (size == -1) size = imgsize;
//   readscan->data_finished();
//   if (linebuff) delete linebuff;
//   if (buff) delete[] buff;
//   if (!readscan->Finished())
//     return -1;	    // scan not finished or not valid
//   else return 1;
}	
	    
bool rp_isam::GetScanSet(rdr_scan *getscan) {
  unsigned long endoffset;
  //    printf("rp_isam::GetScan getting ");
  if (ReadScanSet(datafd, CurrentRec.offset,
		  endoffset, CurrentRec.size, getscan) >= 0) {
    getscan->data_source = DB;
    getscan->StoredInDB = TRUE;
    getscan->DBDuplicate = FALSE;
    //	CurrentRec.PrintRec();
#ifdef TITAN
#ifndef USE_TITAN_THREAD
    if ( useTitan && useTitan == getscan->station) {
      fprintf(stderr,"From rp_isam::GetScanSet\n");
      rapicToTitan(getscan,lastinSet);
    }
#endif
#endif
    return TRUE;
  }
  else return FALSE;
}
	
void rp_isam::Dump100Percent() {
  int pc;
    
  for (pc=0; pc<=100;pc++) {
    RecAtPercent(pc,&CurrentRec);
    CurrentRec.PrintRec();
  }
}

void rp_isam::startProgress(char *progresstitle)
{
  lastProgressPercent = -1;
#ifndef NO_XWIN_GUI
  if (useProgressDialog)
    ProgressDialog(progresstitle);
#endif
  progressTitle = progresstitle;
}

bool rp_isam::updateProgress(int percent, 
			     time_t scantime,
			     int loadedcount,
			     long loadedsize)
{
  bool finished = false;
  char timestr[128], tempstr[32] ;

  if (percent == lastProgressPercent)
    return finished;
  else
    {
#ifndef NO_XWIN_GUI
      if (useProgressDialog)
	{
	  ProgressDialogPercentDone(percent);
	  finished = ProgressDialogInterrupted();
	}
#endif
      if ((percent % 5) == 0)
	{
	  fprintf(stdout, "Load %d%% Complete - Scan time=%s Scans loaded=%d"
		  "Loaded Size=%s\n", 
		  percent,
		  ShortTimeString(scantime, timestr),
		  loadedcount, scaled_kMGTSizeString(loadedsize, tempstr));
	  fflush(stdout);
#ifndef NO_XWIN_GUI
	  char newtitle[512];
	  sprintf(newtitle, "%s - %d%%",
		  progressTitle.c_str(), percent);
	  ProgressDialogUpdateTitle(newtitle);
#endif
	}
    }
  lastProgressPercent = percent;
  return finished;
}
  
void rp_isam::closeProgress()
  {
#ifndef NO_XWIN_GUI
      ProgressDialogOff();
#endif
  }

void rp_isam::ReadFile(int fd, char *fn, bool AddIsamOnly) {
  rdr_scan	*TempScan = NULL;
  unsigned long	offset = 0, 
    endoffset = 0;
  int 	size = -1;
  
  int percentread = 0, loadedcount; 
    
  // if reading from this data file (rebuilding??) only add records to ISAM
  // or fn not defined
  if ((strcmp(fname,fn) == 0) || !fn) AddIsamOnly = 1;
  if (!fn)
    fn = fname;
  if (fd < 0) {
    fd = open(fn,O_RDWR);
    if (fd == -1) {
      fprintf(stderr,"rp_isam::ReadFile Couldn't open data file: %s\n",fn);
      perror(0);
      return;			// if file not present, quit
    }
  }
  // offset=-1 Reads from current file pos, returns start offset
  // size=-1 No limit on read length, return read size
  if (fd < 0) return;
  startProgress("Please wait....Recreating database from file");
  off_t fsize = lseek(fd,0,SEEK_END);
  lseek(fd,0,SEEK_SET);
  int readresult = 0;
  bool finished = false;
  while (!finished) {
    if (TempScan == NULL)
      TempScan = new rdr_scan(this, "rp_isam::ReadFile");
    readresult = ReadScanSet(fd, offset, endoffset, size, TempScan);
    if ((readresult > 0) && TempScan->HeaderValid())
      {
	if (!AddIsamOnly) AddScan(TempScan);
	else AddScanIsam(TempScan,fn,offset,size);
	loadedcount++;
      }
    offset = endoffset;    // get current offset
    size = -1;
    percentread = int(float(endoffset) / float(fsize) * 100.0);
    finished = updateProgress(percentread, TempScan->scan_time_t,
			      loadedcount, endoffset);
    if (TempScan->ShouldDelete(this, "rp_isam::ReadFile")) 
      delete TempScan;
    TempScan = NULL;
    finished |= off_t(endoffset) >= fsize-1;
  }
  closeProgress();
}
	
#ifndef NO_XWIN_GUI
// PreviewScan loads a scan from the dbase, passes it to rdrseq, then
// removes references to the image (ie it becomes rdrseq's responsibility)
void rp_isam::PreviewScan(rdr_seq *viewer, bool DrawFlag) {
  time_t	start_time, end_time;
  bool flag = TRUE, newviewimg = FALSE;

  if (viewer) Viewer = viewer;
  if (DrawFlag) {
    if (!DBMultiStnLoad) {
      if (!tempscan) 
	tempscan = new rdr_scan(this, "rp_isam::PreviewScan");
      if (GetScanSet(tempscan))  
	if (Viewer) Viewer->ViewScan(tempscan);
      if (tempscan->ShouldDelete(this, "rp_isam::PreviewScan")) 	// no longer interested
	delete tempscan;		// delete if no-one else wants it
      tempscan = 0;		//clear reference
    }
    else {	// DBMultiStnLoad is true, get sel img +/- rdrImgTmWin()
      start_time = int(CurrentRec.datetime / viewer->rdrImgTmWin()) * viewer->rdrImgTmWin();
      end_time = start_time + viewer->rdrImgTmWin() -1;	    
      if (!tempscan) tempscan = new rdr_scan(this, "rp_isam::PreviewScan");
      if (flag = SearchDtTm(start_time))
	if (GetScanSet(tempscan)) {
	  PushState();		    
	  if (Viewer) {
	    // force new img, don't draw it yet
	    Viewer->ViewScan(tempscan, FALSE, FALSE);
	    newviewimg = TRUE;
	  }
	  PopState();
	}
      if (tempscan->ShouldDelete(this, "rp_isam::PreviewScan")) 	// no longer interested
	delete tempscan;		// delete if no-one else wants it
      tempscan = 0;		//clear reference
      flag &= NextRec();
      flag &= CurrentRec.datetime <= end_time;
      while (flag) {
	if (!tempscan) tempscan = new rdr_scan(this, "rp_isam::PreviewScan");
	if (GetScanSet(tempscan)) {
	  PushState();		    
	  // add scans to viewimg, don't draw yet
	  if (Viewer) 
	    {
	      if (!newviewimg)
		{
		  Viewer->ViewScan(tempscan, FALSE, FALSE);
		  fprintf(stderr, "rp_isam::PreviewScan - Previous GetScanSet failed, starting new ViewImg\n");
		}
	      else
		Viewer->ViewScan(tempscan, TRUE, FALSE);
	      newviewimg = TRUE;
	    }
	  PopState();
	}
	flag &= NextRec();
	flag &= CurrentRec.datetime <= end_time;
	if (tempscan->ShouldDelete(this, "rp_isam::PreviewScan")) 	// no longer interested
	  delete tempscan;		// delete if no-one else wants it
	tempscan = 0;		//clear reference
      }
      // draw image now
      if (newviewimg) Viewer->DrawViewImg();
    }
  }
  else {
    if (Viewer) Viewer->ViewScan();// clear preview mode in rdrseq
  }
}
#endif
	
bool rp_isam::OpenSrcDB(char *srcname, 
			bool RdOnly, bool AllowRdOnlyRebuild) {
  char pname[256],basename[256];
    
  SplitPName(srcname,pname,basename);
  RemoveDatSuffix(basename);
  if (!srcdb) srcdb = new rp_isam();
   
  if (!srcdb->Open(pname,basename, 
		   RdOnly, AllowRdOnlyRebuild)) { // open source as ReadOnly
    delete srcdb;
    srcdb = 0;
    return FALSE;
  }
  else return TRUE;
}
	
void rp_isam::CloseSrcDB() {
  if (srcdb) {
    delete srcdb;
    srcdb = 0;
  }
}
	
bool rp_isam::OpenDestDB(char *destname) {
  char pname[256],basename[256];
    
  SplitPName(destname,pname,basename);
  RemoveDatSuffix(basename);
  if (!destdb) destdb = new rp_isam();
  destdb->ReadOnly = false;
  if (!destdb->Open(pname,basename) || 
      destdb->ReadOnly) {
    if (destdb->ReadOnly)
      fprintf(stderr, "rp_db::OpenDestDB - ERROR - UNABLE TO OPEN %s AS READ/WRITE, CLOSING\n", 
	      destname);
    delete destdb;
    destdb = 0;
    return FALSE;
  }
  else return TRUE;
}
	
bool rp_isam::CreateDestDB(char *destname) {
  char pname[256],basename[256];
    
  SplitPName(destname,pname,basename);
  RemoveDatSuffix(basename);
  if (!strlen(basename)) return FALSE;
  if (!destdb) destdb = new rp_isam();
  if (!destdb->Create(pname,basename)) {
    delete destdb;
    destdb = 0;
    return FALSE;
  }
  else return TRUE;
}
	
void rp_isam::CloseDestDB() {
  if (destdb) {
    delete destdb;
    destdb = 0;
  }
}
	
// if copy completed, return true
// if maxdbsize specified and destdb size exceeds it, return false (not completed)
// copyrecs can then be called with resumecopy set to continue where left off
bool rp_isam::CopyRecs(long &recscopied, int stn, time_t tm1, time_t tm2, 
		       int spacing, e_scan_type scantype,
		       long long maxdbsize, bool resumecopy, bool quiet) {
  stnSet copystns;
  if (stn)
    copystns.set(stn);
  return CopyRecs(recscopied, copystns, tm1, tm2, spacing, scantype, maxdbsize, resumecopy, quiet);
}

bool rp_isam::CopyRecs(long &recscopied, rp_copy_req& copy_req, bool resumecopy, bool quiet)
{
  return CopyRecs(recscopied, copy_req.stn_set, copy_req.start_time, copy_req.end_time,
	   copy_req.spacing, copy_req.scan_type, copy_req.max_dbsize, 
		  resumecopy, quiet);
}
  

// spacing only relevant for single stn
bool rp_isam::CopyRecs(long &recscopied, stnSet &copystns, 
		       time_t tm1, time_t tm2, 
		       int spacing,  e_scan_type scantype,
		       long long maxdbsize, bool resumecopy, bool quiet) {
  bool flag;
  rp_isam *src_db,*dest_db;
  int count,copycount;
  bool copyallstns = copystns.stnCount() == 0;  // if no stns set, assume copy ALL stns
  int singlestn = copystns.stnCount() == 1;  // special case for single stn, allow spacing

  recscopied = 0;
  if (destdb == srcdb) {
    fprintf(stderr,"rp_isam::CopyRecs ERROR - source=destination\n");
    return true;
  }

  if (singlestn)  // get actual stn number for singlestn
    {
      int stn = 0;
      while (!copystns.isSet(stn) && (stn < copystns.size()))
	stn++;
      singlestn = stn;
    }
  if (tm1 > tm2) {			// ensure tm1 < tm2
    time_t tm_temp = tm1;
    tm1 = tm2;
    tm2 = tm_temp;
  }
    
  if (srcdb) src_db = srcdb;
  else src_db = this;
  if (destdb) dest_db = destdb;
  else dest_db = this;	// copy data to work db
  
  bool destdbfull = maxdbsize && (dest_db->datasize > maxdbsize);
  if (!resumecopy)
    {
      if (singlestn)
	{
	  flag = src_db->SearchStn(singlestn, tm1); // if tm1 is 0, start at first rec
	}
      else
	{
	  flag = src_db->SearchDtTm(tm1); // if tm1 is 0, start at first rec
	}
    }
  if (singlestn)
    dest_db->setIDXMode(STN);
  else
    dest_db->setIDXMode(DTTM);
  copycount = 0;
  while (flag && !destdbfull) {
    if ((copyallstns || copystns.isSet(src_db->CurrentRec.stnid)) &&
	((scantype == e_st_max) || (scantype == CurrentRec.scantype())))
      {
	if (!srcdb) datafd = GetCurrentRecFd();	// sets the datafd for this rec
	if (flag = dest_db->CopyCurrent(src_db, quiet)) copycount++;
      }
    destdbfull = (maxdbsize > 0) && (dest_db->datasize > maxdbsize);
    if (flag)  // only step to next if last CopyCurrent OK
      {
	flag = src_db->NextRec();
	if (tm2) // if tm2 is 0, copy all times
	  flag &= (src_db->CurrentRec.datetime <= tm2);
	if (singlestn && flag)
	  {
	    count = spacing-1;
	    while ((count-- > 0) && flag)
	      {
		flag = src_db->NextRec();
		flag &= src_db->CurrentRec.stnid == singlestn;
		if (tm2) // if tm2 is 0, copy all times
		  flag &= (src_db->CurrentRec.datetime <= tm2);
	      }
	  }
      }
  }
  if (destdbfull)
    printf("rp_isam::CopyRecs - Copy terminated: maxdbsize(%dMB) reached\n", 
	   int(maxdbsize/1000000));
  printf("rp_isam::CopyRecs - Images copied = %d\n",copycount);
  recscopied = copycount;
  return !destdbfull;
}

// spacing only relevant for single stn
void rp_isam::dumpRecStats(long &recsmatching, rp_copy_req& copy_req, FILE *dumpfile,
			   bool sizefirst)
{
  if (!dumpfile)
    return;
  bool flag;
  bool copyallstns = copy_req.stn_set.stnCount() == 0;  // if no stns set, assume copy ALL stns
  int singlestn = copy_req.stn_set.stnCount() == 1;  // special case for single stn, allow spacing

  recsmatching = 0;
  if (singlestn)  // get actual stn number for singlestn
    {
      int stn = 0;
      while (!copy_req.stn_set.isSet(stn) && (stn < copy_req.stn_set.size()))
	stn++;
      singlestn = stn;
    }
  if (copy_req.start_time > copy_req.end_time) {			// ensure tm1 < tm2
    time_t tm_temp = copy_req.start_time;
    copy_req.start_time = copy_req.end_time;
    copy_req.end_time = tm_temp;
  }
    
  if (singlestn)
    {
      flag = SearchStn(singlestn, copy_req.start_time); // if tm1 is 0, start at first rec
    }
  else
    {
      flag = SearchDtTm(copy_req.start_time); // if tm1 is 0, start at first rec
    }
  while (flag) {
    if ((copyallstns || copy_req.stn_set.isSet(CurrentRec.stnid)) &&
	((copy_req.scan_type == e_st_max) || (copy_req.scan_type == CurrentRec.scantype())))
      {
	GetCurrentRecFd();	// sets the datafd for this rec
	//	if (flag = dest_db->CopyCurrent(src_db, quiet)) copycount++;
	CurrentRec.PrintRecShort(dumpfile, sizefirst);
	recsmatching++;
      }
    flag = NextRec();
    if (copy_req.end_time) // if tm2 is 0, copy all times
      flag &= (CurrentRec.datetime <= copy_req.end_time);
  }
  printf("rp_isam::dumpRecStats - matching recs  = %d\n", int(recsmatching));
}

bool rp_isam::CopyCurrent(rp_isam *srcisam, bool quiet) {
  if (!srcisam->IsOpen()) {
    if (!quiet) fprintf(stderr,"rp_isam::CopyCurrent - ERROR srcisam not open\n");
    return FALSE;
  }
  if (!IsOpen()) {
    if (!quiet) fprintf(stderr,"rp_isam::CopyCurrent - ERROR destisam not open\n");
    return FALSE;
  }
  srcisam->CurrentRec.CopyRec(&temprec);
  if (RecPresent(&temprec)) {
    if (!quiet) fprintf(stderr,"rp_isam::CopyCurrent failed. Duplicate record.\n");
    return FALSE;
  }
  if (!(temprec.size = AppendData(srcisam->datafd,temprec.offset,temprec.size))) 
    return FALSE;	// temprec.offset set to start offset in this datafd
  if (strlen(fname) > 15)
    if (!quiet) fprintf(stderr,"rp_isam::CopyCurrent WARNING filename > 15chars - truncating, THIS REC CANNOT BE USED IN MERGED DB.\n");
  strncpy(temprec.filename,fname,15);
  return AddRec(&temprec);
}
	
int rp_isam::GetCurrentRecFd() {
  return datafd;
}

void rp_isam::passScanToClient(rdr_scan *newscan, 
			     scan_client *ScanClient, 
			     scan_client  *ScanClient2)
{
  if (newscan &&
      //	!showDBScansToScanMng &&
      (ScanClient != NULL_CLIENT ))
    {
      ScanClient->NewDataAvail(newscan);
      ScanClient->FinishedDataAvail(newscan);
      ScanClient->CheckNewData();
      ScanClient->ProcessCheckedData();
      //Add second client for Migfa
      if (newscan && (ScanClient2 != NULL_CLIENT )) 
	{
	  ScanClient2->NewDataAvail(newscan);
	  ScanClient2->FinishedDataAvail(newscan);
	}
    }

  // show the new scan throught the scan manager
  // to all of the scan clients
  if (ScanMng && showDBScansToScanMng)
    {
      ScanMng->NewDataAvail(newscan);
      ScanMng->FinishedDataAvail(newscan);
    }   
}

// if num = 0, don't load anything
// if num = -1, load until ScanClient "full"
void rp_isam::LoadSeq(scan_client *ScanClient, DBSEQMODE mode, int num,int stn, time_t tm, int spacing) {
  if (!ScanClient) return;
  switch (mode) {
  case REALTIME:
    LoadTo(ScanClient, true, num);
    break;
  case LOADTO:
    LoadTo(ScanClient, false, num, stn,tm,spacing);
    break;
  case LOADFROM:
    LoadFrom(ScanClient, num, stn,tm,spacing);
    break;
  case LOADCENTRE:
    LoadCentre(ScanClient, num, stn,tm,spacing);
    break;
  }
}

void rp_isam::LoadTo(scan_client *ScanClient, bool ReloadRealtime, int num, int stn, time_t tm, int spacing, scan_client *ScanClient2) {
  bool	flag = TRUE,full = false;
  rdr_scan    *newscan;
  rp_isam	*src_db;
  int	    count;
  long      loadedsize = 0;
  int       loadedcount = 0;
  bool      debug = false;
  //    e_scan_client_mode clientmode, client2mode;
  
  int imgcountpercentfull = 0, 
    maxpercentfull = 0;	// keep track of memory and num images, biggest val used

  time_t prevRecTime = 0,
    timeCutoff = 0,         // time limit for loading
    maxGap = 0;
    
  debug = FileExists("debugloadto");
  if (ScanMng)
    {
      if (ReloadRealtime)
	ScanMng->SetDataMode(REALTIMEMODE);
      else
	ScanMng->SetDataMode(DBREVIEWMODE);
    }
  if (!ScanClient && !ScanMng) return;
  if (ScanClient && (full = ScanClient->Full(-1, false))) return;
  if (num == -1) num = 0;			// if depth -1, disable numtest
  if (srcdb) src_db = srcdb;
  else src_db = this;
  if (DBMultiStnLoad) {
    stn = 0;	    // ignore stn in multi stn load, forces use of dttm index
    if (ScanClient)
      if (tm) tm += ScanClient->rdrImgTmWin();
  } 
  if (stn || tm) { 
    flag = src_db->SearchStn(stn,tm);   // stn and/or time based load
    if (!flag && DBMultiStnLoad && tm) {// search for tm+tmwin failed, try time
      if (ScanClient)
	tm -= ScanClient->rdrImgTmWin(); // remove time window allowance
      flag = src_db->SearchStn(stn,tm);	// try search on orig time
    }
  }
  else {
    flag = src_db->LastRec(DTTM);	// load from most recent back
  }
  /*
    clientmode = ScanClient->GetDataMode();
    ScanClient->SetDataMode(DBREVIEWMODE);
    if (ScanClient2 != NULL_CLIENT)
    {
    client2mode = ScanClient2->GetDataMode();
    ScanClient2->SetDataMode(DBREVIEWMODE);
    }
  */
  startProgress("Please wait....LoadTo from database");
  if (flag)
    {
      if (ReloadRealtime)
	{
	  if (maxLoadRealTimeSecs)
	    timeCutoff = src_db->CurrentRec.datetime - maxLoadRealTimeSecs;
	  maxGap = maxLoadRealTimeGap;
	}
      else
	maxGap = maxLoadDBGap;
      prevRecTime = src_db->CurrentRec.datetime;  
    }
    
  while (flag) {
    if (src_db->CurrentRec.stnid && 
	(!stn || (stn == src_db->CurrentRec.stnid))){ 
      newscan = new rdr_scan(this, "rp_isam::LoadTo");
      if (src_db->GetScanSet(newscan)) {
	if (ReloadRealtime)
	  newscan->data_source = DBRELOADREALTIME;
	passScanToClient(newscan, ScanClient, ScanClient2);
	loadedsize += newscan->scanSetSize();
	loadedcount++;
	if (debug)
	  fprintf(stdout, "%s\n", newscan->ScanKey());
	if (ScanClient)
	  {
	    full = ScanClient->Full(loadedsize, false);
	    maxpercentfull = ScanClient->percentFull(loadedsize);
	    if (num) {
	      full |= ScanClient->NumImgs() >= num;
	      if (num)
		imgcountpercentfull = ScanClient->NumImgs() * 100 / num;
	      if (imgcountpercentfull > maxpercentfull)
		maxpercentfull = imgcountpercentfull;
	    }
	  }
	else if (ScanMng)
	  {
	    maxpercentfull = ScanMng->cachePercentFull();
	    full = maxpercentfull >= 100;
	  }
	flag = !updateProgress(maxpercentfull,
			       newscan->scan_time_t,
			       loadedcount,
			       loadedsize);
      }
      if (newscan->ShouldDelete(this, "rp_isam::LoadTo"))
	delete newscan;
      newscan = 0;
    }
    if (stn) count = spacing;
    else count = 1;
    while (count-- && flag) 
      flag = src_db->PrevRec();
    if (stn) flag &= stn == src_db->CurrentRec.stnid;
    flag &= !full;
    if (timeCutoff && (src_db->CurrentRec.datetime < timeCutoff))
      {
	flag = false;
	cout << "rp_isam::LoadTo - Loading stopped at time cutoff limit\n";
      }
    if (maxGap && ((prevRecTime - src_db->CurrentRec.datetime) > maxLoadDBGap))
      {
	flag = false;
	cout << "rp_isam::LoadTo - Loading stopped - Gap in data exceeds limit\n";
      }
    prevRecTime = src_db->CurrentRec.datetime; 
  }
  closeProgress();
  if (ScanMng)
    ScanMng->NewSeqLoaded();
}
	
void rp_isam::LoadFrom(scan_client *ScanClient, int num, int stn, time_t tm, int spacing, scan_client *ScanClient2) {
  bool	flag, full;
  rdr_scan	*newscan;
  rp_isam	*src_db;
  int count;
  //    e_scan_client_mode clientmode, client2mode;
  long      loadedsize = 0;
  int       loadedcount = 0;
  int imgcountpercentfull, 
    maxpercentfull;	// track of memory and num images, biggest val used
  time_t prevRecTime = 0,
    maxGap = 0;
    
  if (ScanMng)
    ScanMng->SetDataMode(DBREVIEWMODE);	    
  if (!ScanClient) return;
  if ((full = ScanClient->Full(-1, false))) return;
  //	ScanClient->clear_seq();
  //	if (!num) num = ScanClient->seq_depth;	// get set seq depth
  if (num == -1) num = 0;						// if depth -1, disable numtest
  if (srcdb) src_db = srcdb;
  else src_db = this;
  if (DBMultiStnLoad) {
    stn = 0;	    // ignore stn in multi stn load, forces use of dttm index
    tm -= ScanClient->rdrImgTmWin();
  } 
  if (stn || tm) flag = src_db->SearchStn(stn,tm);
  else flag = src_db->FirstRec(STN);// if stn & tm both 0, load earliest
  startProgress("Please wait....LoadFrom from database");
  if (flag)
    {
      maxGap = maxLoadDBGap;
      prevRecTime = src_db->CurrentRec.datetime;  
    }
    
  while (flag) {
    if (src_db->CurrentRec.stnid && (!stn || (stn == src_db->CurrentRec.stnid))){	// don't load stnid = 0 into seq
      newscan = new rdr_scan(this, "rp_isam::LoadFrom");
      if (src_db->GetScanSet(newscan)) {
	passScanToClient(newscan, ScanClient, ScanClient2);
	loadedsize += newscan->scanSetSize();
	loadedcount++;
	maxpercentfull = ScanClient->percentFull(loadedsize);
	if (num) {
	  full |= ScanClient->NumImgs() >= num;
	  if (num)
	    imgcountpercentfull = ScanClient->NumImgs() * 100 / num;
	  if (imgcountpercentfull > maxpercentfull)
	    maxpercentfull = imgcountpercentfull;
	}

	flag = !updateProgress(maxpercentfull,
			       newscan->scan_time_t,
			       loadedcount,
			       loadedsize);
      }
      if (newscan->ShouldDelete(this, "rp_isam::LoadFrom"))
	delete newscan;
      newscan = 0;
    }
    if (stn) count = spacing;
    else count = 1;
    while (count-- && flag)
      flag = src_db->NextRec();
    full = ScanClient->Full(-1, false);
    if (num) full |= ScanClient->NumImgs() >= num;
    if (stn) flag &= stn == src_db->CurrentRec.stnid;
    flag &= !full;
    if (maxGap && ((src_db->CurrentRec.datetime - prevRecTime) > maxLoadDBGap))
      {
	flag = false;
	cout << "rp_isam::LoadFrom - Loading stopped - Gap in data exceeds limit\n";
      }
    prevRecTime = src_db->CurrentRec.datetime; 
  }
  closeProgress();
  if (ScanMng)
    ScanMng->NewSeqLoaded();
}
	
void rp_isam::LoadCentre(scan_client *ScanClient, int num, int stn, time_t tm, int spacing, scan_client *ScanClient2) {
  bool flag, full, stnspecific = FALSE;
  rdr_scan	*newscan;
  int numon2;
  rp_isam	*src_db;
  int count;
  int	fwdstn,bwdstn;
  time_t fwdtm,bwdtm;
  scan_rec *thisrec;
  //    e_scan_client_mode clientmode, client2mode;
  int imgcountpercentfull, 
    maxpercentfull;	// keep track of memory and num images, biggest val used
  long   loadedsize = 0;
  int    loadedcount = 0;
  time_t prevRecTime = 0,
    maxGap = 0;

  if (ScanMng)
    ScanMng->SetDataMode(DBREVIEWMODE);	    
  if (!ScanClient) return;
  if ((full = ScanClient->Full(-1, false))) return;
  //	ScanClient->clear_seq();
  //	if (!num) num = ScanClient->seq_depth;	// get set seq depth
  if (num == -1) num = 0;						// if depth -1, disable numtest
  numon2 = num/2;
  if (srcdb) src_db = srcdb;
  else src_db = this;
  if (DBMultiStnLoad) {
    stn = 0;	    // ignore stn in multi stn load, forces use of dttm index
    tm -= ScanClient->rdrImgTmWin();
  } 
  stnspecific = stn && !DBMultiStnLoad;	// true if station specifed
  if (stn || tm) flag = src_db->SearchStn(stn,tm);
  else flag = src_db->LastRec(STN);// if stn & tm both 0, load earliest
  if (flag) {
    thisrec = GetCurrentRec();
    fwdstn = bwdstn = thisrec->stnid;
    fwdtm = bwdtm = thisrec->datetime;
    maxGap = maxLoadDBGap;
    prevRecTime = src_db->CurrentRec.datetime;  
  }
  startProgress("Please wait....Loading from database");
  while (flag) {       // alternate btwn one before, one after loading
    flag = src_db->SearchStn(fwdstn,fwdtm);
    if (flag) {
      newscan = new rdr_scan(this, "rp_isam::Centre");
      if (src_db->GetScanSet(newscan)) {
	passScanToClient(newscan, ScanClient, ScanClient2);
	loadedsize += newscan->scanSetSize();
	loadedcount++;
	maxpercentfull = ScanClient->percentFull(loadedsize);
	if (num) {
	  full |= ScanClient->NumImgs() >= num;
	  if (num)
	    imgcountpercentfull = ScanClient->NumImgs() * 100 / num;
	  if (imgcountpercentfull > maxpercentfull)
	    maxpercentfull = imgcountpercentfull;
	}
	flag = !updateProgress(maxpercentfull,
			       newscan->scan_time_t,
			       loadedcount,
			       loadedsize);
      }
      if (newscan->ShouldDelete(this, "rp_isam::Centre"))
	delete newscan;
      newscan = 0;
    }
    if (stn) count = spacing;
    else count = 1;
    while (count-- && flag)
      flag = src_db->NextRec();
    if (flag) {
      thisrec = GetCurrentRec();
      fwdstn = thisrec->stnid;
      fwdtm = thisrec->datetime;
    }
    full = ScanClient->Full(-1, false);
    if (num) full |= ScanClient->NumImgs() >= num;
    flag &= !full;
    // load half seq size after centre
    flag &= src_db->SearchStn(bwdstn,bwdtm);
    if (stn) count = spacing;
    else count = 1;
    while (count-- && flag)
      flag = src_db->PrevRec();
    if (flag) {
      thisrec = GetCurrentRec();
      bwdstn = thisrec->stnid;
      bwdtm = thisrec->datetime;
      newscan = new rdr_scan(this, "rp_isam::Centre");
      if (src_db->GetScanSet(newscan)) {
	passScanToClient(newscan, ScanClient, ScanClient2);
	loadedsize += newscan->scanSetSize();
	loadedcount++;
	maxpercentfull = ScanClient->percentFull(loadedsize);
	if (num) {
	  full |= ScanClient->NumImgs() >= num;
	  if (num)
	    imgcountpercentfull = ScanClient->NumImgs() * 100 / num;
	  if (imgcountpercentfull > maxpercentfull)
	    maxpercentfull = imgcountpercentfull;
	}
	flag = !updateProgress(maxpercentfull,
			       newscan->scan_time_t,
			       loadedcount,
			       loadedsize);
      }
      if (newscan->ShouldDelete(this, "rp_isam::Centre"))
	delete newscan;
      newscan = 0;
    }
    full = ScanClient->Full(-1, false);
    if (num) full |= ScanClient->NumImgs() >= num;
    flag &= !full;
  }
  closeProgress();
  if (ScanMng)
    ScanMng->NewSeqLoaded();
}
	
// AppendData appends size bytes of data from srcfd at ofs, to this datafd
// The start offset in datafd is returned in ofs
int rp_isam::AppendData(int srcfd, unsigned long &ofs, unsigned int size) {
  char copybuff[4096];
  int	buffchars,remainingchars,rdchars,wrtchars;
  unsigned long startofs = ofs;
    
  if (lseek(srcfd,ofs,SEEK_SET) < 0) {
    perror("rp_isam::AppendData - Seek to CurrentRec.offset failed ");
    return 0;
  }
  if ((ofs = long(lseek(datafd,0,SEEK_END))) < 0) {
    perror("rp_isam::AppendData - Seek to end of dest. failed ");
    return 0;
  }
  remainingchars = size;
  while (remainingchars > 0) {
    if (remainingchars > 4096) buffchars = 4096;
    else buffchars = remainingchars;
    if ((rdchars = read(srcfd,copybuff,buffchars)) < 0) {
      perror("rp_isam::AppendData - Error reading source data"); 
      return(size - remainingchars);
    }
    if ((wrtchars = write(datafd,copybuff,buffchars)) < 0) {
      perror("rp_isam::AppendData - Error writing dest data");
      return(size - remainingchars);
    }
    remainingchars -= rdchars;	
  }
  
#ifdef CHECK_RECSIZE_BUG
  // A bug in the record sizes btwn version 4.19 or so and 4.38 results in 
  // short image size values (caused during addition of /RXTIME in header)
  // Check here if image read is short, look for string ADAR IMAGE after end of data
  // if found copy extra data equivalent to /RXTIME header entry
  if (lseek(srcfd,startofs+size,SEEK_SET) == off_t(startofs+size)) {
    if ((rdchars = read(srcfd,copybuff,39)) == 39) {
      if (strstr(copybuff, "ADAR IMAGE") == copybuff) // the read IS short, copy remaining data
	{
	  if ((wrtchars = write(datafd,copybuff,39)) < 39) {
	    perror("rp_isam::AppendData - Error appending short record fix up data");
	    return(size - remainingchars);
	  }
	}
    }
    remainingchars -= rdchars;	// remainingchars should be -39 here!!	  
  }
#endif
  datasize = lseek(datafd,0,SEEK_END);  
  return size - remainingchars;	// return no. of chars written
}
	
void rp_isam::SetDBReadOnlyMode(bool SetDBReadOnly) {
  ReadOnly = SetDBReadOnly;
}

bool	rp_isam::GetDBReadOnlyMode() {
  return ReadOnly;
}

rp_db::rp_db(char *pathname, bool rdonly, bool allowdbpurge) {
  next = prev = 0;
  maxdatasz = 50000000;
  DeleteOldDatabases = FALSE;
  DBPurgeDepth = -1;
  ReadOnly = rdonly;
  workdb = new rp_isam(ReadOnly);
  archdb = new rp_isam(ReadOnly);
  tempscan = 0;
  mrgDupCount = 0;
  strcpy(ArchiveDevice, "/dev/nrtape");
  OpenDB(pathname, ReadOnly, allowdbpurge);
  strncpy(SwitchDBFlagName, pname, 256);
  strncat(SwitchDBFlagName, "switchdb.flag", 256);
}
	
rp_db::~rp_db() {
  Close();
  delete workdb;
  delete archdb;
#ifndef NO_XWIN_GUI
  PreviewScan();			// ensure Preview released
#endif
}
	
scan_rec* rp_db::GetCurrentRec() {
  if (srcdb) return &srcdb->CurrentRec;
  else return &CurrentRec;
}
	
/*	
  Open will read the current db path (if any), look for working
  and archive db's (if any) and open them if available.
  Checking will be performed on the db's and if OK they will be opened
  for business.
  Otherwise, new databases will be generated when the first image data 
  is written to the db
*/
void rp_db::OpenDB(char *pathname, bool rdonly, bool allowdbpurge) {
  FILE *file = NULL;
  char Str1[256],Str2[256];
    
  ReadOnly = rdonly;

  // fetch pathname prefix to be used for all database files
  if (pathname) {
    strncpy(pname, pathname, 230);
    if (!strlen(pname))
      strcpy(pname, "./");
    if (pname[strlen(pname)-1] != '/')
      strcat(pname,"/");
  }
  else {
    if (!strlen(pname))
      strcpy(pname,"./");
    if (FileExists(rpdbinifile1))
      file = fopen(rpdbinifile1,"r");
    else if (FileExists(rpdbinifile2))
      file = fopen(rpdbinifile2,"r");
	    
    if (file) {
      if (fscanf(file," pathname=%s",pname) == 1) {
	if (pname[strlen(pname)-1] != '/')
	  strcat(pname,"/");
      }
      fprintf(stderr,"rp_db::OpenDB - using pathname=%s\n",pname);
      fclose(file);
    }
  }
  //	memset(pname, 0,sizeof(pname));
  // fetch current working and archive file names (if any)
  strncpy(rapic_data_path, pname, 256); 
  strcpy(Str1,pname);
  strcat(Str1,rpdb_files);
  fprintf(stderr,"rp_db::OpenDB - Looking for working/archive names in %s\n",
	  Str1);

  // check is rpdb data path exists, else create it
  if (DirExists(pname, true, true)) {
    file = fopen(Str1,"r");
  }
  else {
    file = NULL;
  }
  if (file) {
    if (fscanf(file," working=%s",Str2) == 1) {
      fprintf(stderr,"rp_db::OpenDB - Opening working=%s....",Str2);
      if (workdb->Open(pname,Str2, ReadOnly)) 
	fprintf(stderr,"OK\n");
      else {
	fprintf(stderr,"FAILED\n");
	if (!ReadOnly)
	  {
	    FlagArchSave(workdb->fullname);
	    if (allowdbpurge)
	      PurgeOldDBs();  // check rpdb_arch.sav for databases to remove
	    SaveDBNames();
	  }
      }
    }
    if (fscanf(file," archive=%s",Str2) == 1) {
      fprintf(stderr,"rp_db::OpenDB - Opening archive=%s....",Str2);
      if (archdb->Open(pname,Str2, ReadOnly)) 
	fprintf(stderr,"OK\n");
      else {
	fprintf(stderr,"FAILED\n");
	if (!ReadOnly)
	  SaveDBNames();
      }
    }
    fclose(file);
    fprintf(stderr,"rp_db::OpenDB - Opening Merged Index - (%s)....",
	    MrgIsamName);
    bool OK = OpenIsam(pname,MrgIsamName,ReadOnly);
    if (!OK)
      {
	fprintf(stderr,
		"FAILED\nrp_db::OpenDB - Trying READONLY Open....");
	OK = OpenIsam(pname,MrgIsamName,true); // failed, try readonly open
	if (OK)
	  {
	    fprintf(stderr,
		    "OK\nrp_db::OpenDB - Setting ReadOnly flag and "
		    "Checking Merged Index integrity....");
	    ReadOnly = true;
	  }    
      }
    if (OK) {
      fprintf(stderr,"OK\nrp_db::OpenDB - Checking Merged Index integrity....");
      int recsum = workdb->RecCount() + archdb->RecCount();
      OK = abs((recsum - RecCount())) <= int(recsum / 100);	// only worry about >%1 errors, 
      // it is possible to have same rec in both dbs, it doesn't really hurt anyone
      if (!OK) {
	fprintf(stderr,"Count Mismatch Detected\n");
	fprintf(stderr,"  Rec Counts- Merged=%d Work=%d Arch=%d (Sum=%d)\n",
		int(RecCount()),int(workdb->RecCount()),int(archdb->RecCount()),
		int(workdb->RecCount()+archdb->RecCount()));
	fprintf(stderr,"***THIS MAY BE CAUSED BY A CORRUPT MERGED INDEX, \n"
		"BUT MAY ALSO BE CAUSED BY RECORDS DUPLICATED IN BOTH THE \n"
		"WORKING AND ARCHIVE DATABASES.\n"
		);
	OK = FALSE;	// disable auto merged index creation on bad count
      }
      else fprintf(stderr,"OK\n");
    }
    if (!OK) {
      if (!ReadOnly)
	{
	  fprintf(stderr, "rp_db::OpenDB - AUTOMATICALLY REBUILDING MERGED INDEX\n");
	  OK = RepairMrgIsam(); // CreateMrgIsam();
	}
      else
	fprintf(stderr, "rp_db::OpenDB - DATABASE IS READONLY - Unable to automatically rebuild merged index\n");
    }
    fprintf(stderr,"  Rec Counts- Merged=%d Work=%d Arch=%d (Sum=%d)\n",
	    int(RecCount()),int(workdb->RecCount()),int(archdb->RecCount()),
	    int(workdb->RecCount()+archdb->RecCount()));
    if (!OK)
      {
	if (!ReadOnly)
	  fprintf(stderr, "rp_db::OpenDB - MERGED INDEX REBUILD FAILED, CREATING NEW DATABASE\n");
	workdb->Close();	    
	archdb->Close();
	rp_isam::Close();
      }
  }
  else {
    fprintf(stderr,"rp_db::OpenDB unable to open %s - ",Str1);
    perror(0);
  }
}
	
void rp_db::Close() {
  datafd = -1;
  rp_isam::Close();
  workdb->Close();
  archdb->Close();
  if (srcdb) srcdb->Close();
  if (destdb) destdb->Close();
}
	
bool rp_db::RepairMrgIsam() {
  fprintf(stderr, "rp_db::RepairMrgIsam - Rebuilding working and archive databases\n");
  if (workdb->dfil != -1) if (!workdb->Rebuild()) workdb->Close();
  if (archdb->dfil != -1) if (!archdb->Rebuild()) archdb->Close();
  return CreateMrgIsam();	// generate new merged isam
}

bool rp_db::CreateMrgIsam() {
  char  temp[136],temp1[136];;
  rp_isam	*biggestisam,*smallestisam;

  if (ReadOnly)
    {
      fprintf(stderr,"rp_db::CreateMrgIsam - Cannot Create - ReadOnly mode set\n");
      return false;
    }
  
  fprintf(stderr,"rp_db::CreateMrgIsam - Starting Merge\n");
  strcpy(fullname_isam,fullname);
#ifdef __i386__
  strcat(fullname_isam, ".i386"); 
#endif
  // try to create isam
  rp_ifil.pfilnam = fullname_isam;           // point to name
  if (dfil != -1) {
    if (CloseIFile(&rp_ifil))	//close open IFIL
      fprintf(stderr,"rp_db::CreateMrgIsam - ERROR Closing IFile\n");
    dfil = stnidx = dttmidx = -1;
  }
  strcpy(temp,fullname);
  addIDXSuffix(temp);
  unlink(temp);
  strcpy(temp,fullname);
  addDATSuffix(temp);
  unlink(temp);
  if (!(workdb->RecCount() || archdb->RecCount())) {	// no existing databases
    fprintf(stderr,"rp_db::CreateMrgIsam - No dbs open, starting fresh\n");
    return CreateIsam(pname,MrgIsamName);							// start fresh mrg isam
  }
  biggestisam = workdb;
  smallestisam = archdb;
  if (archdb->RecCount() > workdb->RecCount()) {
    biggestisam = archdb;
    smallestisam = workdb;
  }
  strcpy(temp,biggestisam->fullname);
  addIDXSuffix(temp);
  strcpy(temp1,fullname);
  addIDXSuffix(temp1);
  CopyFile(temp,temp1);
  strcpy(temp,biggestisam->fullname);
  addDATSuffix(temp);
  strcpy(temp1,fullname);
  addDATSuffix(temp1);
  CopyFile(temp,temp1);
  lock_rp_isam();
  if (!OpenIsam(pname,MrgIsamName)) {
    unlock_rp_isam();
    return FALSE;
  }
  if (!smallestisam->isOpen() || !smallestisam->FirstRec(STN)) {
    unlock_rp_isam();
    return TRUE;
  }
  bool OK = AddRec(&smallestisam->CurrentRec);
  if (!OK) 
    if (isam_err == KDUP_ERR) OK = TRUE; // continue on DUP errs
  int reccount = 0;
  mrgDupCount = 0;
  fprintf(stderr,"rp_db::CreateMrgIsam - Recs merged = ");
  while (OK && smallestisam->NextRec()) {
    OK = AddRec(&smallestisam->CurrentRec);
    reccount ++;
    if (! (reccount % 100)) fprintf(stderr," %d",reccount);
    if (USELOCKS) LockISAM(RESET);
    if (!OK) 
      {
	if (isam_err == KDUP_ERR) 
	  {
	    OK = TRUE; // continue on DUP errs
	    mrgDupCount++;
	  }
	else
	  fprintf(stderr,"\nrp_db::CreateMrgIsam - DATABASE FAULT, CANNOT CREATE MERGED INDEX\n");
      }
  }
  unlock_rp_isam();
  fprintf(stderr,"\nrp_db::CreateMrgIsam - Finished\n");
  SaveDBNames();
  return TRUE;
};
	
void rp_db::PurgeOldDBs(int count) {
  FILE    *file = 0, *tempfile = 0;
  char    fname1[256], fname2[256], temp[256], logstr[256];
  int	    linecount = 0, delcount = 0;
    
  if ((count > 0) ||
      (DBPurgeDepth > -1) ||
      DeleteOldDatabases) {	// database purging enabled
    strcpy(fname1, pname);
    strcat(fname1, "rpdb_arch.sav");
    file = fopen(fname1, "r+");
    if (!file) return;
    while (fgets(temp, 256, file)) linecount++; // count entries
    if (linecount > 0) 
      linecount--;	    // last should always be archive db, don't count it
    if (count > 0)
      delcount = count;
    else if (DeleteOldDatabases) 
      delcount = linecount;   // delete all listed databases
    else if (linecount > DBPurgeDepth)
      delcount = linecount - DBPurgeDepth;
    rewind(file);
    strcpy(fname2,pname);
    strcat(fname2,"rpdb_arch.temp");
    tempfile = fopen(fname2,"w");
    while (fgets(temp, 256, file)) {
      StripTrailingWhite(temp);
      if (delcount) {
	DeleteDB(temp);
	sprintf(logstr, "Database Purged = %s\n", temp);
	RapicLog(logstr, LOG_INFO);
	delcount--;
      }
      else
	if (tempfile) fprintf(tempfile, "%s\n", temp);
    }
  }
  if (file) { 
    fclose(file);
  }
  if (tempfile) {
    fclose(tempfile);
    if (rename(fname2, fname1) < 0)	// rename temp file over sav file
      fprintf(stderr, "rp_db::PurgeOldDBs Error renaming %s to %s : %s\n", 
	      fname2, fname1, strerror(errno));
  }
}

void rp_db::DeleteDB(char *name, char *PName) {
  char temp[256];
    
  strncpy(temp, "", 256);
  if (PName) strncpy(temp, PName, 256);
  strncat(temp,name, 256);
  if (unlink(temp) < 0)
    fprintf(stderr, "rp_db::DeleteDB Error unlinking %s: %s\n", 
	    temp, strerror(errno));
  addIDXSuffix(temp);
  if (unlink(temp) < 0)
    fprintf(stderr, "rp_db::DeleteDB Error unlinking %s: %s\n", 
	    temp, strerror(errno));
  strncpy(temp, "", 256);
  if (PName) strncpy(temp, PName, 256);
  strncat(temp,name, 256);
  addDATSuffix(temp);
  if (unlink(temp) < 0)
    fprintf(stderr, "rp_db::DeleteDB Error unlinking %s: %s\n", 
	    temp, strerror(errno));
  strncpy(temp, "", 256);
  if (PName) strncpy(temp, PName, 256);
  strncat(temp,name, 256);
  strncat(temp, ".arch", 256);
  if (FileExists(temp))
    {
      if (unlink(temp) < 0)
	fprintf(stderr, "rp_db::DeleteDB Error unlinking %s: %s\n", 
		temp, strerror(errno));
    }
}
	
/* notify archive process that there is a file to be stored to tape */
void rp_db::FlagArchSave(char *name) {
  FILE *file;
  char temp[128];
    
  strcpy(temp,pname);
  strcat(temp,"rpdb_arch.sav");
  file = fopen(temp,"a");
  if (file) { 
    fprintf(file,"%s\n",name);
    fclose(file);
  }
    
  strcpy(temp, name);		// write a null "flag" file to indicate
  strcat(temp, ".arch");	// that this database is complete and 
  file = fopen(temp, "w");	// ready to be archived
  if (file) fclose(file);
}
	
void rp_db::SwitchDB(rdr_scan *addscan) {
  char temp[128];
  rp_isam *swapdb;
    
    
  swapdb = workdb;					// straight swap of database objects
  workdb = archdb;
  archdb = swapdb;
  if (archdb->IsOpen()) { 		// notify Archive handler
    FlagArchSave(archdb->fullname);	// of db to be stored to archive	
    PurgeOldDBs();			// check rpdb_arch.sav for databases to remove
  }
  MakeDBName(addscan,temp);					// make new workdb
  if (!workdb->Open(pname,temp)) {
    if (workdb->Create(pname,temp)) 		// then create
      fprintf(stderr, " .... new db created OK\n");
  }
  else fprintf(stderr, "rp_db::SwitchDB - New db name already exists!!! Using it\n");
  SaveDBNames();
  CreateMrgIsam();
  WriteArch(0);
}
			
void rp_db::SwitchDB(scan_rec *rec) {
  char temp[128];
  rp_isam *swapdb;
    
    
  swapdb = workdb;					// straight swap of database objects
  workdb = archdb;
  archdb = swapdb;
  /*
    if (workdb->IsOpen()) {		// notify Archive handler
    FlagArchDel(workdb->fullname);	// of db to be deleted (prev arch)	
    workdb->Close();				// close prev archdb
    }
  */
  if (archdb->IsOpen()) { 		// notify Archive handler
    FlagArchSave(archdb->fullname);	// of db to be stored to archive	
    PurgeOldDBs();			// check rpdb_arch.sav for databases to remove
  }
  MakeDBName(rec,temp);					// make new workdb
  if (!workdb->Open(pname,temp)) {
    if (workdb->Create(pname,temp)) 		// then create
      fprintf(stderr, " .... new db created OK\n");
  }
  else fprintf(stderr, "rp_db::SwitchDB - New db name already exists!!! Using it\n");
  SaveDBNames();
  CreateMrgIsam();
  WriteArch(0);
}
			
void rp_db::SaveDBNames() {
  char temp[128];
  FILE *file;
    
  strcpy(temp,pname);
  strcat(temp,rpdb_files);
  file = fopen(temp,"w");
  if (file) {
    if (workdb->IsOpen())
      fprintf(file,"working=%s\n",workdb->fname);
    if (archdb->IsOpen())
      fprintf(file,"archive=%s\n",archdb->fname);
    fclose(file);
  }
  else perror("rp_db::SaveDBNames failed creating new rpdb_files");
}
	
/*
  bool rp_db::RecPresent(scan_rec *presentrec) {
  return ((workdb && workdb->IsOpen() && workdb->RecPresent(presentrec)) ||
  (archdb && archdb->IsOpen() && archdb->RecPresent(presentrec)));
  }
    
  bool rp_db::ScanPresent(rdr_scan *presentscan) {
  return ((workdb && workdb->IsOpen() && workdb->ScanPresent(presentscan)) ||
  (archdb && archdb->IsOpen() && archdb->ScanPresent(presentscan)));
  }
*/

bool rp_db::RecPresent(scan_rec *presentrec) {
  scan_rec temprec;
  MakeRecStnKey(presentrec);
  return (GetRecord(stnidx,&stnkey,&temprec) == NO_ERROR);
}

bool rp_db::ScanPresent(rdr_scan *presentscan) {
  scan_rec temprec;

  MakeScanStnKey(presentscan);
  return (GetRecord(stnidx,&stnkey,&temprec) == NO_ERROR);
}

bool rp_db::AddScan(rdr_scan *addscan) {
  if (ReadOnly) return TRUE;
  else return AddScanDest(addscan);
}
	
bool rp_db::SwitchDBFlag() 
{
  return FileExists(SwitchDBFlagName, 1, 1);
}

bool rp_db::AddScanDest(rdr_scan *addscan,rp_isam *_destdb) {
  char temp[128];
  bool OK;
    
  if (!addscan) {
    fprintf(stderr,"rp_db::AddScan - ERROR addscan=0\n");
    return TRUE;	// nothing added - as requested
  }
  if (_destdb) return _destdb->AddScan(addscan);
  if (workdb->IsOpen()) {			// only test for max sz if workdb
    if ((workdb->datasize > maxdatasz) ||
	SwitchDBFlag())
      {
	SwitchDB(addscan);
      }
    OK = workdb->AddScan(addscan);	// add image to working db
    if (OK && !RecPresent(&workdb->temprec)) {
      OK = lock_rp_isam() == 0;												// add isam record to combined isam 
      if (OK) OK = AddRec(&workdb->temprec);
      if (!OK)
	fprintf(stderr,"rp_db::AddScan - UNABLE TO ADD SCAN TO MRGISAM %s",temp);
      unlock_rp_isam();
    }
    return OK;
  }
  else {
    MakeDBName(addscan,temp);
    if (!workdb->Open(pname,temp)) {	// first try to open this name
      workdb->Create(pname,temp); // then create
      CreateMrgIsam();
    }
    if (workdb->IsOpen()) {
      SaveDBNames();
      OK = workdb->AddScan(addscan);
      if (OK) { 
	OK = lock_rp_isam() == 0;										// add isam record to combined isam 
	if (OK) OK = AddRec(&workdb->temprec);
	unlock_rp_isam();
      }
      return OK;
    }
    else {
      fprintf(stderr,"rp_db::AddScan - UNABLE TO OPEN NEW DB %s",temp);
      perror(0);
      return FALSE;
    }
  }
}
	
int rp_db::GetCurrentRecFd() {
  // Typically the first 2 test will require exact matches
  if (FNameMatch(CurrentRec.filename,workdb->fname)) 
    return workdb->datafd;
  else 
    if (FNameMatch(CurrentRec.filename,archdb->fname)) 
      return archdb->datafd;
  // These next test should give truncated names in a record a chance to match
    else
      if (FNameMatch(workdb->fname, CurrentRec.filename)) 
	return workdb->datafd;
      else 
	if (FNameMatch(archdb->fname, CurrentRec.filename)) 
	  return archdb->datafd;
	else 
	  return -1;
}
	
/*	
  bool rp_db::GetScan(rdr_scan *getscan) {
  if (srcdb) return srcdb->GetScan(getscan);
  else {
  datafd = GetCurrentRecFd();
  return rp_isam::GetScan(getscan);	
  }
  }
*/
	
bool rp_db::FNameMatch(char *fname1, char *fname2) {
  if (!strlen(fname1) || !strlen(fname2)) 	// if either NULL return FALSE
    return FALSE;
  return (strstr(fname1,fname2) != NULL);
}	
	
bool rp_db::GetScanSet(rdr_scan *getscan) {
  if (srcdb) return srcdb->GetScanSet(getscan);
  else {
    datafd = GetCurrentRecFd();
    return rp_isam::GetScanSet(getscan);	
  }
}
	
#ifndef NO_XWIN_GUI
void rp_db::PreviewScan(rdr_seq *viewer, bool flag) {
  if (srcdb) srcdb->PreviewScan(viewer,flag);
  else rp_isam::PreviewScan(viewer,flag);
}
#endif
	
void rp_db::MakeDBName(rdr_scan *scan, char *name) {
  if (scan)
    {
      if (RPDBNAME_STN_LS)
	sprintf(name,"%02d%02d%02d%02d%02d_%04d",	    
		scan->year%100, scan->month, scan->day, 
		scan->hour, scan->min, scan->station);
      else
	sprintf(name,"%04d%02d%02d%02d%02d%02d",
		scan->station,
		scan->year%100, scan->month, scan->day, scan->hour, scan->min);
    }
  else name[0] = '\0';
}
	
void rp_db::MakeDBName(scan_rec *rec, char *name) {
  int year,month,day,hour,min,sec;
  UnixTime2DateTime(rec->datetime,year,month,day,hour,min,sec);
  if (RPDBNAME_STN_LS)
    sprintf(name,"%02d%02d%02d%02d%02d_%04d",
	    year%100,month,day,hour,min, rec->stnid);
  else
    sprintf(name,"%04d%02d%02d%02d%02d%02d",rec->stnid,
	    year%100,month,day,hour,min);
}
	
bool rp_db::FirstRec(idxtype IdxType) {
  if (srcdb) return srcdb->FirstRec(IdxType);
  else return rp_isam::FirstRec(IdxType);
}
	
bool rp_db::LastRec(idxtype IdxType) {
  if (srcdb) return srcdb->LastRec(IdxType);
  else return rp_isam::LastRec(IdxType);
}
	
bool rp_db::NextStn() {
  if (srcdb) return srcdb->NextStn();
  else return rp_isam::NextStn();
}
	
bool rp_db::PrevStn() {
  if (srcdb) return srcdb->PrevStn();
  else return rp_isam::PrevStn();
}
	
bool rp_db::NextDay() {
  if (srcdb) return srcdb->NextDay();
  else return rp_isam::NextDay();
}
	
bool rp_db::PrevDay() {
  if (srcdb) return srcdb->PrevDay();
  else return rp_isam::PrevDay();
}
	
bool rp_db::NextRec(bool quiet) {
  if (srcdb) return srcdb->NextRec(quiet);
  else return rp_isam::NextRec(quiet);
}
	
bool rp_db::PrevRec(bool quiet) {
  if (srcdb) return srcdb->PrevRec(quiet);
  else return rp_isam::PrevRec(quiet);
}
	
bool rp_db::NextScan(rdr_scan *nextscan) {
  if (srcdb) return srcdb->NextScan(nextscan);
  else return rp_isam::NextScan(nextscan);
}
	
bool rp_db::PrevScan(rdr_scan *prevscan) {
  if (srcdb) return srcdb->PrevScan(prevscan);
  else return rp_isam::PrevScan(prevscan);
}
	
bool rp_db::SearchDtTm(time_t tm, bool quiet) {
  if (srcdb) return srcdb->SearchDtTm(tm);
  else return rp_isam::SearchDtTm(tm, quiet);
}
	
bool rp_db::SearchStn(short stn, time_t tm) {
  if (srcdb) return srcdb->SearchStn(stn,tm);
  else return rp_isam::SearchStn(stn,tm);
}
	
bool rp_db::FirstStnRec(short stn) {
  if (srcdb) return srcdb->FirstStnRec(stn);
  else return rp_isam::FirstStnRec(stn);
}
	
bool rp_db::LastStnRec(short stn) {
  if (srcdb) return srcdb->LastStnRec(stn);
  else return rp_isam::LastStnRec(stn);
}
	
rdr_scan* rp_db::LoadStn(short stn, time_t tm) {
  rdr_scan *localscan = 0;
  bool flag;

  localscan = new rdr_scan(this, "rp_isam::LoadStn");
  flag = SearchStn(stn,tm);
  if (!(flag && GetScanSet(localscan))) {
    if (localscan->ShouldDelete(this, "rp_isam::LoadStn"))
      delete localscan;
    localscan = 0;
  }
  return localscan;
}   

rdr_scan* rp_db::LoadStnNext(short stn) {
  rdr_scan *localscan = 0;
  bool flag;

  localscan = new rdr_scan(this, "rp_isam::LoadStnNext");
  flag = NextRec();
  if (srcdb) flag &= srcdb->CurrentRec.stnid == stn;
  else flag &= CurrentRec.stnid == stn;
  if (!(flag && GetScanSet(localscan))) {
    if (localscan->ShouldDelete(this, "rp_isam::LoadStnNext"))
      delete localscan;
    localscan = 0;
  }
  return localscan;
}   

bool rp_db::CopyCurrent(rp_isam *srcisam) {	// copy (append) current from srcdb to  this
  char temp[128];
  bool OK;
    
  if (!srcisam || !srcisam->IsOpen()) {
    fprintf(stderr,"rp_db::CopyCurrent - ERROR src not open\n");
    return FALSE;	// nothing added - as requested
  }
  if (destdb) return destdb->CopyCurrent(srcisam);
  if (workdb->IsOpen()) {			// only test for max sz if workdb
    if (workdb->datasize > maxdatasz) SwitchDB(&srcisam->CurrentRec);
    OK = workdb->CopyCurrent(srcisam);	// copy image to working db
    if (OK)												// add isam record to combined isam 
      OK = AddRec(&workdb->temprec);
    return OK;
  }
  else {
    MakeDBName(&srcisam->CurrentRec,temp);
    if (!workdb->Open(pname,temp)) {	// first try to open this name
      workdb->Create(pname,temp); // then create
      CreateMrgIsam();
    }
    if (workdb->IsOpen()) {
      SaveDBNames();
      OK = workdb->CopyCurrent(srcisam);
      if (OK) 
	OK = AddRec(&workdb->temprec);
      return OK;
    }
    else {
      fprintf(stderr,"rp_db::CopyCurrent - UNABLE TO OPEN NEW DB %s",temp);
      perror(0);
      return FALSE;
    }
  }
}
	
		
void rp_db::WriteArch(char *archdev) {
  if (archdev);
  fprintf(stderr,"rp_db::WriteArch - NOT YET IMPLEMENTED\n");
}
	
/*
  DBMng will allow multiple databases to be attached
  ONE ONLY (THE FIRST) will be used to read radar data.
  Others may be written to.
*/
	
void rp_db::SetDBReadOnlyMode(bool SetDBReadOnly) {
  ReadOnly = SetDBReadOnly;
  if (workdb) workdb->SetDBReadOnlyMode(SetDBReadOnly);
  if (archdb) archdb->SetDBReadOnlyMode(SetDBReadOnly);
}

bool	rp_db::GetDBReadOnlyMode() {
  return ReadOnly;
}

void rp_db::PushState() {
  rp_isam::PushState();
  if (srcdb) srcdb->PushState();
  if (destdb) destdb->PushState();
  if (workdb) workdb->PushState();
  if (archdb) archdb->PushState();
}
    
void rp_db::PopState() {
  rp_isam::PopState();
  if (srcdb) srcdb->PopState();
  if (destdb) destdb->PopState();
  if (workdb) workdb->PopState();
  if (archdb) archdb->PopState();
}

DBMng::DBMng(bool allowdbpurge,
	     char *inifilename) : scan_client() {
  FILE *file = 0;
  char Str1[256],
    LogStr[256],
    pnamestr[128] = "./", 
    *Str3;
  bool firstdb_ok = FALSE;
  rp_db   *newdb = 0;
  int args = 0, DBPurgeDepth;
  int MaxDBSz = 0;
  bool readonly = FALSE, DeleteOldDatabases = FALSE;
  RapicFileEntry fileEntry;
  int tempint;

  ClientType = SC_DB;
  FirstDB = LastDB = 0;
  DBReadOnly = FALSE;		// by default DB NOT read-only
  noRapicDB = false;
  InitLoadDB= NULL;             // the initLoadDB will always be readonly, used for initial sequence load
  rpdbCache = NULL;
  strcpy(ArchiveDevice, "/dev/nrtape");
  badDateTimeWindow = 4 * futureDataTmWin;	// 2hr default time window allowed for valid times
  filterBadDate = ignoreFutureData;
  // only applies if ignoreFutureData flag is set
  //    strcpy(VolFilePath, "");
  //    VolFileStn = 0;	// by default don't write volume files
  //    VolFileUseStnID = false;
  //    VolFileCreateLatestDataInfo = false;
  lock = new spinlock("DBMng->lock", 500);	// 5 secs
  if (inifilename && FileExists(inifilename))
    file = fopen(inifilename,"r");
  else
    {
      if (FileExists(rpdbinifile1))
	file = fopen(rpdbinifile1,"r");
      else if (FileExists(rpdbinifile2))
	file = fopen(rpdbinifile2,"r");
    }
  maxdatasz = 0;
  if ((Str3 = getenv("RPDBSIZE"))) {
    if ((MaxDBSz = atoi(Str3)) > 0)  {
      fprintf(stderr,"DBMngr::DBMngr - Environment setting RPDBSIZE = %dMB\n",MaxDBSz);
      maxdatasz = MaxDBSz*1000000;
    }
  }
  AllowReplayMode = true;	// allow replay mode, but only write replay scans to files
  DBCheckTime = 0;
  DBCheckPeriod = 5;	// default 5 seconds loop delay
  if (file) {
    while (fgets(Str1, 256, file)) {
      if (Str1[0] != '#') {
	if ((args = sscanf(Str1," pathname=%s",pnamestr)) >= 1) { 
	  if (strstr(Str1,"READONLY")) {
	    readonly = TRUE;
	  }
	  else readonly = FALSE;
	  if (strstr(Str1,"DELALLOLD")) {
	    DeleteOldDatabases = TRUE;
	  }
	  else DeleteOldDatabases = FALSE;
	  if ((Str3 = strstr(Str1, "RPDBSIZE="))) {
	    if ((sscanf(Str3, "RPDBSIZE=%d", &MaxDBSz)) == 1)
	      maxdatasz = MaxDBSz * 1000000;
	  }
	  if ((Str3 = strstr(Str1, "DBPURGEDEPTH="))) {
	    if ((sscanf(Str3, "DBPURGEDEPTH=%d", &DBPurgeDepth)) < 1)
	      DBPurgeDepth = -1;
	  }
	  else DBPurgeDepth = -1;
	  if (!firstdb_ok)
	    {
	      sprintf(Str1,"DBMng::DBMng - Adding primary database: pathname=%s ReadOnly=%d \n",pnamestr,readonly);
	      RapicLog(Str1, LOG_ERR);
	    }
	  else
	    {
	      sprintf(Str1,"DBMng::DBMng - Adding secondary database: pathname=%s ReadOnly=%d \n",pnamestr,readonly);
	      RapicLog(Str1, LOG_ERR);
	    }
	  if (DBPurgeDepth > 0)
	    fprintf(stderr,"DBMng::DBMng - Database purge depth set to %d\n", DBPurgeDepth);
	  newdb = new rp_db(pnamestr);
	  newdb->SetDBReadOnlyMode(readonly);
	  if (maxdatasz) newdb->maxdatasz = maxdatasz;
	  newdb->DBPurgeDepth = DBPurgeDepth;
	  newdb->DeleteOldDatabases = DeleteOldDatabases;
	  strcpy(newdb->ArchiveDevice, ArchiveDevice);
	  if (allowdbpurge)
	    newdb->PurgeOldDBs();	    // purge databases if needed
	  AddDB(newdb);
	  firstdb_ok = TRUE;
	}
	else if (strstr(Str1, "rpdbCache="))
	  {
	    openRpdbCache(Str1);
	  }		    
	else if (strstr(Str1, "initial_db_load_path="))
	  {
	    if ((args = sscanf(Str1,"initial_db_load_path=%s",pnamestr)) >= 1) { 
	      fprintf(stderr,"DBMng::DBMng - Opening initial_db_load_path=%s\n",pnamestr);
	      InitLoadDB = new rp_db(pnamestr,true);
	    }
	  }
	else if (sscanf(Str1, "archivedevice=%s", ArchiveDevice))
	  fprintf(stderr,"DBMng::DBMng - Archive device set to %s\n",ArchiveDevice);
	else if (strstr(Str1, "volfilepath")) {
	  fileEntry.init(Str1);
	  rapicFileEntries.push_back(fileEntry);
	}
	else if (strstr(Str1, "no_rapic_db")) {
	  noRapicDB = true;
	}
	else if (strstr(Str1, "filter_bad_date")) {
	  filterBadDate = true;
	}
	else if ((Str3 = strstr(Str1, "bad_date_window="))) {
	  sscanf(Str3, "bad_date_window=%d", &tempint);
	  badDateTimeWindow = tempint;
	}
	else if ((Str3 = strstr(Str1, "loop_delay="))) {
	  sscanf(Str3, "loop_delay=%d", &tempint);
	  DBCheckPeriod = tempint;
	}		    
	else if ((Str3 = strstr(Str1, "maxLoadRealTimeHours="))) {
	  sscanf(Str3, "maxLoadRealTimeHours=%d", &tempint);
	  maxLoadRealTimeSecs = tempint * 3600;
	}		    
	else if ((Str3 = strstr(Str1, "maxLoadRealTimeGap="))) {
	  sscanf(Str3, "maxLoadRealTimeGap=%d", &tempint);
	  maxLoadRealTimeGap = tempint * 3600;;
	}		    
	else if ((Str3 = strstr(Str1, "maxLoadDBGap="))) {
	  sscanf(Str3, "maxLoadDBGap=%d", &tempint);
	  maxLoadDBGap = tempint * 3600;
	}		    
	else if ((Str3 = strstr(Str1, "oldstyle_rapicdb_names"))) {
	  RPDBNAME_STN_LS = false;
	} 
	else if (sscanf(Str1," serveRecentDataMins=%d",&tempint) == 1) {
	  sprintf(LogStr,"DBMng::DBMng - serveRecentDataMins set to %d minutes\n",tempint);
	  RapicLog(LogStr, DfltLogLevel);
	  if (ScanMng)
	    ScanMng->setRecentCachePeriod(tempint);
	}
      }
    }
    fclose(file);
  }
  if (!firstdb_ok && !noRapicDB) {
    sprintf(Str1, 
	    "DBMng::DBMng - firstdb_ok %s failed, trying in ./",
	    pnamestr);
    RapicLog(Str1, LOG_ERR);
    strcpy(pnamestr ,"./");
    fprintf(stderr,"DBMng::DBMng - Adding primary database: pathname=%s\n",pnamestr);
    newdb = new rp_db(pnamestr);
    AddDB(newdb);
  }
}
	
DBMng::~DBMng() {
  rp_db *TempDB;
  rdr_scan_node *temp,*next;
    
  lock->get_lock();
  temp = checkedscans;
  while (temp) {
    lock->rel_lock();
    AddScan(temp->scan);		// try to add partial scans
    lock->get_lock();
    next = temp->next;
    delete temp;
    decScanNodeCount();
    temp = next;
  }
  while (FirstDB) {
    TempDB = FirstDB->next;
    delete FirstDB;
    FirstDB = TempDB;
  }
  if (rpdbCache)
    delete rpdbCache;
  rapicFileEntries.erase(rapicFileEntries.begin(), rapicFileEntries.end());
  lock->rel_lock();
  delete lock;
  lock = 0;
}
		
void DBMng::AddDB(rp_db *newdb) {
  lock->get_lock();
  newdb->prev = LastDB;
  newdb->next = 0;
  if (LastDB)
    LastDB->next = newdb;		// list exists, append new
  else FirstDB = newdb;			// else new list, new is first
  LastDB = newdb;
  lock->rel_lock();
}
	
/*
  bool DBMng::VolFileName(char *namestr, char *pathstr, rdr_scan *scan) {
  struct tm Gmtime;

  if (!namestr || !pathstr || !scan) {
  fprintf(stderr, "DBMng::VolFileName - FAILED\n");
  return false;
  }
  else
  {	
  gmtime_r(&scan->scan_time_t, &Gmtime);
  if (VolFileUseStnID)
  sprintf(namestr, "%s%04d%02d%02d%02d%02d%03d", 
  pathstr, 
  Gmtime.tm_year + 1900, 
  Gmtime.tm_mon + 1, 
  Gmtime.tm_mday, 
  Gmtime.tm_hour, 
  Gmtime.tm_min, 
  scan->station
  );			
  else
  sprintf(namestr, "%s%04d%02d%02d%02d%02d%s", 
  pathstr, 
  Gmtime.tm_year + 1900, 
  Gmtime.tm_mon + 1, 
  Gmtime.tm_mday, 
  Gmtime.tm_hour, 
  Gmtime.tm_min, 
  stn_name(scan->station)
  );	
  }
  return true;
  }
*/

bool DBMng::AddScan(rdr_scan *addscan) {
  rp_db *TempDB;
  bool FirstOK = FALSE;
  bool OK;
  char errstr[512];
    
  //    if (!FirstDB)
  //	return TRUE;
  if (!addscan || !addscan->station || (addscan->scan_time_t < 0)) {
    fprintf(stderr,"DBMng::AddScan - ERROR INVALID SCAN\n");
    return TRUE;
  }
  if ((addscan->data_source == COMM) &&
      filterBadDate && 
      (abs(addscan->scan_time_t - time(0)) > badDateTimeWindow)) {
    sprintf(errstr,"DBMng::AddScan - SCAN DATE/TIME OUTSIDE %dSEC WINDOW - %s\n", 
	    int(badDateTimeWindow), addscan->ScanString());
    RapicLog(errstr, LOG_ERR);
    return TRUE;
  }
  if (!(addscan->data_source == REPLAY))  // don't write replay scans into rapic databases
    {
      lock->get_lock();
      TempDB = FirstDB;
      while (TempDB) {
	if (!(OK = TempDB->AddScan(addscan))) // if failed, try again
	  /*OK = TempDB->AddScan(addscan)*/;	// on second thoughts, don't, it may add twice to data file if isam err
	if (TempDB == FirstDB) FirstOK = OK;
	TempDB = TempDB->next;
      }
      if (!noRapicDB && !FirstOK && FirstDB) {
	fprintf(stderr,"DBMng::AddScan - UNABLE TO ADD IMG TO MAIN DB. ATTEMPTING RESET OF MAIN DB\n");	
	FirstDB->Close();
	FirstDB->OpenDB();
	FirstOK = FirstDB->AddScan(addscan);
	if (!FirstOK)
	  fprintf(stderr,"DBMng::AddScan - *****SERIOUS ERROR - UNABLE TO ADD SCAN*******\n");
      }
    }
  if (!addscan->DBDuplicate)
    CheckRapicFileWrite(addscan);  // DO write duplicate scans into rapic files, as feeds to other systems
  /*
    if ((VolFileStn != 0) && !addscan->DBDuplicate) {
    if ((VolFileStn < 0) || (VolFileStn == addscan->station))
    WriteVolFile(addscan);
    }
  */
  lock->rel_lock();
#ifdef TITAN
#ifndef USE_TITAN_THREAD
  if ( FirstOK && useTitan && !addscan->DBDuplicate && 
       !FirstDB->showDBScansToScanMng &&
       (useTitan == addscan->station) ) {
    int debug = 0;
    rdr_scan *tmpscan = addscan->RootScan();
    int ncount = 1;
    if ( debug )
      {
	while ( tmpscan ) {
	  fprintf(stderr,"From Addscan to TITAN %d \n",ncount);
	  tmpscan = tmpscan->NextScan(tmpscan);
	  ncount++;
	}
	fprintf(stderr,"Total sets are %d \n",ncount -1 );
      }
    fprintf(stdout,"RealTime AddScan Total Scans are %d\n",ncount -1);
    rapicToTitan(addscan->RootScan(),lastinSet);
  }
#endif
#endif
  return FirstOK;
}
	
int DBMng::NewDataAvail(rdr_scan *newscan) {
  //    rdr_scan_node*  temp_scan_node;
    
  if (!DBReadOnly) {	    // if read only do bother with new scans
    lock->get_lock();
    scan_client::NewDataAvail(newscan);
    lock->rel_lock();
    return 1;
  }
  else return 0;
}
    	
bool DBMng::ScanSourceOK(rdr_scan *srcscan)
{
  bool OK = false;

  if (DataMode == REALTIMEMODE)
    switch (srcscan->data_source)
      {
      case COMM:
      case COMMREQ:
      case PROD_ACCUM:
      case PROD_XLAT:
      case RADARCONVERT:
	OK = true;
	break;
      default:
	OK = false;
	break;
      }
  if ((DataMode == REPLAYMODE) &&        // also allow replay scans in replay mode
      (srcscan->data_source == REPLAY))
    OK |= true;
  return OK;
}


/*
 * DBMng::IsDuplicate is currently the authoritative
 * IsDuplicate scan client.
 */

bool DBMng::IsDuplicate(rdr_scan *new_scan, 	// return true if known ie already in seq or database etc
			bool FinishedOnly) {
  bool present = FALSE;
  rp_db *TempDB;
  rdr_scan_node*  temp_scan_node;
    
  lock->get_lock();
  TempDB = FirstDB;
  while (!present && TempDB) {
    present = TempDB->ScanPresent(new_scan);
    TempDB = TempDB->next;
  }
  if (!FinishedOnly) {
    temp_scan_node = newscans;	// check incomplete scans also
    while (temp_scan_node && !present) {
      present = new_scan->ScanSame(temp_scan_node->scan);
      temp_scan_node = temp_scan_node->next;
    }
  }
  lock->rel_lock();
  return present;
}

bool DBMng::Full(int passedSize) {
  return FALSE;
}
	
bool DBMng::MergeScans() {
  return FALSE;
}

bool DBMng::CheckScans() {	    // return TRUE if all Incomplete scans processed
  rdr_scan_node *temp,*next;
  int	    maxscanstowrite = 4;   // maximum scans per check call
    
  if (!DBReadOnly) {	    // if read only do bother with new scans
    lock->get_lock();
    temp = newscans;
    while (temp) {
      next = temp->next;
      if (temp->scan->Complete() || temp->scan->Finished()) {
	//scan is finished, move to checked scans list or delete if faulty
	// remove from newscans list
	temp->remove_from_list();
	// move to checked scans list
	temp->next = checkedscans;	
	temp->prev = 0;
	if (checkedscans)
	  checkedscans->prev = temp;
	checkedscans = temp;
	if (temp == newscans) 
	  newscans = next;	// removed first in list, point to new head
      }
      temp = next;
    }
    lock->rel_lock();
  }
  /*
   * add checked scans to database
   */
  temp = checkedscans;
  while (temp && maxscanstowrite) {
    next = temp->next;
    AddScan(temp->scan);		// add completed scan
    if (temp == checkedscans) 
      checkedscans = next;
    delete temp;			// remove from checked list
    decScanNodeCount();
    maxscanstowrite--;
    temp = next;
  }
  if (checkedscans) return FALSE;	// still scans to add to db, return FALSE
  else return TRUE;			// no scans left to add to db, return TRUE
}
	
void DBMng::SetDBReadOnlyMode(bool SetDBReadOnly) {
  if (!FirstDB)
    return;
  DBReadOnly = SetDBReadOnly;
  FirstDB->SetDBReadOnlyMode(SetDBReadOnly);
}

bool	DBMng::GetDBReadOnlyMode() {
  return DBReadOnly;
}

void DBMng::PrintScanUsage(FILE *file, bool verbose) {
  int newcount = 0, newsize = 0, 
    checkedcount = 0, checkedsize = 0;
  if (!file) file = stderr;
  if (lock) lock->get_lock();
  if (newscans) {
    newcount = newscans->ListCount();
    newsize = newscans->ListSize();
  }
  if (checkedscans) {
    checkedcount = checkedscans->ListCount();
    checkedsize = checkedscans->ListSize();
  }
  fprintf(file, "DBMng::PrintScanUsage, newscans: %d:%1.3fMB,  CompleteScans %d:%1.3fMB\n", 
	  newcount, newsize/1000000.0, 
	  checkedcount, checkedsize/1000000.0);
  if (lock) lock->rel_lock();
}

void DBMng::CheckRapicFileWrite(rdr_scan *addscan)
{
  vector<RapicFileEntry>::iterator thisentry, lastentry;
    
  thisentry = rapicFileEntries.begin();
  lastentry = rapicFileEntries.end();
  while (thisentry < lastentry)
    {
      if (thisentry->scanMatch(addscan))
	thisentry->WriteRapicFile(addscan);
      thisentry++;
    }
}

/*
  Not complete
  Need to implement proper shared record locking for this to work
*/
void DBMng::loadInitialSeq(scan_client *ScanClient, int num, bool closeAfterLoad)
{
  if (InitLoadDB)
    {
      fprintf(stdout, "DBMng::loadInitialSeq - Loading from Initial Database = %s\n",
	      InitLoadDB->pname);
      InitLoadDB->LoadTo(ScanClient, true, num);
      if (closeAfterLoad)
	closeInitLoadDB();
    }
  else if (FirstDB)
    FirstDB->LoadTo(ScanClient, true, num);
}

void DBMng::closeInitLoadDB()
{
  if (InitLoadDB)
    {
      InitLoadDB->Close();
      delete InitLoadDB;
      InitLoadDB = NULL;
    }
}

void DBMng::openRpdbCache(char *cacheinitstr)
{
  if (!rpdbCache)
    rpdbCache = new rpdb_cache(cacheinitstr);
  else
    rpdbCache->open(cacheinitstr);
}

void DBMng::openRpdbCache(string cachepath, string cachelist, bool allowrebuild)
{
  if (!rpdbCache)
    {
      fprintf(stdout, "DBMng::openRpdbCache - creating cache from %s %s\n",
	      cachepath.c_str(), cachelist.c_str());
      rpdbCache = new rpdb_cache(cachepath, cachelist, allowrebuild);
    }
  else
    {
      fprintf(stdout, "DBMng::openRpdbCache - opening cache from %s %s\n",
	      cachepath.c_str(), cachelist.c_str());
      rpdbCache->open(cachepath, cachelist, allowrebuild);
    }
}


RapicFileEntry::RapicFileEntry(char *initstr)
{
  init(initstr);
}

/*
 * volfilepath=/pathname/ [volfilestn=nn] [volfilesuffix=.suffix] 
 [filename_use_stnid] [filename_use_scantype] 
 [filename_use_datepath] [filename_use_stndatepath]
 [create_latest_data_info] [volumeonly] [compscanonly]
 [permissions=oooo] where oooo is octal permissions
 [owner=uid] [group=gid]
 [verbose=0/1]
*/

void RapicFileEntry::init(char *initstr)
{

  char	*tempstr;
  char	stnstr[64];
  unsigned int	tempmode_t;

  strcpy(FilePath, "./rapicfiles/");
  strcpy(DatePath, "");
  strcpy(FileSuffix, volFileSuffix);    
  FileStn = -1;
  FileNameUseStnID = false;
  FileNameUseScanType = false;
  useDatePathHierarchy = false;
  useStnDatePathHierarchy = false;
  VolumeOnly = false;
  CompScanOnly = false;
  writeReflOnly = false;
  verbose = true;
  CreateLatestDataInfo = false;
  CreateFileList = false;
  permissions = 0644;
  owner = uid_t(-1);	    // will use default owner
  group = gid_t(-1);	    // will use default group
  if (initstr)
    {
      if (sscanf(initstr, "volfilepath=%s", FilePath) == 1) 
	{
	  if (FilePath[strlen(FilePath)-1] != '/')
	    strcat(FilePath, "/");
	  if ((tempstr = strstr(initstr, "volfilestn=")) )
	    {
	      sscanf(tempstr, "volfilestn=%s", stnstr);
	      FileStn = decode_stnstr(stnstr);
	    }		    
	  if (strstr(initstr, "filename_use_stnid"))
	    FileNameUseStnID = true;
	  if (strstr(initstr, "filename_use_datepath"))
	    useDatePathHierarchy = true;
	  if (strstr(initstr, "filename_use_stndatepath"))
	    useStnDatePathHierarchy = true;
	  if (strstr(initstr, "filename_use_scantype"))
	    FileNameUseScanType = true;
	  if (strstr(initstr, "create_latest_data_info"))
	    CreateLatestDataInfo = true;
	  if (strstr(initstr, "create_filelist"))
	    CreateFileList = true;
	  if (strstr(initstr, "volumeonly"))
	    VolumeOnly = true;
	  if (strstr(initstr, "compscanonly"))
	    CompScanOnly = true;
	  if ((tempstr = strstr(initstr, "volfilesuffix=")))
	    {
	      sscanf(tempstr, "volfilesuffix=%s", FileSuffix);
	    }
		
	  if ((tempstr = strstr(initstr, "permissions=")))
	    {
	      if (sscanf(tempstr, "permissions=%o", &tempmode_t) == 1)
		permissions = tempmode_t;	// don't know mode_t type for scanf
	    }
	  if ((tempstr = strstr(initstr, "user="))) 
	    {
	      sscanf(tempstr, "owner=%d", &tempmode_t);
	      owner = uid_t(tempmode_t);
	    }		    
	  if ((tempstr = strstr(initstr, "group=")))
	    {
	      sscanf(tempstr, "group=%d", &tempmode_t);
	      group = gid_t(tempmode_t);
	    }		    
	  if ((tempstr = strstr(initstr, "verbose=")))
	    {
	      sscanf(tempstr, "verbose=%d", &tempmode_t);
	      verbose = tempmode_t;
	    }		    
	  if (strstr(initstr, "reflonly"))
	    writeReflOnly = true;
		
	  fprintf(stderr,"RapicFileEntry::init - VolFilePath set to %s, VolFileStn=%d\n",FilePath, FileStn);
	}
      else
	strcpy(FilePath, "./rapicfiles/");
    }
}
	
char* RapicFileEntry::datePath(rdr_scan *addscan)
{
  if (!addscan)
    {
      strcpy(DatePath, "");
      return DatePath;
    }
  if (useStnDatePathHierarchy)
    return stnDatePath(addscan);
  if (useDatePathHierarchy)
    {
      sprintf(DatePath, "%04d/%02d/%02d/", addscan->year, addscan->month, addscan->day);
    }
  else
    strcpy(DatePath, "");
  return DatePath;
}

char* RapicFileEntry::stnDatePath(rdr_scan *addscan)
{
  if (!addscan)
    {
      strcpy(DatePath, "");
      return DatePath;
    }
  if (useStnDatePathHierarchy)
    {
      sprintf(DatePath, "%s/%04d/%02d/%02d/", stn_name(addscan->station, true), addscan->year, addscan->month, addscan->day);
    }
  else
    strcpy(DatePath, "");
  return DatePath;
}

/*
  if useDatePathHierarchy is set, the LatestDataInfoFile is still written to the root of the 
  root of the datepath heirarchy
*/
void RapicFileEntry::CreateLatestDataInfoFile(rdr_scan *addscan)
{
  FILE *tempfile;
  char fname[512], tempfname[512];
  char volfname[512]; 
//	realvolfname[512];
    
  if (!CreateLatestDataInfo)
    return;
  sprintf(fname, "%s%s", FilePath, latestDataInfoFilename);
  strcpy(tempfname, fname);
  strcat(tempfname, ".temp");
/*
  VolFileName(volfname, addscan);
  realpath(volfname, realvolfname);
*/
  VolFileName(volfname, addscan, true, permissions);  // suppressed path prefix version
  tempfile = fopen(tempfname, "w");
  if (tempfile)
    {
      fprintf(tempfile, "%d %04d %02d %02d %02d %02d %02d\n"
	      "%s\n"
	      "%s\n"
	      "%s%s\n"
	      "0", 
	      int(addscan->scan_time_t), 
	      addscan->year, addscan->month, addscan->day, 
	      addscan->hour, addscan->min, addscan->sec,
	      &FileSuffix[1],  
	      volfname, 
	      volfname, FileSuffix);
//	      realvolfname, 
//	      realvolfname, FileSuffix);
    }
  else
    {
      fprintf(stderr, "RapicFileEntry::CreateLatestDataInfoFile - ERROR opening %s\n", 
	      tempfname);
      perror(0);
      return;
    }
  fclose(tempfile);
  rename(tempfname, fname);
  chmod(fname, permissions);
  if ((int(owner) > -1) || (int(group) > -1))
    chown(fname, owner, group);
  if (verbose)
    fprintf(stdout, "RapicFileEntry::CreateLatestDataInfoFile - Wrote successfully\n %s\n", 
	    fname);
}

/*
  if useStnDatePathHierarchy is set, the LatestDataInfoFile is still written to the datepath directory  
  of this scan
*/
void RapicFileEntry::UpdateFileList(rdr_scan *addscan)
{
  FILE *tempfile;
  char fname[512], tempfname[512];
  char volfname[512], tempstr[64], tempstr2[64];
  char *path;
    
  if (!CreateFileList || !(useDatePathHierarchy || useStnDatePathHierarchy)) // only allow filelist if date path heirarchy used
    return;
  VolFileName(volfname, addscan, false, permissions);  // get full file name again
  strcpy(tempfname, volfname); 
  path = dirname(tempfname);
  sprintf(fname, "%s/%s", path, fileListName);
  tempfile = fopen(fname, "a");
  if (tempfile)
    {
      fprintf(tempfile, "%s%s\n", 
	      basename(volfname), FileSuffix);
    }
  else
    {
      fprintf(stderr, "RapicFileEntry::CreateFileList - ERROR opening %s\n", 
	      fname);
      perror(0);
      return;
    }
  fclose(tempfile);
  chmod(fname, permissions);
  if ((int(owner) > -1) || (int(group) > -1))
    chown(fname, owner, group);
  if (verbose)
    fprintf(stdout, "RapicFileEntry::CreateFileList - Wrote successfully\n %s\n", 
	    fname);
  sprintf(fname, "%s/%s", path, fileListTime);
  strcpy(tempfname, fname);
  strcat(tempfname, ".temp");
  tempfile = fopen(fname, "w");
  if (tempfile)
    {
      fprintf(tempfile, "update_time_t=%d\nupdate_time=%s\n"
	      "scan_time_t=%d\nscan_time=%s\n", 
	      int(time(0)), TimeString(time(0), tempstr, true, true),
	      int(addscan->scan_time_t), 
	      TimeString(addscan->scan_time_t, tempstr2, true, true)
	      );
    }
  else
    {
      fprintf(stderr, "RapicFileEntry::CreateFileList - ERROR opening %s\n", 
	      fname);
      perror(0);
      return;
    }
  fclose(tempfile);
  rename(tempfname, fname);
  chmod(fname, permissions);
  if ((int(owner) > -1) || (int(group) > -1))
    chown(fname, owner, group);
}

bool RapicFileEntry::scanMatch(rdr_scan *addscan)
{
  return (addscan &&
	  ((FileStn == -1)|| (addscan->station == FileStn)) &&
	  (!VolumeOnly || (addscan->scan_type == VOL)) &&
	  (!CompScanOnly || (addscan->scan_type == CompPPI) || (addscan->scan_type == PPI)));
}

bool RapicFileEntry::VolFileName(char *namestr, rdr_scan *scan, bool nopath, mode_t perms) {
  char localpath[256] = "";
  mode_t dirperms = perms | 0111;  // ensure executable perms for dir creation

  if (!namestr || !scan) {
    fprintf(stderr, "DBMng::VolFileName - FAILED\n");
    return false;
  }
  else
    {	
      if (!nopath)
	{
	  strcpy(localpath, FilePath);
	  strcat(localpath, datePath(scan));
	  if (strlen(localpath) && !DirExists(localpath, true, true, dirperms))
	    fprintf(stdout, "RapicFileEntry::VolFileName - Error creating directory\n  %s\n", 
		    localpath);
	}
      else
	{
          strcat(localpath, datePath(scan));
          if (strlen(localpath) && !DirExists(localpath, true, true, dirperms))
            fprintf(stdout, "RapicFileEntry::VolFileName - Error creating directory\n  %s\n",
                    localpath);
        }

      if (FileNameUseStnID)
	sprintf(namestr, "%s%04d%02d%02d%02d%02d%03d", 
		localpath, 
		scan->year, scan->month, scan->day, 
	        scan->hour, scan->min,
		scan->station
		);			
      else
	sprintf(namestr, "%s%04d%02d%02d%02d%02d%s", 
		localpath, 
		scan->year, scan->month, scan->day, 
	        scan->hour, scan->min,
		stn_name(scan->station)
		);	
      if (FileNameUseScanType)
	strcat(namestr, get_scan_type_text(scan->scan_type));
    }
  return true;
}

void RapicFileEntry::WriteRapicFile(rdr_scan *addscan) {
  char fname[512], tempfname[512];
    
  if (!scanMatch(addscan))
    return;
  if (VolFileName(fname, addscan, false, permissions))
    {
      strcat(fname, FileSuffix);	// add suffix
      strcpy(tempfname, fname);
      strcat(tempfname, ".temp");
      if (addscan->write_scan_set(tempfname, writeReflOnly) == 0)
	{
	  fprintf(stderr, "RapicFileEntry::WriteRapicFile - WRITE FAILED\n %s\n", 
		  fname);
	  return;
	}
      rename(tempfname, fname);
      chmod(fname, permissions);
      if ((int(owner) > -1) || (int(group) > -1))
	chown(fname, owner, group);
      if (verbose)
	fprintf(stdout, "RapicFileEntry::WriteRapicFile - Wrote successfully\n %s\n", 
		fname);
    }
  if (CreateLatestDataInfo)
    CreateLatestDataInfoFile(addscan);
  if (CreateFileList)
    UpdateFileList(addscan);
}

rpdb_cachedata::rpdb_cachedata(string dbname, time_t starttime, time_t endtime, stnSet stnset)
{
  db_name = dbname;
  start_time = starttime;
  end_time = endtime;
  rec_count = 0;
  datasize = 0;
  stn_set = stnset;
}

rpdb_cachedata::rpdb_cachedata()
{
  start_time = 0;
  end_time = 0;
  rec_count = 0;
  datasize = 0;
}  

rpdb_cachedata::rpdb_cachedata(string dbname, rp_isam *rpdb)
{
  getCacheData(dbname, rpdb);
}

void rpdb_cachedata::getCacheData(string dbname, rp_isam *rpdb)
{
  db_name = dbname;
  if (rpdb)
    {
      start_time = rpdb->FirstRecTime();
      end_time = rpdb->LastRecTime();
      rpdb->getStnSet(&stn_set);
      rec_count = rpdb->RecCount();
      datasize = rpdb->datasize;
    }
}

void rpdb_cachedata::dumpData(FILE *file, bool printStnNames)
{
  char startTimeStr[256];
  char endTimeStr[256];
  if (!file) file = stdout;
  if (file)
    {
      fprintf(file, "%s -> %s Recs=%ld Stns=%d\n", 
	      TimeString(start_time, startTimeStr, true, true), 
	      TimeString(end_time, endTimeStr, true, true), rec_count, stn_set.stnCount());
      int y = 0;
      if (printStnNames)
	for (int x = 0; x < stn_set.size(); x++)
	  {
	    if (stn_set.isSet(x))
	      {
		fprintf(file, "%s, ", stn_name(x));
		y++;
	      }
	    if (y == 12)
	      {
		fprintf(file, "\n");
		y = 0;
	      }
	  }
    }
  fprintf(file, "\n");
}

rpdb_cache::rpdb_cache(string pathname, string listname, bool allowrebuild)
{
  allowRebuild = allowrebuild;
  open(pathname, listname, allowrebuild);
}

rpdb_cache::rpdb_cache(char *initstr)
{
  char pathstr[256], liststr[128];
  if (initstr && strstr(initstr, "allowRebuild"))
    allowRebuild = true;
  if (initstr && sscanf(initstr, "rpdbCache=%s %s", pathstr, liststr) == 2)
    loadDbList(pathstr, liststr);
}

rpdb_cache::~rpdb_cache()
{
}

void rpdb_cache::close()
{
  startTimes.clear();
  endTimes.clear();
  cachedData.clear();
  cacheStartTime = cacheEndTime = 0;
  totalRecs = 0;
  totalDBSize = 0;
  allowRebuild = false;
}

void rpdb_cache::open(string pathname, string listname, bool allowrebuild)
{
  close();
  char pathstr[256], liststr[128];
  fprintf(stdout, "rpdb_cache::open - Opening %s %s\n",
	  pathname.c_str(), listname.c_str());
  sscanf(pathname.c_str(), "cache_path=%s", pathstr);
  sscanf(listname.c_str(), "cachedb_list=%s", liststr);
  allowRebuild = allowrebuild;
  loadDbList(pathstr, liststr);
}

void rpdb_cache::open(char *initstr)
{
  char pathstr[256], liststr[128];
  close();
  fprintf(stdout, "rpdb_cache::open - Opening %s\n",
	  initstr);
  if (initstr && strstr(initstr, "allowRebuild"))
    allowRebuild = true;
  if (initstr && sscanf(initstr, "rpdbCache=%s %s", pathstr, liststr) == 2)
    loadDbList(pathstr, liststr);
}

void rpdb_cache::loadDbList(string pathname, string listname)   // load databases in list file
{

  string filenm = pathname + listname;
  char dbstr[512];
  rp_isam *rpisam = new rp_isam(true);   // open read_only mode
  path_name = pathname;
  FILE *file = fopen(filenm.c_str(), "r");
  if (!file)
    {
      fprintf(stdout, "rpdb_cache::loadDbList - failed opening dblist %s\n",
	      filenm.c_str());
      perror(0);
      return;
    }
  while (fgets(dbstr, 512, file))
    {
      if (dbstr[0] != '#')
	{
	  rpisam->ReadOnly = !allowRebuild;
	  addToCache(pathname, dbstr, rpisam);
	}
    }
  rpisam->Close();
  fclose(file);
  delete rpisam;
}

bool rpdb_cache::open_rp_isam(rp_isam *rpisam, 
			      const char *pathname, 
			      const char *dbname)
{
  bool openOK = false;
  if (dbname[0] == '/') // absolute pathname, don't prefix pathname
    openOK = rpisam->Open("", dbname, !allowRebuild, allowRebuild);
  else 
    openOK = rpisam->Open(pathname, dbname, !allowRebuild, allowRebuild);
  
  if (!openOK)  // try pathname with tail of dbname
    {
      char *tempstr = strdup(dbname);
      openOK = rpisam->Open(pathname, basename(tempstr), !allowRebuild, allowRebuild);
      free(tempstr);
    }
  return openOK;
}

void rpdb_cache::addToCache(string pathname, string dbname, rp_isam *rpisam)
{

  if (!rpisam) return;
  rpdb_cachedata *iter;
  rpisam->Close();

  bool openOK = open_rp_isam(rpisam, pathname.c_str(), dbname.c_str());
  
  if (openOK)
    {
      iter = &(cachedData[dbname]);
      iter->getCacheData(dbname, rpisam);
      startTimes.insert(make_pair(iter->start_time, dbname));
      endTimes.insert(make_pair(iter->end_time, dbname));
      if (!cacheStartTime || (iter->start_time < cacheStartTime))
	cacheStartTime = iter->start_time;
      if (!cacheEndTime || (iter->end_time > cacheEndTime))
	cacheEndTime = iter->end_time;
      totalRecs += iter->rec_count;
      totalDBSize += iter->datasize;
      stn_set.merge(iter->stn_set);
      fprintf(stdout, "rpdb_cache::addToCache - Succeeded opening database - %s%s TotalRecs=%ld TotalSizeSoFar=%lldMB\n", 
	    pathname.c_str(), dbname.c_str(), totalRecs, (totalDBSize/1000000));
      iter->dumpData(stdout);
    }
  else
    fprintf(stderr, "rpdb_cache::addToCache - Failed to open database - %s%s\n", 
	    pathname.c_str(), dbname.c_str());
  rpisam->Close();
}
      
void rpdb_cache::dumpCacheContents(char *filenm)
{
  FILE *file;

  if (filenm)
    file = fopen(filenm, "w");
  else
    file = stdout;
  
  multimap<time_t, string>::iterator start_iter = startTimes.begin();  // keep map of starttime vs db name for time period searches
  multimap<time_t, string>::iterator start_iter_end = startTimes.end();  // keep map of starttime vs db name for time period searches

  multimap<time_t, string>::iterator end_iter = endTimes.begin();    // keep map of endtime vs db name for time period searches
  multimap<time_t, string>::iterator end_iter_end = endTimes.end();    // keep map of endtime vs db name for time period searches

  map <string, rpdb_cachedata>::iterator data_iter = cachedData.begin();
  map <string, rpdb_cachedata>::iterator data_iter_end = cachedData.end();
  char timeStr[256];

  if (file)
    {
      fprintf(file, "Cached Data Listing - Database Count=%d Total Records=%ld \nTotal Data Size=%lldMB\n", 
	      int(cachedData.size()), totalRecs, totalDBSize/1000000);
      char startTimeStr[256];
      char endTimeStr[256];
      fprintf(file, "%s -> %s Stns=%d\n\n", 
	      TimeString(cacheStartTime, startTimeStr, true, true), 
	      TimeString(cacheEndTime, endTimeStr, true, true), stn_set.stnCount());
      while (data_iter != data_iter_end)
	{
	  fprintf(file, "dbname=%s ", data_iter->first.c_str());
	  data_iter->second.dumpData(file);
	  data_iter++;
	}
      fprintf(file, "\n\nStart Times Listing - Entries=%d\n\n", 
	      int(startTimes.size()));
      while (start_iter != start_iter_end)
	{
	  fprintf(file, "%s dbname=%s\n", 
		  TimeString(start_iter->first, timeStr, true, true), 
		  start_iter->second.c_str()); 
	  start_iter++;
	}
      fprintf(file, "\n\nEnd Times Listing - Entries=%d\n\n", 
	      int(endTimes.size()));
      while (end_iter != end_iter_end)
	{
	  fprintf(file, "%s dbname=%s\n", 
		  TimeString(end_iter->first, timeStr, true, true), 
		  end_iter->second.c_str()); 
	  end_iter++;
	}
      if (filenm) fclose(file);
    }
}

void rpdb_cache::copyDB(rp_copy_req& copyreq)
{
  map <string, rpdb_cachedata>::iterator iter;
  map <string, rpdb_cachedata>::iterator iter_end;
  map <string, rpdb_cachedata> matchingdbs;
  
  char startTimeStr[256];
  char endTimeStr[256];
  int files=0;
  int destfname_inc = 0;
  string destincname;
  char destincstr[16];
  bool copytodestcompleted = true;
  bool failed = false;
  bool ends_before, 
    starts_after;
  long totalRecsCopied = 0, recscopied;

  rp_isam *srcdb = new rp_isam(true);
  rp_isam *destdb = NULL;
  iter = cachedData.begin();
  iter_end = cachedData.end();
  fprintf(stdout, "rpdb_cache::copyDB - Req start=%s End=%s\n",
	  TimeString(copyreq.start_time, startTimeStr, true, true), 
	  TimeString(copyreq.end_time, endTimeStr, true, true));
  
  fprintf(stdout, "rpdb_cache::copyDB - Checking for databses which overlap req times\n");
  while ((iter != iter_end) && !failed)
    {
      ends_before = iter->second.end_time < copyreq.start_time;  //this db ends before starting time of rew
      starts_after = iter->second.start_time > copyreq.end_time; // this db starts after end time of req
      if (!(ends_before || starts_after)) // cache db overlaps req times
	{
	  iter->second.dumpData(stdout);
	  if (!destdb)
	    destdb = new rp_isam();
	  if (!destdb->isOpen())
	    {
	      destincname = copyreq.destFName;
	      if (destfname_inc)
		{
		  sprintf(destincstr, "_%d", destfname_inc);
		  destincname += destincstr; // append dest filename increment number
		}
	      destdb->Open(copyreq.destPath.c_str(), destincname.c_str());
	      if (!destdb->isOpen())
		destdb->Create(copyreq.destPath.c_str(), destincname.c_str());
	      failed = !destdb->isOpen();
	    }
	  if (!failed)
	    {
	      bool openOK = open_rp_isam(srcdb, path_name.c_str(), 
					 iter->second.db_name.c_str());
	      if (openOK)
		{
		  srcdb->destdb = destdb;
		  copytodestcompleted = 
		    srcdb->CopyRecs(recscopied, copyreq.stn_set, 
				    copyreq.start_time, copyreq.end_time,
				    1, copyreq.scan_type, 
				    copyreq.max_dbsize, 
				    !copytodestcompleted, true); 
		  totalRecsCopied += recscopied;
		}
	      if (copytodestcompleted)
		{
		  srcdb->Close();
		  files++;
		}
	      else 
		if (destdb &&
		    (destdb->datasize > copyreq.max_dbsize)) // destdb full
		{
		  destfname_inc++;
		  fprintf(stdout, "rpdb_cache::copyDB - Dest DB "
			  " full - opening increment %s%s_%d\n",
			  copyreq.destPath.c_str(),
			  copyreq.destFName.c_str(), destfname_inc);
		  destdb->Close();
		  delete destdb;
		  destdb = NULL;
		  srcdb->destdb = NULL;
		}
	    }
	}
      if (copytodestcompleted)
	{
	  iter++;
	}
    }
  fprintf(stdout, "rpdb_cache::copyDB - Found %d matching cache dbs\n", files);
  fprintf(stdout, "Total scans copied = %ld\n", totalRecsCopied);
  srcdb->destdb = NULL; // dereference destdb so srcdb destructor doesn't delete it
  if (srcdb)
    {
      srcdb->Close();
      delete srcdb;
    }
  if (destdb)
    {
      destdb->Close();
      delete destdb;
    }
}

void rpdb_cache::dumpStats(rp_copy_req& copyreq, char *dump_fname)
{
  if (!dump_fname)
    return;
  map <string, rpdb_cachedata>::iterator iter;
  map <string, rpdb_cachedata>::iterator iter_end;
  map <string, rpdb_cachedata> matchingdbs;
  
  char startTimeStr[256];
  char endTimeStr[256];
  int files=0;
  bool failed = false;
  bool ends_before, 
    starts_after;
  long totalRecsCopied = 0, recsmatching;
  FILE *dumpfile = fopen(dump_fname, "w");
  if (!dumpfile)
    return;
  
  rp_isam *srcdb = new rp_isam(true);
  iter = cachedData.begin();
  iter_end = cachedData.end();
  fprintf(stdout, "rpdb_cache::dumpStats - Req start=%s End=%s dump_fname=%s\n",
	  TimeString(copyreq.start_time, startTimeStr, true, true), 
	  TimeString(copyreq.end_time, endTimeStr, true, true), dump_fname);
  
  while ((iter != iter_end) && !failed)
    {
      ends_before = copyreq.start_time &&      // if start_time not defined - all end times ok
	(iter->second.end_time < copyreq.start_time);  //this db ends before starting time of rew
      starts_after = copyreq.end_time &&       // if end_time defined - all start times ok
	(iter->second.start_time > copyreq.end_time); // this db starts after end time of req
      if (!(ends_before || starts_after)) // cache db overlaps req times
	{
	  iter->second.dumpData(stdout);
	  if (!failed)
	    {
	      bool openOK = open_rp_isam(srcdb, path_name.c_str(), iter->second.db_name.c_str());
	      if (openOK)
		{
		  srcdb->dumpRecStats(recsmatching, copyreq, dumpfile, true);
		  totalRecsCopied += recsmatching;
		  srcdb->Close();
		}
	      files++;
	    }
	}
      iter++;
    }
  fprintf(stdout, "rpdb_cache::dumpStats - Found %d matching cache dbs\n", files);
  fprintf(stdout, "Total matching scans = %ld\n", totalRecsCopied);
  if (srcdb)
    {
      srcdb->Close();
      delete srcdb;
    }
  fclose(dumpfile);
}

rp_copy_req::rp_copy_req(string destpath, string destfname, int stn, 
			 e_scan_type scantype, 
			 time_t starttime, time_t endtime, 
			 int maxdbsize_mb)
{
  destPath = destpath;
  destFName = destfname;
  destDateFName = false;
  Stn = stn;
  if (stn)
    stn_set.set(stn);
  scan_type = scantype;
  start_time = starttime;
  end_time = endtime;
  max_dbsize = maxdbsize_mb * 1000000;
}

rp_copy_req::rp_copy_req(int argc, char **argv)
{
  init();
  setByArgs(argc, argv);
}

rp_copy_req::rp_copy_req()
{
  init();
}

void rp_copy_req::init()
{
  destPath = "./";
  destFName = "cachecopy";
  destDateFName = false;
  Stn = 0;
  scan_type = e_st_max;
  start_time = 0;
  end_time = 0;
  max_dbsize = 0;
  spacing = 1;
}

void rp_copy_req::setByArgs(int argc, char **argv)
{
  char tempstr[512];
  int tempint;
  for (int x = 1; x < argc; x++)
    {
      if (strlen(argv[x]))
	{
	  if (sscanf(argv[x], "destdb_path=%s", tempstr) == 1)
	    destPath = tempstr;
	  else if (sscanf(argv[x], "destdb_fname=%s", tempstr) == 1)
	    destFName = tempstr;
	  else if (sscanf(argv[x], "copy_stns=%s", tempstr) == 1)
	    {
	      fprintf(stdout, "rp_copy_req::setByArgs - stns str = %s\n", tempstr);
	      stn_set.set(tempstr);
	    }
	  else if (sscanf(argv[x], "scan_type=%s", tempstr) == 1)
	    scan_type = e_scan_type(get_scan_type(tempstr));
	  else if (sscanf(argv[x], "start_time=%s", tempstr) == 1)
	    start_time = DateTimeStr2UnixTime(tempstr);
	  else if (sscanf(argv[x], "end_time=%s", tempstr) == 1)
	    end_time = DateTimeStr2UnixTime(tempstr);
	  else if (sscanf(argv[x], "spacing=%d", &tempint) == 1)
	    spacing = tempint;
	  else if (sscanf(argv[x], "max_dbsize=%d", &tempint) == 1)
	    max_dbsize = tempint * 1000000;
	}
    }
}


#ifdef DEBUGMAIN
	
main() {
  return 0;
}
#endif // DEBUGMAIN
