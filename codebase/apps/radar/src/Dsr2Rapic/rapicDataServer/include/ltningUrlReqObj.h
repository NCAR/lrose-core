/*

  ltningUrlReqObj.h
  
  Child class of urlReqObj - specialised to fetch index.txt
  and then fetch initial ltning urls from url server directory
  to suit sequence depth
  Once initial load complete reverts to realtime operation - 
  i.e. checking lgt_01.axf for updates
  Needs to read index.txt and assemble map of file times and filenames
  fileReaders.ini entry e.g.
  ltningURL=http://comms.ho.bom.gov.au/gpats reader=LightningAXF debug=1 event_exec_delay=2
  reqFName must refer to the ltning data directory

*/

#ifndef __LTNINGURLREQOBJ__H
#define __LTNINGURLREQOBJ__H

#include "fam_mon.h"
#include <map>

enum ltningUrlReqObj_modes
  {
    LURO_loadingIndex,         // loading lightning index.txt file from server
    LURO_loadingTimeFiles,     // loading lightning time files from serer
    LURO_loadingRealTimeFiles, // loading real time files lgt_nn.axf from the server
    LURO_realTime             // running real-time mode, checking for lgt_01.axf
  };

class ltningUrlReqObj : public urlReqObj
{
 public:
  std::string ltningDirFName;  // request file name
  virtual void setFName(char *newfname);
  std::map<time_t, std::string> timeFileIndex;
  std::map<time_t, std::string>::iterator loadUrl_iter_start, loadUrl_iter_end;
  ltningUrlReqObj_modes mode;
  int realTimeLoadCount;
  int realTimeFileNo;
  int loadTimePeriod;
  ltningUrlReqObj(char *initstr = 0, int id = 0) : 
    urlReqObj(initstr, id)
    {
      froType = FRO_LTNING;
      realTimeLoadCount = 12; // number of lgt_nn.axf files to attempt to load
      loadTimePeriod = 180;   // minutes of lightning data to load
      init(initstr);
      realtimeEventDelay = doEventDelay;
      setMode(LURO_loadingIndex);
      fro_id = id;
    };
  time_t realtimeEventDelay;
  void init(char *initstr);
  void setMode(ltningUrlReqObj_modes setmode = LURO_realTime);
  virtual bool checkLoadIndex(); // return true if url date changed
  virtual bool checkLoadRealTime(); // return true if url date changed
  virtual bool checkLoadTimeFiles(); // return true if url date changed
  virtual bool checkRealTime(time_t timenow = 0); // return true if url date changed
  virtual bool check(time_t timenow = 0); // return true if url date changed
  void addTimeFileStr(char *str);
  void readIndexTxt();
  // load from newest back to oldest
  bool setLoadingTimeFiles(time_t starttime, time_t endtime = 0);
  bool setNextTimeFile();
  bool setLoadingRealTimeFiles();
  bool setNextRealTimeFile();
  virtual void doEvent(); // do the event operations  
  virtual void doFileReader();
  virtual void doFileReaderLoadIndex();
  virtual void doFileReaderTimeFiles();
  virtual void doFileReaderRealTimeFiles();
  virtual void doFileReaderRealTime();
  virtual void dumpStatus(FILE *dumpfile, char *title = NULL);
};

#endif
