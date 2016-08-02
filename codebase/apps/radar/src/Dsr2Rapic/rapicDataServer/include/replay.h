#ifndef __RP_REPLAY_H
#define __RP_REPLAY_H

/*
 * The replay class is given a data directory which should 
 * contain 3DRapic databases.
 * It is also given the name of a replay "clock file"
 * which specifies the current replay time.
 * The clock file contains an ASCII string as follows
 * yyyy mm dd hh MM ss (DoW Mon DoM hh:mm:ss TMZN yyyy)
 * e.g.
 * 2000 01 18 22 00 00 (Thu Jan 20 13:12:23 AEDT 2000)
 * where the date/time in brackets is the time the file was updated.
 */

#include "rpdb.h"
#ifdef STDCPPHEADERS
#include <list>
#include <vector>
#else
#include <list.h>
#include <vector.h>
#endif

class replay {
    vector<rp_isam*> rpdbs;
    vector<rp_isam*>::iterator thisdb;
    int	    db_count;
    bool    ReplayAllowed;  // if true, totally disable replay operation
    bool    enabled, 
	    data_path_valid, 
	    clock_file_valid, 
	    debug;
    char    replay_data_path[256], 
      replay_enabled_file_name[256], 
      replay_ini_file_name[256],
      write_clocktime_file_name[256],
      replay_clock_file_name[256];
    char  replayFileList[256];
    char    rpdb_filelist[256];
    time_t  last_replay_clock_time, 
	    this_replay_clock_time;

    bool    useInternalClock;        // if defined use internal replay clock

    bool    replayAllDbs;        
    // if true replay will run from earliest time in loaded rpdbs through to the latest time

    float   internalTimeScale;       // time scaling factor for internal clock
    time_t  replay_start_real_time,  // real time clock at replay start
      replay_start_time,             // replay start time
      replay_end_time;               
    bool    replay_paused;
    time_t  replay_paused_real_time;
    void    writeClockTimeFile(time_t newtime = 0);
public:
    replay(char *replayinifile = 0);
    ~replay();
    void init(char *replayDataPath, char *replayClockFileName = 0);
    bool    check_replay_clock(char *clock_name = 0);
    void    check_replay();
    void    load_rpdbs(char *rpdb_Filelist = 0);
    void    close_rpdbs();
    void    SetReplayAllowed(bool isallowed);

    void    startInternalClockReplay(time_t replaystarttime, time_t replayendtime = 0, float timescalefactor = 1.0);
    void    stopInternalClockReplay();
    void    pauseReplay(bool pausestate);
    bool    replayPaused() { return replay_paused; };
    
    time_t  getReplayTime();
    float   getTimeScale() { return internalTimeScale; };

/* 
    traverse the rpdbs and
    load scans > last_replay_clock_time
    and <= this_replay_clock_time    
*/
    void    get_replay_scans();	    
    void    setEnabled(bool en = true);
    bool    isEnabled() { return enabled; };
};

extern replay	*ReplayMngr;
extern char	defaultreplayinifile[];

#endif // __RP_REPLAY_H
