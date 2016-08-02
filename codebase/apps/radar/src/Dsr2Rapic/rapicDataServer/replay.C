/*
 * 
 * replay.C
 * 
 * Implementation of the replay class
 * THis class allows a simulated real-time playback of data under 3drapic
 * It passes the scans through the scan manager, with a source 
 * enumerator of DB_REPLAY
 * This allows the clients to behave as appropriate. 
 * i.e. some clients may not choose to use the replay data
 * 
 * In the replayDataPath replay expects a file rpdb_filelist which
 * contains names (one per line) of the replay databases to be loaded
 * It also expect a flag file "ReplayEnabled" which exists only while 
 * the system is in replay mode.
 * The 
 */

#include "replay.h" 
#include "utils.h" 
#include "rdrutils.h" 
#include "uiftovrc.h"

replay *ReplayMngr = 0;

char defaultreplayinifile[] = "replay.ini";
char defaultreplayclockfile[] = "clock";
char defaultreplayenabledfile[] = "replayenabled";
int maxfirstblocktime = 900;	// limit first db read window to maxfirstblocktime seconds

replay::replay(char *replayinifile)
{
  char tempstr[256];
  FILE *inifile = 0;
  int yr, mon, dom, hr, min, sec;

  if (!replayinifile) replayinifile = defaultreplayinifile;
  strcpy(replay_ini_file_name, replayinifile);
  useInternalClock = false;
  internalTimeScale = 1.0;
  this_replay_clock_time = last_replay_clock_time = 0;
  replay_paused = false;
  replay_paused_real_time = 0;
  replayAllDbs = false;
  replay_start_time = replay_end_time = 0;
  inifile = fopen(replayinifile, "r");
  if (!inifile)
    {
      fprintf(stderr, "replay::replay - FAILED TO OPEN INI FILE (%s)\n", replayinifile);
      return;
    }
  strcpy(replay_data_path, "");
  strcpy(replay_clock_file_name, "clock");
  strcpy(write_clocktime_file_name, "");
  strcpy(replayFileList, "rpdb_filelist");
  while (fgets(tempstr, 256, inifile))
    {
      if (tempstr[0] != '#')
	{
	  if (strstr(tempstr, "replaydatapath="))
	    sscanf(tempstr, "replaydatapath=%s", replay_data_path);
	  if (strstr(tempstr, "replayfilelist="))
	    sscanf(tempstr, "replayfilelist=%s", replayFileList);
	  if (strstr(tempstr, "clockfilename="))
	    sscanf(tempstr, "clockfilename=%s", replay_clock_file_name);
	  if (strstr(tempstr, "writeClockTimeFile="))
	    sscanf(tempstr, "writeClockTimeFile=%s", write_clocktime_file_name);
	  if (strstr(tempstr, "useInternalClock"))
	    useInternalClock = true;
	  if (strstr(tempstr, "replayAllDBs"))
	    replayAllDbs = true;
	  if (strstr(tempstr, "internalTimeScale="))
	    sscanf(tempstr, "internalTimeScale=%f", &internalTimeScale);
	  if (strstr(tempstr, "startTime="))
	    {
	      if (sscanf(tempstr, "startTime=%d %d %d %d %d %d", &yr, &mon, &dom, &hr, &min, &sec) == 6)
		{
		  replay_start_time = DateTime2UnixTime(yr, mon, dom, hr, min, sec);
		}
	    }
	  if (strstr(tempstr, "endTime="))
	    {
	      if (sscanf(tempstr, "endTime=%d %d %d %d %d %d", &yr, &mon, &dom, &hr, &min, &sec) == 6)
		{
		  replay_end_time = DateTime2UnixTime(yr, mon, dom, hr, min, sec);
		}
	    }
	}
    }
  init(replay_data_path, replay_clock_file_name);
  if (useInternalClock)
    {
      if (replay_start_time == 0)
	{
	  fprintf(stderr, "replay::replay - ERROR useInternalClock defined, but startTime not defined\n");
	  useInternalClock = false;
	}
      else
	startInternalClockReplay(replay_start_time, replay_end_time,internalTimeScale);
    }
}

void replay::init(char *replayDataPath, char *replayClockFileName)
{
  char tempstr[256];

  strncpy(replay_data_path, replayDataPath, 256);
  if (replay_data_path[strlen(replay_data_path)-1] != '/')
    strcat(replay_data_path, "/");	// ensure path ends with /
  if (replayClockFileName)
    strncpy(tempstr, replayClockFileName, 256);
  else
    strncpy(tempstr, defaultreplayclockfile, 256);
  if (tempstr[0] == '/')  // clock path is absolute
    strncpy(replay_clock_file_name, tempstr, 256);
  else	// clock file name is relative to replay_data_path
    {
      strncpy(replay_clock_file_name, replay_data_path, 256);
      strncat(replay_clock_file_name, tempstr, 256);
    }
  enabled = false;
  ReplayAllowed = true;
  debug = true;
  last_replay_clock_time = this_replay_clock_time = 0;
  thisdb = rpdbs.begin();
  data_path_valid = DirExists(replay_data_path);
  strncpy(rpdb_filelist, replay_data_path, 256);
  strncat(rpdb_filelist, replayFileList, 256);
  strncpy(replay_enabled_file_name, replay_data_path, 256);
  strncat(replay_enabled_file_name, defaultreplayenabledfile, 256);
  if (!data_path_valid)
    {
      fprintf(stderr, "replay::init - ERROR replayDataPath (%s) doesn't seem to exist\n", 
	      replay_data_path);
    }
  else 
    {
      fprintf(stderr, "replay::init - using replayDataPath=%s clockfile=%s\n", 
	      replay_data_path, replay_clock_file_name);
    }
  db_count = 0;    
}

replay::~replay()
{
  close_rpdbs();
}

void replay::SetReplayAllowed(bool isallowed)
{
  if (isallowed == ReplayAllowed)
    return ;
  if (isallowed)
    {
      ReplayAllowed = isallowed;
      enabled = false;    // Replay was disabled, force enabled state to flase before checking
      check_replay();
    }
  else
    {
      ReplayAllowed = isallowed;
      if (enabled)
	{
	  ScanMng->SetDataMode(REALTIMEMODE);
	  close_rpdbs();
	  enabled = false;
	}
    }
}

void replay::startInternalClockReplay(time_t replaystarttime, time_t replayendtime, float timescalefactor)
{
  useInternalClock = true;
  if (replaystarttime)
    replay_start_time = replaystarttime;
  if (replayendtime)
    replay_end_time = replayendtime;
  replay_start_real_time = time(0);  // reference time
  if (timescalefactor)
    internalTimeScale = timescalefactor;
  this_replay_clock_time = replay_start_time;
  last_replay_clock_time = 0;
}

void replay::stopInternalClockReplay()
{
  useInternalClock = false;
  last_replay_clock_time = this_replay_clock_time = 0;
}

void replay::pauseReplay(bool pausestate)
{
  if (pausestate)
    {
      if (!replay_paused)  // don't do anything if already paused
	{
	  replay_paused_real_time = time(0);
	  replay_paused = true;
	}
    }
  else
    {
      if (replay_paused)  // don't do abything if not already paused
	{
	  replay_start_real_time +=   // add time paused to start time
	    time(0) - replay_paused_real_time;
	  replay_paused_real_time = 0;
	  replay_paused = false;
	}
    }
}

time_t  replay::getReplayTime()
{
  if (useInternalClock)
    {
      if (replay_paused)
	this_replay_clock_time = replay_start_time + 
	  time_t((float(replay_paused_real_time - replay_start_real_time)) *
		 internalTimeScale);
      else
	this_replay_clock_time = replay_start_time + 
	  time_t((float(time(0) - replay_start_real_time)) * 
		 internalTimeScale);
    }
  return this_replay_clock_time;
}

bool replay::check_replay_clock(char *clock_name)
{
  FILE *clockfile;
  int yr, mon, dom, hr, min, sec;


  // if using internal clock, update it from real time delta and exit
  if (useInternalClock)
    {
      this_replay_clock_time = getReplayTime();
      if (replay_end_time &&
	  (this_replay_clock_time > replay_end_time))
	{
	  stopInternalClockReplay();
	  return false;
	}
      else
	return true;
    }

  clockfile = fopen(replay_clock_file_name, "r");
  if (!clockfile)
    {
      fprintf(stderr, "replay::check_replay_clock - FAILED TO OPEN CLOCK FILE %s - RETRYING\n", replay_clock_file_name);
      perror("");
      sleep(1);
      clockfile = fopen(replay_clock_file_name, "r");
      if (!clockfile) 
	{
	  fprintf(stderr, "replay::check_replay_clock - RETRY FAILED TO OPEN CLOCK FILE %s\n", replay_clock_file_name);
	  perror("");
	  return false;
	}
    }
  if (fscanf(clockfile, "%d %d %d %d %d %d", &yr, &mon, &dom, &hr, &min, &sec) == 6)
    {
      this_replay_clock_time = DateTime2UnixTime(yr, mon, dom, hr, min, sec);
      /*
	if (debug)
	fprintf(stderr, "replay::check_replay_clock Newtime=%s, PrevTime=%s\n", 
	ctime(&this_replay_clock_time), ctime(&last_replay_clock_time));
      */
    }
  else
    {
      fprintf(stderr, "replay::check_replay_clock - ERROR READING CLOCK FILE %s\n", replay_clock_file_name);
      return false;	
    }
  fclose(clockfile);
  return true;
}

void replay::check_replay()
{

  if (!ReplayAllowed)
    return;

  bool newenabled;
  char tempstr[256];

  char tempstr1[256], tempstr2[128];
    
  if (useInternalClock)
    newenabled = (this_replay_clock_time != 0);
  else
    newenabled = FileExists(replay_enabled_file_name);

  if (newenabled != enabled)
    {
      if (newenabled)
	{
	  sprintf(tempstr, "replay::check_replay ENABLED by file %s,  Setting 3DRapic to REPLAYMODE\n", replay_enabled_file_name);
	  ScanMng->SetDataMode(REPLAYMODE);
	  load_rpdbs();
	  if (useInternalClock)
	    {
	      UnixTime2DateTimeString(replay_start_time, false, tempstr1);
	      UnixTime2DateTimeString(replay_end_time, false, tempstr2);
	      fprintf(stderr, "replay::load_rpdbs - Replay starting at %s Ending at %s\n",
		      tempstr1, tempstr2);
	    }
	}
      else
	{
	  sprintf(tempstr, "replay::check_replay DISABLED file no longer present = %s Setting 3DRapic to REALTIMEMODE\n", replay_enabled_file_name);
	  ScanMng->SetDataMode(REALTIMEMODE);
	  close_rpdbs();
#ifndef NO_XWIN_GUI
	  DBLoadRealTime();
#endif
	  last_replay_clock_time = this_replay_clock_time = 0;
	}
      RapicLog(tempstr, LOG_WARNING);
      enabled = newenabled;
    }
  if (enabled)
    {
      check_replay_clock();
      get_replay_scans();
    }
}

void replay::load_rpdbs(char *rpdb_Filelist)
{
  FILE *tempfile = 0;
  char fname[256];
  char tempstr[256];
  rp_isam *tempisam = 0;
  bool fname_ok = false;

  time_t first_tm = 0, last_tm = 0;

  if (db_count)
    close_rpdbs();
  if (!rpdb_Filelist)
    rpdb_Filelist = rpdb_filelist;	// if not specified, use default
  tempfile = fopen(rpdb_Filelist, "r");
  if (!tempfile)
    {
      fprintf(stderr, "replay::load_rpdbs FAILED - Unable to open list file %s\n", rpdb_Filelist);
    }
  while (tempfile && fscanf(tempfile, "%255s", fname) == 1)
    {
      if (!tempisam)
	tempisam = new rp_isam();
      if (tempisam->Open(replay_data_path, fname, true, true))	
	          // open as read only, but allow rdonlyrebuild
	{
	  if (!first_tm || (tempisam->first_tm < first_tm))
	    first_tm = tempisam->first_tm;
	  if (!last_tm || (tempisam->last_tm > last_tm))
	    last_tm = tempisam->last_tm;
	  rpdbs.push_back(tempisam);
	  fprintf(stderr, "replay::load_rpdbs dbname= %s%s DB#%d %s\n", 
		  replay_data_path, fname, 
		  db_count, 
		  tempisam->LabelFromTo());
	  tempisam = 0;
	  db_count++;
	}
      else
	{
	  fprintf(stderr, "replay::load_rpdbs Unable to open %s%s\n", 
		  replay_data_path, fname);
	}
    }
  tempfile = fopen(replay_ini_file_name, "r");
  if (!tempfile)
    {
      fprintf(stderr, "replay::load_rpdbs FAILED - Unable to open replay ini file %s\n", replay_ini_file_name);
    }
  while (tempfile && fgets(tempstr, 256, tempfile))
    {
      if (tempstr[0] != '#')
	{
	  if (strstr(tempstr, "rpdb="))
	    fname_ok = (sscanf(tempstr, "rpdb=%s", fname) == 1);
	  else 
	    fname_ok = false;
	  if (fname_ok)
	    {
	      if (!tempisam)
		tempisam = new rp_isam();
	      if (tempisam->Open(replay_data_path, fname, true, true))	
		// open as read only, but allow rdonlyrebuild
		{
		  if (!first_tm || (tempisam->first_tm < first_tm))
		    first_tm = tempisam->first_tm;
		  if (!last_tm || (tempisam->last_tm > last_tm))
		    last_tm = tempisam->last_tm;
		  rpdbs.push_back(tempisam);
		  fprintf(stderr, "replay::load_rpdbs dbname= %s%s DB#%d %s\n", 
			  replay_data_path, fname, 
			  db_count, 
			  tempisam->LabelFromTo());
		  tempisam = 0;
		  db_count++;
		}
	      else
		{
		  fprintf(stderr, "replay::load_rpdbs Unable to open %s%s\n", 
			  replay_data_path, fname);
		}
	    }
	}
    }
  if (tempisam)   // last open failed, clean up tempisam
    delete tempisam;
  if (replayAllDbs)
    {
      replay_start_time = first_tm;
      replay_end_time = last_tm;
    }
}

void replay::close_rpdbs()
{

  thisdb = rpdbs.begin();
  while (thisdb != rpdbs.end())
    {
      delete *thisdb;
      thisdb++;
    }
  rpdbs.erase(rpdbs.begin(), rpdbs.end());
  thisdb = rpdbs.begin();
  db_count = 0;
}

/*
 * traverse all of the dbs and load any scans
 * > last_replay_clock_time and <= this_replay_clock_time
 */
void replay::get_replay_scans()
{
  bool	useThisDB = false;
  bool done = false; 
  rdr_scan *new_scan = 0;
  char tempstr1[256], tempstr2[128];
  if (last_replay_clock_time == this_replay_clock_time)
    return;
  if (last_replay_clock_time == 0)	// first db read, limit time window
    last_replay_clock_time = this_replay_clock_time - maxfirstblocktime;
  thisdb = rpdbs.begin();
  if (debug)
    {
      UnixTime2DateTimeString(last_replay_clock_time, false, tempstr1);
      UnixTime2DateTimeString(this_replay_clock_time, false, tempstr2);
      fprintf(stderr, "replay::get_replay_scans - Getting scans btwn %s and %s\n", 
	      tempstr1, tempstr2);
    }
  while (thisdb < rpdbs.end())
    {
      useThisDB = !(this_replay_clock_time < (*thisdb)->first_tm ||
		    last_replay_clock_time > (*thisdb)->last_tm);
      if (!useThisDB && (*thisdb)->IsOpen())
	{
	  fprintf(stderr, "replay::get_replay_scans - Times not within database %s, closing until required\n", 
		  (*thisdb)->LabelFromTo());
	  (*thisdb)->Close();
	}
      if (debug && useThisDB)
	fprintf(stderr, "replay::get_replay_scans - Checking DB %s\n", 
		(*thisdb)->LabelFromTo());
      if (useThisDB)
	{
	  if (!(*thisdb)->IsOpen())
	    (*thisdb)->Open();
	  done = !(*thisdb)->SearchDtTm(last_replay_clock_time, true);
	  done |= (*thisdb)->CurrentRec.datetime <= last_replay_clock_time ||
	    (*thisdb)->CurrentRec.datetime > this_replay_clock_time;
	}
      else
	done = true;
      while (!done)	    // load all scans between last and this time
	{
	  if (!(*thisdb)->IsOpen())
	    {
	      fprintf(stderr, "replay::get_replay_scans - Times within Closed database %s, opening it\n", 
		      (*thisdb)->LabelFromTo());
	      (*thisdb)->Open();
	    }
	  if (!new_scan)
	    new_scan = new rdr_scan(this, "replay::get_replay_scans");
	  if ((*thisdb)->GetScanSet(new_scan))
	    {
	      fprintf(stderr, "replay::get_replay_scans - Loading scan %s from DB %s\n",
		      new_scan->ScanString(), (*thisdb)->LabelFromTo()); 
	      new_scan->data_source = REPLAY;
	      if (ScanMng)
		{
		  ScanMng->NewDataAvail(new_scan);
		  ScanMng->FinishedDataAvail(new_scan);
		}
	      if (new_scan->ShouldDelete(this, "replay::get_replay_scans"))
		delete new_scan;
	      new_scan = 0;
	    }
	  done = !(*thisdb)->NextRec(true);
	  done |= (*thisdb)->CurrentRec.datetime <= last_replay_clock_time ||
	    (*thisdb)->CurrentRec.datetime > this_replay_clock_time;
	}
      thisdb++;
    }
  if (strlen(write_clocktime_file_name))
    writeClockTimeFile(this_replay_clock_time);
  last_replay_clock_time = this_replay_clock_time;       
}

void replay::writeClockTimeFile(time_t newtime)
{
  if (!newtime)
    newtime = this_replay_clock_time;
  char tempname[270];
  char timestr[64];
  strcpy(tempname, write_clocktime_file_name);
  strcat(tempname, ".temp");
  FILE *timefile = fopen(tempname, "w");
  if (!timefile)
    {
      fprintf(stderr, "replay::writeClockTimeFile FAILED - Unable to open file %s\n",
	      write_clocktime_file_name);
      perror(NULL);
      return;
    }
  fprintf(timefile, "%s", ShortTimeString(newtime, timestr));
  if (replay_paused)
    fprintf(timefile, "paused");
  fclose(timefile);
  if (rename(tempname, write_clocktime_file_name) < 0) // move temp file name to final filename
    {
      fprintf(stderr, "replay::writeClockTimeFile - Error Renaming %s to %s\n", tempname, write_clocktime_file_name);
      perror(0);
      return;
    }
}
  
