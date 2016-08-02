/*
 * obsData.h - 
 * */
#ifndef __OBSDATA_H
#define __OBSDATA_H

#include <map>
#include <string>
#include "draw.h"
#include "spinlock.h"

// class to store fixed point 2 decimal place values as shorts
// i.e. approx range +/- 320.00
// Need to check for class overheads vs savings - 
// "this" pointer alone makes per item class inefficient
// maybe a fixed point container for multiple vals would be worth it

class shVal100
{
 private:
  short _shval;
 public:
  shVal100(float _val = 0.0) { set(_val); };
  void set(float _val = 0.0) { _shval = short((_val * 100.0) + 0.5); };
  float get() { return float(_shval) / 100.0; };
  float operator=(float rhs) 
    { 
      set(rhs); 
      return get();
    };
/*   float operator=() { return get(); }; */
};

inline bool shObsUndef(short val)
{
  return (val == SHRT_MIN);
};

char *shObsStr(short val, int scale,  char *precstr, char *outstr);

enum obsType { ot_base, ot_fixed, ot_moving };

/*
  need to keep this as compact as possible
  stores values fixed point as appropriate
  Most use 2 dec pts
  press uses 1 dec pt
*/
class baseObsData
{
 private:
  short _ws, _wd, _wg,
    _temp, _dp;
  unsigned short _spress;
 public:
  baseObsData();
  baseObsData(time_t _tm, int _stn,
	      float ws, float wd, float tmp,  // required fields
	      float wg = -1.0, float dp = -9999.0, // optional fields 
	      float spress = 0.0); 
  virtual void init();
  void set(time_t _tm, int _stn,
	   float ws, float wd, float tmp,  // required fields
	   float wg = -1.0, float dp = -9999.0, // optional fields 
	   float spress = 0.0); 
  virtual ~baseObsData();
  virtual void setWS(float _val) { _ws = short((_val * 100.0) + 0.5); };
  virtual float getWS() { return float(_ws) / 100.0; };
  virtual void setWD(float _val) { _wd = short((_val * 10.0) + 0.5); };
  virtual float getWD() { return float(_wd) / 10.0; };
  virtual void setWG(float _val) { _wg = short((_val * 100.0) + 0.5); };
  virtual float getWG() { return float(_wg) / 100.0; };
  virtual void setTemp(float _val) { _temp = short((_val * 100.0) + 0.5); };
  virtual float getTemp() { return float(_temp) / 100.0; };
  virtual void setDP(float _val) { _dp = short((_val * 100.0) + 0.5); };
  virtual float getDP() { return float(_dp) / 100.0; };
  virtual void setSPress(float _val) { _spress = short((_val * 10.0) + 0.5); };
  virtual float getSPress() { return float(_spress) / 10.0; };
  virtual void setTm(time_t _tm) { tm = _tm; };
  virtual time_t getTm() { return tm; };
  virtual void setStn(int _stn) { stn = _stn; };
  virtual time_t getStn() { return stn; };
  virtual float getRain9am() { return -999; };
  virtual float getRain10() { return -999; };
  obsType obstype;
  int stn;
  time_t tm;
  bool valid;
  virtual void dumphdr(FILE *dumpfile, bool endline = true);
  virtual void dump(FILE *dumpfile, bool endline = true);
};

class fixedObsData : public baseObsData
{
 private:
  unsigned short _rain9am, // in 0.1mm
    _rain10;               // in 0.01mm
 public:
  fixedObsData();
  fixedObsData(time_t _tm, int _stn,
	       float ws, float wd, float tmp,  // required fields
	       float rain9am, float rain10,
	       float wg = -1.0, float dp = -9999.0, // optional fields 
	       float spress = 0.0); 
  virtual void init();
  void set(time_t _tm, int _stn,
	   float ws, float wd, float tmp,  // required fields
	   float rain9am, float rain10,
	   float wg = -1.0, float dp = -9999.0, // optional fields 
	   float spress = 0.0); 
  virtual ~fixedObsData();
  void setRain9am(float _val) { _rain9am = short((_val * 10.0) + 0.5); };
  virtual float getRain9am() { return float(_rain9am) / 10.0; };
  void setRain10(float _val) { _rain10 = short((_val * 100.0) + 0.5); };
  virtual float getRain10() { return float(_rain10) / 100.0; };
  virtual void dumphdr(FILE *dumpfile, bool endline = true);
  virtual void dump(FILE *dumpfile, bool endlin = true);
};

class movingObsData : public baseObsData
{
 public:
  movingObsData();
  virtual void init();
  float lat,lng,ht;
  virtual void dumphdr(FILE *dumpfile, bool endline = true);
  virtual void dump(FILE *dumpfile, bool endlin = true);
};

class lightningData
{
 public:
  lightningData()
    {
      init();
    };
  lightningData(time_t _tm, float _lat, float _lng, float _kamps)
    {
      set(_tm, _lat, _lng, _kamps);
    };
  virtual ~lightningData()
    {};
  virtual void init()
    {
      tm = 0;
      lat = lng = kamps = 0.0;
    };
  void set(time_t _tm, float _lat, float _lng, float _kamps);
  time_t tm;
  float lat,lng,kamps;
  virtual void dumphdr(FILE *dumpfile, bool endline = true);
  virtual void dump(FILE *dumpfile, bool endlin = true);
};

extern bool useObsDataFreelist;
// return new baseObsData from freelist if avail else alloc
baseObsData* newBaseObsData();
// return new fixedObsData from freelist if avail else alloc
fixedObsData* newFixedObsData();
// return new fixedObsData from freelist if avail else alloc
movingObsData* newMovingObsData();
// add generic baseObsData instance to appropriate freelist
void freeObsData(baseObsData *freeobs);
// add generic baseObsData instance to appropriate freelist
// return new fixedObsData from freelist if avail else alloc
lightningData* newLightningData();
void freeLightningData(lightningData *freeobs);
// clear obsDataFreeLists
void ClearFreeObsData();
int freeObsDataSize();

class obsStn
{
 public:
  obsStn(int _stn, int _site,
	 float _lat, float _lng, float _ht,
	 std::string _name, std::string _station);
  obsStn();
  void init();
  void set(int _stn, int _site,
	   float _lat, float _lng, float _ht,
	   std::string _name, std::string _station);
  virtual ~obsStn();
  int stn, site;
  float lat, lng, ht;
  std::string name, station;
  obsType obstype;
  void dump(FILE *dumpfile);
  obsType getType();
  bool inLLBoundingBox(llBoundingBox &llbb)
    {
      return llbb.inBox(lat, lng);
    };
};

class obsStnMap
{
 private:
  spinlock *mapLock;
 public:
  std::map<int, obsStn*> stnMap;    
  obsStnMap();
  virtual ~obsStnMap();
  bool addObsStn(obsStn *newstn);  // return true if added, false if duplicate (not added)
  bool obsStnExists(int _stn);
  obsStn* getObsStn(int _stn);        // return NULL if not found
  void clear();                       // delete all obsStn instances and erase map
  void dump(FILE *dumpfile);
  void dump(char *fname);
};

typedef std::map<time_t, baseObsData*> obsDataMap_type;
typedef std::map<time_t, baseObsData*>::iterator obsDataMap_iter;

/*  maintain a map of obsData for a given stn */
/* map of baseObsData - users need to check actual obs type before use */
class stnObsData
{
 private:
  spinlock *mapLock;
 public:
  int stnID;                    // id corresponding to globalObsStnMap
  obsStn *_obsstn;              // point to corresponding obsStn
  obsStn *obsstn();             // point to corresponding obsStn
  obsType obstype;
  stnObsData(int stn);
  virtual ~stnObsData();
  void init();
  void check(bool uselock = true);
  std::map<time_t, baseObsData*> obsDataMap;
  void clear();                  // delete all obsData instances and erase map
  void purgeBefore(time_t beforetm);
  void purgeAfter(time_t aftertm);
  // addObs - returns true if ref kept to newobs - caller relinquishes 
  //   ownership - Caller should de-reference newobs
  // else no reference kept, caller keeps ownership of newobs
  bool obsExists(time_t obstime);
  // if tm_window not defined get any obs >= time
  baseObsData* firstObsAfterTm(time_t obstime, time_t tm_window = -1); 
  baseObsData* obsNearestTm(time_t obstime, time_t tm_window = -1); 
  bool addObs(baseObsData *newobs);
  time_t firstTm, lastTm;
  int obsCount;
  long dataSize();
  void dump(FILE *dumpfile, bool summary = false);
  bool inLLBoundingBox(llBoundingBox &llbb);
};

typedef std::map<int, stnObsData*> stnObsDataMap_type;
typedef std::map<int, stnObsData*>::iterator stnObsDataMap_iter;

class stnObsDataSet
{
 public:
  stnObsDataSet()
    {
      totalObsStns = 0;
    };
  stnObsDataMap_type stnObsDataMap;
  void clear()
    {
      stnObsDataMap.clear();
    };
  void add(int stn, stnObsData* stnobsdata)
    {
      stnObsDataMap[stn] = stnobsdata;
    };
  int totalObsStns;   // keep total stns count for delta checks 
  llBoundingBox llbb; // keep llbb for delta checks
};

typedef std::multimap<time_t, lightningData*> lightningDataMMap_type;
typedef std::multimap<time_t, lightningData*>::iterator lightningDataMMap_iter;

class lightningPeriodIters
{
 public:
  time_t starttime, endtime;
  lightningDataMMap_iter iter_start,
    iter_end, mmap_end;
  bool valid;
};

class obsDataMng
{
 public:
  time_t purgeDepth;  // seconds of data to keep
  time_t firstTm, lastTm;
  bool debug;
  obsDataMng();
  virtual ~obsDataMng();
  stnObsDataMap_type stnObsDataMap;
  // addObs - returns true if ref kept to newobs - caller relinquishes 
  //   ownership - Caller should de-reference newobs
  // else no reference kept, caller keeps ownership of newobs
  void purgeBefore(time_t beforetm);
  void purgeAfter(time_t aftertm);
  bool addObs(baseObsData *newobs);
  
  spinlock *lightningLock;
  lightningDataMMap_type lightningDataMMap;
  bool addLightning(lightningData *newobs);
  // NOTE - getting iters LOCKS the lightning map
  // ENSURE that relLightningLock is called when finished
  bool getLightningPeriodIters(lightningPeriodIters &lightning_iters);
  void relLightningLock()
    {
      lightningLock->rel_lock();
    };

  void clear();
  long dataSize();
  void dump(FILE *dumpfile, bool summary = false);
  void dump(char *fname, bool summary = false);
  void getStnsInLLBB(llBoundingBox &llbb, stnObsDataSet &llBoxStnSet);
  void check();
  void setReloadSettings();
};

extern obsStnMap *globalObsStnMap;
extern obsDataMng *globalObsData;
extern obsStnMap* getGlobalObsStnMap();
extern obsStn* getGlobalObsStn(int _stn);
extern obsDataMng* getGlobalObsData(bool autoCreate = true);
extern bool reloadDrawObsSettings;
extern bool reloadDrawLtningSettings;

#endif
