/*

  wdssData.h
  
  wdss Data class headers

*/

#ifndef __WDSSDATA_H
#define __WDSSDATA_H

#include <vector>
#include <list>
#include <map>
#include <string>
#include <iostream>
#include "spinlock.h"
#include "rdrscan.h"

#define LB_THREADED
// Every WDSS-II program requires these
#include <sources/code_Baseline.h>
#include <wdssii/code_StartupOptions.h>
#include <wdssii/code_Error.h>
#include <wdssii/code_EventHandler.h>

// All these are required for input
#include <wdssii/code_IndexRecordInformation.h>
#include <wdssii/code_IndexFactory.h>

// These are required for output
#include <rss_notifier/code_LinearBuffer.h>
#include <rss_notifier/code_XMLLBIndex.h>

// This will let us avoid bugs when we mistype Reflectivity, etc.
#include <wdssii/code_TypeName.h>

// Reflectivity, Velocity are RadialSets ... if you are reading
// gridded data, you might be using code_LatLonGrid.h here.
#include <data_object/code_DataTable.h>

// Utility classes for dealing with Time
#include <util/code_TimeInterval.h>
#include <util/code_CalendarDate.h>

// Needed for our typeid call
#include <typeinfo>

using namespace std;
using namespace code;

float null_999(float inval);
char *time_t_2str(time_t time, char *str_buff);
char *time_t_2axfstr(time_t time, char *str_buff);
char *time_t_2axfdatestr(time_t time, char *str_buff);

void writeCellAXF2Description(ofstream& axf_file, time_t updatetime, string& radarname);
void writeMesoAXF1Description(ofstream& axf_file, time_t updatetime, string& radarname);
void writeTvsAXF1Description(ofstream& axf_file, time_t desctime, string& radarname);

enum wdssCellMaxValType
  {
    WCMV_VIL,
    WCMV_DBZ,
    WCMV_HAIL,
    WCMV_SVHAIL
  };

enum wdssCellType
  {
    WCT_BASE,
    WCT_STORM,
    WCT_MESO,
    WCT_TVS
  };

class wdssBaseData
{
 public:
  float    rdrLat;            // location of radar
  float    rdrLong;           // location of radar
  float    h;                 // (deg, deg, km) radar origin
  time_t   tm;                // (unix time_t) time of cell
  float    Latitude;          // (deg) lat of cell
  float    Longitude;         // (deg) long of cell
  float    XLoc, YLoc;        // (km) position relative to radar origin ??
  float    Dir;
  float    Speed;
 private:
  int      CellID;            // ID # of cell
  int      CellIDTrackNum;    // track number for this CellID
  int      maxCellID;         // Cell ID recycle limit
 public:
  wdssCellType cellType;
  int      cellID();
  int      rawCellID() { return CellID; };
  int      rawTrackID() { return CellIDTrackNum; };
  int      rawTrackIDMult() { return maxCellID; };
  void     setRawCellID(int cellid) { CellID = cellid; };
  void     setRawTrackID(int tracknum) { CellIDTrackNum = tracknum; };
  void     setMaxCellID(int maxcellid) { maxCellID = maxcellid; };
  virtual float    severity() { return 1.0; };  // cell feature severity rating - normal = 1.0
  int      radarID;     // bom radar site id
  string   radarName;     // bom radar name
  bool     valid;
  bool     deleted;
  virtual void     init(int radarid);
  wdssBaseData(int radarid) { init(radarid); };
  virtual ~wdssBaseData() {};
};

class wdssMesoData;
class wdssTvsData;

class wdssStormCellData : public wdssBaseData
{
 public:
  int      AgeHours, AgeMin, AgeSec; // age of cell
  int      AgeVS;             // Age in # of vol scans ??
  int      AlgRank;           // netssap Algorithm Ranking
  float    Azimuth;           // (deg) Az of cell relative to radar origin
  float    CellBase;          // (km) base of cell
 public:
  float    CellMass;          // (kg) mass of cell
  float    CellTop;           // (km) top of cell
  float    CellVolume;        // (m^3???? More like km^3 from sample values pjp) volume of cell
  float    ConvDepth;         // (km) Convective depth of cell
  float    CoreAspectRatio;   // aspect ratio of cell
  int      DDPDAStrength;     // ??
  int      DDPDAType;         // int
  float    FcstError;         // (km) 
  float    HailSizeEstimate;  // (mm)
  float    Height;            // (km) height of cell??
  float    HeightOfCenterOfMass; // (km)
  float    HeightOfMaxdBZ;    // (km)
  float    LowLevelXLoc;      // (km)
  float    LowLevelYLoc;      // (km)
  float    MaxConvergence;    // (m/s)
  float    MaxdBZ;            // (dBZ)
  string   MesoDetectionType; // 
  int      ProbOfHail;        // (%)
  int      ProbOfSevereHail;  // (%)
  float    Range;             // (km)
  float    ReflectivityRatio; // 
  int      RowName;           // 
  int      RowNameUsage;      // 
  float    SevereHailIndex;   //
  float    StormTopDiv;       // (m/s)
  float    UMotion;           // (m/s)
  float    VIL;               // (kg/m^2)
  float    VMotion;           // (m/s);

  wdssMesoData *assocMeso; 
  wdssTvsData *assocTvs; 

  virtual void     init(int radarid = 0);
  wdssStormCellData(int radarid) : wdssBaseData(radarid) { init(radarid); };
  virtual ~wdssStormCellData();
  void     printSummary() {};
  char*    makeDataTableString(char *stringbuff512);
/*
 axfStr1, format compatible with Sydney Olympics AIFS WDSS output
 Format is 
 lat, lon, aifstime[30], track_num, cellid, mxz, hgtmxz, base, top, mass, ltngflshrate, posltngrate, negltngrate, pctposltng, strmrelhlcty, vil, cellvolume, coraspctratio, 3dltngrate, cntrmasshgt, reflratio, mxconv, dir, speed
*/
  char*     axfStr1(char *axfstr);
  char*     axfStr1Heading(char *axfstrhd);
/*
 axfStr2, format similar to Sydney Olympics AIFS WDSS output, no ltning, add hail
 Format is 
 lat, lon, aifstime[30], track_num, mxz, hgtmxz, base, top, topdivrg, mass, vil, cellvolume, coraspctratio, cntrmasshgt, reflratio, mxconv, dir, speed, fcsterr, probhail, probsvhail, sevhailidx, hailsz, algrank
*/

  char*     axfStr2(char *axfstr);
  char*     axfStr2Heading(char *axfstrhd);
/*
 axfStrDebug, format similar to Sydney Olympics AIFS WDSS output, no ltning, add hail, agevs etc
 Format is 
 lat, lon, aifstime[30], track_num, cellid, mxz, hgtmxz, base, top, mass, vil, cellvolume, coraspctratio, cntrmasshgt, reflratio, mxconv, dir, speed, probhail, probsvhail, hailsz, algrank, agevs, agehrminsec
*/
  char*     axfStrDebug(char *axfstr);
  char*     axfStrDebugHeading(char *axfstrhd);
};

/* stl map of pointers to wdss cells for given time, indexed by cellid */
// not thread safe, assumes use from main rendering thread only
class wdssCellMap 
{
 public:
  time_t cell_time;
  float lastlat, lastlong, lastdist;
  wdssStormCellData *lastCell;
  map<int, wdssStormCellData*> cellMap;
  wdssCellMap() { clear(); };
  void addCell(wdssStormCellData *newcell);
  void clear();
  void newTime(time_t newtime) { clear(); cell_time = newtime; };
  wdssStormCellData* getNearestCell(float lat, float lng,
				    float max_dist_to_nearest_cell = -1);
};

class wdssMesoData : public wdssBaseData
{
 public:
  float    _3DAz; 	// Unit=Degrees
  float    _3DRan; 	// Unit=Kilometers
  float    _6kmXLoc; 	// Unit=Kilometers
  float    _6kmYLoc; 	// Unit=Kilometers
  float    Age; 	// Unit=Minutes
  int      AlgRank;           // netssap Algorithm Ranking
  float    CoreBase; 	// Unit=Kilometers
  float    CoreDepth; 	// Unit=Kilometers
  float    CoreTop; 	// Unit=Kilometers
  float    HtMaxLevDia; // Unit=Kilometers
  float    HtMaxLevGtGdV; // Unit=Kilometers
  float    HtMaxLevRotV;  // Unit=Kilometers
  float    HtMaxLevShear; // Unit=Kilometers
 public:
  float    LowLevAz; 	// Unit=Degrees
  float    LowLevDia; 	// Unit=Kilometers
  float    LowLevGtGdV; // Unit=MetersPerSecond
  float    LowLevRan; 	// Unit=Kilometers
  float    LowLevRotV; 	// Unit=MetersPerSecond
  float    LowLevShear; // Unit=MetersPerSecondPerKilometer
  float    LowLevXLoc; 	// Unit=Kilometers
  float    LowLevYLoc; 	// Unit=Kilometers
  float    LowLevelConv;// Unit=MetersPerSecond
  float    LowToppedMeso; // Unit=dimensionless
  string   MDAType; 	// Unit=dimensionless
  int      MSIRank; 	// Unit=dimensionless
  float    MaxLRotV; 	// Unit=MetersPerSecond
  float    MaxLevDia; 	// Unit=Kilometers
  float    MaxLevGtGdV; // Unit=MetersPerSecond
  float    MaxLevShear; // Unit=MetersPerSecondPerKilometer
  float    MesoBase; 	// Unit=Kilometers
  float    MesoDepth; 	// Unit=Kilometers
  string   MesoDetectionType; // Unit=dimensionless
  float    MesoStrengthIndex; // Unit=dimensionless
  float    MesoTop; 	// Unit=Kilometers
  float    MidLevelConv;// Unit=MetersPerSecond
  float    NSSLBase; 	// Unit=Kilometers
  float    NSSLDepth; 	// Unit=Kilometers
  float    NSSLMeso; 	// Unit=dimensionless
  float    NSSLTop; 	// Unit=Kilometers
  float    ProbOfSvrWx; // Unit=%
  float    ProbOfTornado; // Unit=%
  int      RowName; 	// Unit=dimensionless
  int      RowNameUsage;// Unit=dimensionless
  int      StormCellID; // Unit=dimensionless
  float    StormDepth; 	// Unit=Kilometers
  float    StormRelativeDepth; // Unit=%
  float    StrengthRank;// Unit=dimensionless
  int      TVSID; 	// Unit=dimensionless
  int      TimeAssocCode; // Unit=dimensionless
  float    TornadicMeso;// Unit=dimensionless
  float    UMotion; 	// Unit=MetersPerSecond
  float    VMotion; 	// Unit=MetersPerSecond
  float    VertIntegratedGtGdV; // Unit=dimensionless
  float    VertIntegratedRotVel;// Unit=dimensionless
  float    VertIntegratedShear; // Unit=dimensionless
  float    WeakLowtopBase; 	// Unit=Kilometers
  float    WeakLowtopDepth; 	// Unit=Kilometers
  float    WeakLowtopTop; 	// Unit=Kilometers

  virtual float    severity();  // cell feature severity rating - 
  // Circ = 1.0, LowAlt=2.0 Cplt=3.0 meso = 4.0, tvs = 5.0
  bool isCirc();
  bool isLowAlt();
  bool isCplt();
  bool isMeso();
  bool isTVS();
  void     init(int radarid = 0);
  wdssMesoData(int radarid) : wdssBaseData(radarid) { init(radarid); };
  ~wdssMesoData();
  void     printSummary() {};
/*
 axfStr1, format compatible with Sydney Olympics AIFS WDSS output
 Format is 
 lat, lon, aifstime[30], track_num, cellid, mxz, hgtmxz, base, top, mass, ltngflshrate, posltngrate, negltngrate, pctposltng, strmrelhlcty, vil, cellvolume, coraspctratio, 3dltngrate, cntrmasshgt, reflratio, mxconv, dir, speed
*/
  char*     axfStr1(char *axfstr);
  char*     axfStr1Heading(char *axfstrhd);
};

/* stl map of pointers to wdss cells for given time, indexed by cellid */
// not thread safe, assumes use from main rendering thread only
class wdssMesoMap 
{
 public:
  time_t cell_time;
  float lastlat, lastlong, lastdist;
  wdssMesoData *lastCell;
  map<int, wdssMesoData*> cellMap;
  wdssMesoMap() { clear(); };
  void addCell(wdssMesoData *newcell);
  void clear();
  void newTime(time_t newtime) { clear(); cell_time = newtime; };
  wdssMesoData* getNearestCell(float lat, float lng, float max_dist_to_nearest_cell = -1);
};

class wdssTvsData : public wdssBaseData
{
 public:
  int      AlgRank;           // netssap Algorithm Ranking
  int      AssocMesoID;
  int      AssocStormID;
  float    Azimuth;           // (deg)
  string   CircType;          // ??
  float    DistToDetection;   // (km)
  float    HeightMaxLevShear; // (km)
  float    HeightMaxLevdV;    // (km)
  float    LowLevShear;       // (m/s/km)
  float    LowLevdV;          // (m/s)
  float    MaxLevShear;       // (m/s/km)
  float    MaxLevdV;          // (m/s)
  int      OldEventID;        //
  float    ProbOfTornado;     // (%)
  float    Range;             // (km)
  int      RowName;           // 
  int      RowNameUsage;      // 
  float    TVSBase;           // (km)
  float    TVSDepth;          // (km)
  float    TVSTop;            // (km)
  float    TornadoStrengthIndex; 

  void     init(int radarid = 0);
  wdssTvsData(int radarid) : wdssBaseData(radarid) { init(radarid); };
  ~wdssTvsData();
  void     printSummary() {};
/*
 axfStr1, format compatible with Sydney Olympics AIFS WDSS output
 Format is 
 lat, lon, aifstime[30], track_num, cellid, mxz, hgtmxz, base, top, mass, ltngflshrate, posltngrate, negltngrate, pctposltng, strmrelhlcty, vil, cellvolume, coraspctratio, 3dltngrate, cntrmasshgt, reflratio, mxconv, dir, speed
*/
  char*     axfStr1(char *axfstr);
  char*     axfStr1Heading(char *axfstrhd);
};

/* stl map of pointers to wdss cells for given time, indexed by cellid */
// not thread safe, assumes use from main rendering thread only
class wdssTvsMap 
{
 public:
  time_t cell_time;
  float lastlat, lastlong, lastdist;
  wdssTvsData *lastCell;
  map<int, wdssTvsData*> cellMap;
  wdssTvsMap() { clear(); };
  void addCell(wdssTvsData *newcell);
  void clear();
  void newTime(time_t newtime) { clear(); cell_time = newtime; };
  wdssTvsData* getNearestCell(float lat, float lng, 
			      float max_dist_to_nearest_cell = -1);
};

class wdssCellTrack
{
 public:
  map<time_t, wdssStormCellData*> stormCells;
  int      stormCellID;
  int      TrackNum;    // additional cell track number
  int      maxCellID;   // limit for CellId recycling
  void     closeTrack(bool state) { trackClosed = state; };
  bool     trackClosed;
  bool     deleted;
  wdssStormCellData *maxValCell;
  float    maxVal;   // max value of specified type in track
  wdssCellMaxValType maxType;   // max value of specified type in track
  time_t   firstTime, lastTime;
  wdssStormCellData *lastRenderedCell;

  bool     addWdssStormCell(wdssStormCellData *newCell);
  int      cellCount();
  wdssStormCellData* getStormCellAtTime(time_t celltime);
  wdssStormCellData* getStormCellNearestTime(time_t neartime, time_t nearWindow = -1); // find cell closest to time and within window
  wdssStormCellData* getStormCellLTETime(time_t ltetime, time_t nearWindow = -1); // find cell LessThanOrEqual to time and within window
  wdssStormCellData* getStormCellGTETime(time_t gtetime, time_t nearWindow = -1); // find cell GreaterThanOrEqual to time and within window

  spinlock *lock;
  bool     getLock();
  void     relLock();
  int      radarID;     // bom radar site id
  void     clear();     // delete all wdssStormCells and clear map
  void     init(int radarid, int cellID, int trackNum, int maxcellid);
  wdssCellTrack(int radarid, int cellID, int trackNum, int maxcellid) 
    { lock = NULL; init(radarid, cellID, trackNum, maxcellid); };
  ~wdssCellTrack();
  void     printSummary();
  void     writeAxfFile(char *axf_fname, bool trknumonly = false);
  void     writeAxfFile(ofstream& axf_file, bool trknumonly = false);
};

// Due to the WDSS reuse of CellIDs a class is required to manage separate tracks with the same CellID
// with an additional cell track number
class wdssCellIDTracks
{
 public:
  vector<wdssCellTrack*> cellTracks;
  int      stormCellID;
  int      openTrackNum;
  int      maxCellID;
  int      validTrackCount;
  time_t   firstTime, lastTime;
  time_t   updateTime;
  time_t   minTrackReuseTime;
  int      validTracksCount();
  int      totalTracksCount();
  int      getFreeTrackNumber(wdssStormCellData *newCell);  // get next free cell track number that hasn't been used for  a while
  bool     addWdssStormCell(wdssStormCellData *newCell);
  wdssStormCellData* getStormCellAtTime(time_t celltime);
  wdssStormCellData* getStormCellNearestTime(time_t neartime, time_t nearWindow = -1); // find cell closest to time and within window
  wdssStormCellData* getStormCellLTETime(time_t ltetime, time_t nearWindow = -1); // find cell LessThanOrEqual to time and within window
  wdssStormCellData* getStormCellGTETime(time_t gtetime, time_t nearWindow = -1); // find cell GreaterThanOrEqual to time and within window

  wdssCellTrack* getTrackAtTime(time_t celltime);
  wdssCellTrack* getTrackNearestTime(time_t neartime, time_t nearWindow = -1); // find cell track closest to time and within window
  wdssCellTrack* getTrackLTETime(time_t ltetime, time_t nearWindow = -1); // find cell track LessThanOrEqual to time and within window
  wdssCellTrack* getTrackGTETime(time_t gtetime, time_t nearWindow = -1); // find cell track GreaterThanOrEqual to time and within window

  void     deleteTracksBeforeTime(time_t deleteBeforeTime); // delete any tracks whose last time is before this time
  void     deleteTracksAfterTime(time_t deleteBeforeTime); // delete any tracks whose last time is before this time

  spinlock *lock;
  bool     getLock();
  void     relLock();
  int      radarID;     // bom radar site id
  void     clear();     // delete all wdssStormCells and clear map
  void     init(int radarid, int cellid, int maxcellid);
  void     updateTrackClosedFlags(time_t updateTime); // use the last cell in track time to close any tracks not updated since this time
  void     updateFirstLastTime();
  void     setUpdateTime(time_t updatetime) { updateTime = updatetime; };
  wdssCellIDTracks(int radarid, int cellid, int maxcellid) 
    { init(radarid, cellid, maxcellid); };
  ~wdssCellIDTracks();
  void     printSummary();
  void     writeAxfFile(char *axf_fname, bool trknumonly = false);
  void     writeAxfFile(ofstream& axf_file, bool trknumonly = false);
  void     writeAxfFileDescription(ofstream& axf_file, string& radarname);
};

class wdssMesoTrack
{
 public:
  map<time_t, wdssMesoData*> mesoCells;
  time_t   firstTime, lastTime;
  wdssMesoData *lastRenderedCell;
  void     closeTrack(bool state) { trackClosed = state; };
  bool     trackClosed;
  bool     deleted;
  int      mesoCellID;
  int      TrackNum;    // additional cell track number
  int      maxCellID;   // limit for CellId recycling
  bool     addWdssMesoCell(wdssMesoData *newMeso);
  int      cellCount();
  wdssMesoData* getMesoCellAtTime(time_t celltime);
  wdssMesoData* getMesoCellNearestTime(time_t neartime, time_t nearWindow = -1); // find cell closest to time and within window
  wdssMesoData* getMesoCellLTETime(time_t ltetime, time_t nearWindow = -1); // find cell LessThanOrEqual to time and within window
  wdssMesoData* getMesoCellGTETime(time_t gtetime, time_t nearWindow = -1); // find cell GreaterThanOrEqual to time and within window

  spinlock *lock;
  bool     getLock();
  void     relLock();
  int      radarID;     // bom radar site id
  void     clear();     // delete all wdssStormCells and clear map
  void     init(int radarid, int cellid, int trackNum, int maxcellid);
  wdssMesoTrack(int radarid, int cellid, int trackNum, int maxcellid) 
    { init(radarid, cellid, trackNum, maxcellid); };
  ~wdssMesoTrack();
  void     printSummary();
  void     writeAxfFile(char *axf_fname, bool trknumonly = false);
  void     writeAxfFile(ofstream& axf_file, bool trknumonly = false);
  void     writeAxfFileDescription(ofstream& axf_file, string& radarname);
};

// Due to the WDSS reuse of MesoIDs a class is required to manage separate tracks with the same MesoID
// with an additional cell track number
class wdssMesoIDTracks
{
 public:
  vector<wdssMesoTrack*> mesoTracks;
  int      MesoID;
  int      openTrackNum;
  int      maxCellID;
  int      validTrackCount;
  time_t   firstTime, lastTime;
  time_t   updateTime;
  time_t   minTrackReuseTime;
  int      validTracksCount();
  int      totalTracksCount();
  int      getFreeTrackNumber(wdssMesoData *newCell);  // get next free cell track number that hasn't been used for  a while
  bool     addWdssMesoCell(wdssMesoData *newCell);
  wdssMesoData* getMesoCellAtTime(time_t celltime);
  wdssMesoData* getMesoCellNearestTime(time_t neartime, time_t nearWindow = -1); // find cell closest to time and within window
  wdssMesoData* getMesoCellLTETime(time_t ltetime, time_t nearWindow = -1); // find cell LessThanOrEqual to time and within window
  wdssMesoData* getMesoCellGTETime(time_t gtetime, time_t nearWindow = -1); // find cell GreaterThanOrEqual to time and within window

  wdssMesoTrack* getTrackAtTime(time_t celltime);
  wdssMesoTrack* getTrackNearestTime(time_t neartime, time_t nearWindow = -1); // find cell track closest to time and within window
  wdssMesoTrack* getTrackLTETime(time_t ltetime, time_t nearWindow = -1); // find cell track LessThanOrEqual to time and within window
  wdssMesoTrack* getTrackGTETime(time_t gtetime, time_t nearWindow = -1); // find cell track GreaterThanOrEqual to time and within window

  void     deleteTracksBeforeTime(time_t deleteBeforeTime); // delete any tracks whose last time is before this time
  void     deleteTracksAfterTime(time_t deleteBeforeTime); // delete any tracks whose last time is before this time

  spinlock *lock;
  bool     getLock();
  void     relLock();
  int      radarID;     // bom radar site id
  void     clear();     // delete all wdssStormCells and clear map
  void     init(int radarid, int cellID, int maxcellid);
  void     updateTrackClosedFlags(time_t updateTime); // use the last cell in track time to close any tracks not updated since this time
  void     updateFirstLastTime();
  void     setUpdateTime(time_t updatetime) { updateTime = updatetime; };
  wdssMesoIDTracks(int radarid, int cellID, int maxcellid) 
    { init(radarid, cellID, maxcellid); };
  ~wdssMesoIDTracks();
  void     printSummary();
  void     writeAxfFile(char *axf_fname, bool trknumonly = false);
  void     writeAxfFile(ofstream& axf_file, bool trknumonly = false);
  void     writeAxfFileDescription(ofstream& axf_file, string& radarname);
};

class wdssTvsTrack
{
 public:
  map<time_t, wdssTvsData*> tvsCells;
  time_t   firstTime, lastTime;
  wdssTvsData *lastRenderedCell;
  void     closeTrack(bool state) { trackClosed = state; };
  bool     trackClosed;
  bool     deleted;
  int      tvsCellID;
  int      TrackNum;    // additional cell track number
  int      maxCellID;   // limit for CellId recycling
  bool     addWdssTvsCell(wdssTvsData *newTvs);
  int      cellCount();
  wdssTvsData* getTvsCellAtTime(time_t celltime);
  wdssTvsData* getTvsCellNearestTime(time_t neartime, time_t nearWindow = -1); // find cell closest to time and within window
  wdssTvsData* getTvsCellLTETime(time_t ltetime, time_t nearWindow = -1); // find cell LessThanOrEqual to time and within window
  wdssTvsData* getTvsCellGTETime(time_t gtetime, time_t nearWindow = -1); // find cell GreaterThanOrEqual to time and within window

  spinlock *lock;
  bool     getLock();
  void     relLock();
  int      radarID;     // bom radar site id
  void     clear();     // delete all wdssStormCells and clear map
  void     init(int radarid, int cellID, int trackNum, int maxcellid);
  wdssTvsTrack(int radarid, int cellID, int trackNum, int maxcellid) 
    { init(radarid, cellID, trackNum, maxcellid); };
  ~wdssTvsTrack();
  void     printSummary();
  void     writeAxfFile(char *axf_fname, bool trknumonly = false);
  void     writeAxfFile(ofstream& axf_file, bool trknumonly = false);
  void     writeAxfFileDescription(ofstream& axf_file, string& radarname);
};

// Due to the WDSS reuse of MesoIDs a class is required to manage separate tracks with the same MesoID
// with an additional cell track number
class wdssTvsIDTracks
{
 public:
  vector<wdssTvsTrack*> tvsTracks;
  int      TvsID;
  int      openTrackNum;
  int      maxCellID;
  int      validTrackCount;
  time_t   firstTime, lastTime;
  time_t   updateTime;
  time_t   minTrackReuseTime;
  int      validTracksCount();
  int      totalTracksCount();
  int      getFreeTrackNumber(wdssTvsData *newCell);  // get next free cell track number that hasn't been used for  a while
  bool     addWdssTvsCell(wdssTvsData *newCell);
  wdssTvsData* getTvsCellAtTime(time_t celltime);
  wdssTvsData* getTvsCellNearestTime(time_t neartime, time_t nearWindow = -1); // find cell closest to time and within window
  wdssTvsData* getTvsCellLTETime(time_t ltetime, time_t nearWindow = -1); // find cell LessThanOrEqual to time and within window
  wdssTvsData* getTvsCellGTETime(time_t gtetime, time_t nearWindow = -1); // find cell GreaterThanOrEqual to time and within window

  wdssTvsTrack* getTrackAtTime(time_t celltime);
  wdssTvsTrack* getTrackNearestTime(time_t neartime, time_t nearWindow = -1); // find cell track closest to time and within window
  wdssTvsTrack* getTrackLTETime(time_t ltetime, time_t nearWindow = -1); // find cell track LessThanOrEqual to time and within window
  wdssTvsTrack* getTrackGTETime(time_t gtetime, time_t nearWindow = -1); // find cell track GreaterThanOrEqual to time and within window

  void     deleteTracksBeforeTime(time_t deleteBeforeTime); // delete any tracks whose last time is before this time
  void     deleteTracksAfterTime(time_t deleteBeforeTime); // delete any tracks whose last time is before this time

  spinlock *lock;
  bool     getLock();
  void     relLock();
  int      radarID;     // bom radar site id
  void     clear();     // delete all wdssStormCells and clear map
  void     init(int radarid, int cellID, int maxcellid);
  void     updateTrackClosedFlags(time_t updateTime); // use the last cell in track time to close any tracks not updated since this time
  void     updateFirstLastTime();
  void     setUpdateTime(time_t updatetime) { updateTime = updatetime; };
  wdssTvsIDTracks(int radarid, int cellID, int maxcellid) 
    { init(radarid, cellID, maxcellid); };
  ~wdssTvsIDTracks();
  void     printSummary();
  void     writeAxfFile(char *axf_fname, bool trknumonly = false);
  void     writeAxfFile(ofstream& axf_file, bool trknumonly = false);
  void     writeAxfFileDescription(ofstream& axf_file, string& radarname);
};

class wdssTracksRadar    // container for all tracks for given radar
{
 public:
  map<int,wdssCellIDTracks*> wdssCellTracks;  // use StormCellID enumerator as array index
  map<int,wdssMesoIDTracks*> wdssMesoTracks;  // use StormCellID enumerator as array index
  map<int,wdssTvsIDTracks*> wdssTvsTracks;    // use StormCellID enumerator as array index

  wdssCellMap      latestStormCells;
  wdssMesoMap      latestMesoCells;
  wdssTvsMap       latestTvsCells;
  
  bool     addWdssStormCell(wdssStormCellData *newCell);
  bool     addWdssMeso(wdssMesoData *newMeso);
  bool     addWdssTvs(wdssTvsData *newTvs);
  wdssCellIDTracks *getCellIDTracks(int cellID);
  wdssMesoIDTracks *getMesoIDTracks(int cellID);
  wdssTvsIDTracks *getTvsIDTracks(int cellID);
  wdssCellTrack *getCellTrack(int cellID, int trackid);
  wdssMesoTrack *getMesoTrack(int cellID, int trackid);
  wdssTvsTrack *getTvsTrack(int cellID, int trackid);
  wdssCellTrack *getCellTrack(wdssStormCellData *cellintrack);
  wdssMesoTrack *getMesoTrack(wdssMesoData *mesointrack);
  wdssTvsTrack *getTvsTrack(wdssTvsData *tvsintrack);
  void     updateTrackClosedFlags(time_t updateTime); // use the last cell in track time to close any tracks not updated since this time
  void     updateCellTrackClosedFlags(time_t updateTime); // use the last cell in track time to close any tracks not updated since this time
  void     updateMesoTrackClosedFlags(time_t updateTime); // use the last cell in track time to close any tracks not updated since this time
  void     updateTvsTrackClosedFlags(time_t updateTime); // use the last cell in track time to close any tracks not updated since this time
  void     deleteTracksBeforeTime(time_t deleteBeforeTime); // delete any tracks whose last time is before this time
  void     deleteCellTracksBeforeTime(time_t deleteBeforeTime); // delete any tracks whose last time is before this time
  void     deleteMesoTracksBeforeTime(time_t deleteBeforeTime); // delete any tracks whose last time is before this time
  void     deleteTvsTracksBeforeTime(time_t deleteBeforeTime); // delete any tracks whose last time is before this time

  /*
    There should be a StormCell table, Meso table and Tvs table at each time
    As the Meso data and Tvs data are stored as objects under the Storm Cell object
    it is easiest if the storm cell table is added to the tracks first
    As each table event occurs, process but keep the resulting vector of meso and tvs objects
    if either or both events occur before the Storm Cell table is read.
  */
  void     processCellTable(const DataTable &table);
  time_t   lastCellTable_tm;
  void     processMesoTable(const DataTable &table);
  time_t   lastMesoTable_tm;
  void     processTvsTable(const DataTable &table);
  time_t   lastTvsTable_tm;
  

  spinlock *lock;
  bool     getLock();
  void     relLock();
  int      radarID;     // bom radar site id
  int      maxCellID;
  string   radarName;
  bool     debug;
  string   debugClassName;
  virtual void  setDebug(bool state);
  void     setRadarName(char *radarname);
  void     clear();     // delete all wdssTrack and clear map
  void     clearCellTracks();     // delete all wdssTrack and clear map
  void     clearMesoTracks();     // delete all wdssTrack and clear map
  void     clearTvsTracks();     // delete all wdssTrack and clear map
  int      validCellTracks();
  int      validMesoTracks();
  int      validTvsTracks();
  void     init(int radarid, string &radarname);
  wdssTracksRadar(int radarid, string  &radarname) { init(radarid, radarname); };
  virtual ~wdssTracksRadar();
  void     printSummary();
  void     writeAxfFile(const char *axfPath, const char *axfPrefix = NULL);
  void     writeCellAxfFile(const char *axfPath, const char *axfPrefix = NULL);
  void     writeCellAxfFile(ofstream& axf_file);
  void     writeMesoAxfFile(const char *axfPath, const char *axfPrefix = NULL);
  void     writeMesoAxfFile(ofstream& axf_file);
  void     writeTvsAxfFile(const char *axfPath, const char *axfPrefix = NULL);
  void     writeTvsAxfFile(ofstream& axf_file);
};

class wdssTracksRadarEventHandler;

class wdssNetssapListener : public ActionListener
{
  //  string   lbName;  
  //  LinearBuffer lb;  // LinearBuffer in example was for outpur LB
  wdssTracksRadarEventHandler *netssapEventHandler;
  virtual void actionPerformed(const ActionEvent* e);
 public:
  //  wdssNetssapListener(string &lbname, wdssTracksRadarEventHandler *_handler);
  wdssNetssapListener(wdssTracksRadarEventHandler *_handler);
  virtual ~wdssNetssapListener();
  //  void openLb(string &lbname);
  //  void closeLb();
  void setListenerHandler(wdssTracksRadarEventHandler *_handler);
  bool     debug;
  time_t   lastActionTime;
  int      actionCount;
  int      actionsIgnored;
};

#include "rpEventTimer.h"
extern rpEventTimer globalIndexFactoryEventTimer;
extern int maxGlobalIndexFactoryCount;
extern bool useGlobalIndexFactoryLock;
extern bool useGlobalIndexFactoryCount;

class wdssTracksRadarEventHandler : public wdssTracksRadar  // container for all tracks for given radar
{
  SmartPtr<Index> eventidx;
  SmartPtr<Index> pollidx;
  SmartPtr<ActionListener> listener;

 public:
  bool     realTime;
  bool     usePolling;
  bool     checkMeso, checkTVS;
  time_t   pollInterval;
  time_t   nextPollTime;
  code::Time     lastPolledRecTime;
  bool     useResolvedUrl;
  bool     resolveUrl();
  string   Url();
  string   rawUrl;
  string   resolvedUrl;
  string   UrlPrefix;
  string   UrlHostName;
  string   UrlResolvedHostName;
  string   UrlPath;
  string   axfFilePath;  // if this is defined, write axf files there on every event
  float    historyDepth;
  time_t   latest_CellEventTime, latest_MesoEventTime, latest_TvsEventTime; // time event occured
  time_t   lastConnectTime, lastConnectAttemptTime;
  Index::Record latestCellRec, latestMesoRec, latestTvsRec;
  rpEventTimer pollEventTimer;
  void     processNetssapAction(const Index::Record& rec);
  time_t   latestCellTime() { return lastCellTable_tm; };
  time_t   latestMesoTime() { return lastMesoTable_tm; };
  time_t   latestTvsTime() { return lastTvsTable_tm; };
  time_t   latestCellEventTime() { return latest_CellEventTime; };
  time_t   latestMesoEventTime() { return latest_MesoEventTime; };
  time_t   latestTvsEventTime() { return latest_TvsEventTime; };
  void     clear();
  void     open(string url, int rdrid, string radarname, 
		bool realtime = true,
		float historydepth = 3, string axfdir = "");
  bool     connectEventIdx(float loaddepth, bool doclear = false);
  void     start();
  void     close();
  void     setRealTime(bool state = true) { realTime = state; };
  bool     getRealTime() { return realTime; };
  time_t   serverTimeout;      // data lateness allowed before assuming server reconnection reqd
  time_t   lastServerTimeout;  // time of last server timeout
  int      serverTimeoutCount; // count of server timeouts
  int      reconnectCount;     // successful reconnects
  int      reconnectFailCount; // unsuccessful reconnect
  void     checkEventIdx();    // check for server timeout, reconnect if required
  void     setDebug(bool state);
  void     loadHistory(SmartPtr<Index> &newidx, 
		       float loaddepth = -1, bool doclear = false);
  int      loadNew(Index::Record &latestrec, char *selstr, SmartPtr<Index> &newidx);
  int      loadNewer(Index::Record &latestrec, char *selstr, SmartPtr<Index> &newidx);
  int      loadMostCurrent(Index::Record &latestrec, SmartPtr<Index> &newidx);
  void     checkPolling();
  int	   FinishedDataAvail(rdr_scan *finishedscan);
  time_t   newscanEventTime, newscanEventTimeOut;
  time_t   newscanEventScanTime;      // scan time for newscan
  int      newscanEventCount;
  time_t   lastNewscanEventTimeOut;
  int      newscanEventTimeoutCount;
  int      newScanEventRearms;         // count times tmout product not avail, so rearmed timout 
  bool     newScanEventRearmed;        // set flag to only rearm once
  bool     forceReconnectOnScanEventTimeout;
  time_t   sumScanEventToActionDelay, lastScanEventToActionDelay,
    maxScanEventToActionDelay,
    minScanEventToActionDelay;
  int      scanEventToActionCount;
  int      newscanEventAfterActionCount;
  wdssTracksRadarEventHandler(string url, int rdrid, string radarname, 
			      bool realtime = true, float historydepth = 3, 
			      string axfdir = "");
  virtual ~wdssTracksRadarEventHandler();
  void     dumpStatus(FILE *statusfile);
  void     dumpSummary(FILE *statusfile);
  void     writeAxfFile();
};

class wdssManager : public scanEventClient     // container for all tracks for given radar
{
 public:
  multimap<int,wdssTracksRadarEventHandler*> radarHandlers;  // use radar id as array index
  wdssCellIDTracks* getCellIDTracks(int cellID, int rdrid);
  wdssMesoIDTracks* getMesoIDTracks(int cellID, int rdrid);
  wdssTvsIDTracks* getTvsIDTracks(int cellID, int rdrid);
  wdssTracksRadar* getTracksForRadar(int rdrid);
  wdssTracksRadarEventHandler* getLatestTracksForRadar(int rdrid);
  void     deleteTracksBeforeTime(time_t deleteBeforeTime); // delete any tracks that finish before this time
  spinlock *lock;
  string   initFname;
  time_t   nextStatusTime;
  float    initLoadMargin;  // hours of additional wdss data to load before display start time
  float    initLoadDepth; // initial load depth (hours)
  int      statusPeriod;  // default - update status file every 600 secs.
  int      defaultMaxCellID;
  bool     firstInit;
  bool     getLock();
  void     relLock();
  void     clear(int rdrid);
  void     reload();
  void     clear_all();
  wdssTracksRadarEventHandler* addSource(char *url, int rdrid, char *radarname, bool realtime = true, bool debug = false);
  void     addSource(char *clientstr);
  void     close(int rdrid);
  void     close(string &url, int rdrid);
  void     close_all();
  void     init(const char *initfname = NULL);
  wdssManager(char *initfname = NULL);
  virtual ~wdssManager();
  void     newSeqLoaded() {};
  void     printSummary(int rdrid);
  void     writeAxfFile(char *axfPath, int rdrid);
  void	   CheckEvents();
  void     checkPolling();
  void     checkHandlers();
  void     check();           // periodically called by main thread
  time_t   nextCheckTime, checkPeriod;
  virtual int	FinishedDataAvail(rdr_scan *finishedscan);
  virtual void  threadInit();
  virtual void	workProc();
  virtual void	threadExit(); // allow thread stopped tidy up
  void dumpStatus(char *statusfname = NULL);
  void dumpStatus(FILE *statusfile);
};

char *time_t_2str(time_t time, char *strbuff);

extern wdssManager *wdssMng;

#endif
