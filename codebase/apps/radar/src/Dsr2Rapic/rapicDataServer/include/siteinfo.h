#ifndef	__SITEINFO_H
#define	__SITEINFO_H

/*
   ******************************
  *  siteinfo.h                *
  *  Radar Site Info header    *
  *                            *
  ******************************
*/

#include "rdr.h"
#include <string.h>
#include <bitset>
#include <map>
#include <vector>
#include <iostream>
#include "spinlock.h"

using namespace std;

// The site info is data related to each source RAPIC radar data site 

#define  StnDataMax  512	// max number of stations (char) 
#define  StnDefCount 512	// current number of stations defined 
				// can be increased safely
extern const short LastStn;			// id of last stn
#define	 RdrTypeCount 64	// number of radar types defined
#define  SiteInfoFName "siteinfo.txt" // site info data file
#define  DISABLE_COVERAGE_LOAD false

struct  RadarParamType { 
  char 	Name[32];		 // Radar Type name
  int	WaveLength;		 // Wavelength : cm * 10
  int	AzBeamWidth,ElBeamWidth; // BeamWidth  : degrees * 10
};										
class stnSet
{
 public:
  string listStr;
  bitset<StnDataMax> stnset;
  void clear() { stnset.reset(); };
  void set(int stn, bool state = true) 
    { 
      if ((stn >= 0) && (stn < int(stnset.size()))) 
	stnset[stn] = state; 
    };
  void set(string stnstr);
  void set(stnSet &_stnset);
  void setAll()
    {
      for (int stn = 1; stn < int(stnset.size()); stn++)
	set(stn);
    };
  bool isSet(int stn) 
    {
      if ((stn >= 0) && (stn < int(stnset.size()))) 
	return stnset[stn]; 
      else
	return false;
    };
  int size() { return stnset.size(); };
  int stnCount() { return stnset.count(); };
  void merge(stnSet& mergeset) { stnset |= mergeset.stnset; };
  void stnList(string& stnlist, bool clearstr = true); 
  const char *stnListStr();
  int stnNearest(float lat, float lng, float &stndist);
};

// ordered list of stns
// Could be used to step through stns in a particular order
class stnList
{
 public:
  stnSet listSet; // set of stns present in list
  std::vector<int> stnlist;
  void  clear() 
    {
      stnlist.clear();
      listSet.clear();
    };
  void  appendStn(int newstn);
  int   firstStn();  // return 0 if no stns in list
  int   lastStn();   // return 0 if no stns in list
  int   nextStn(int thisstn); // return 0 if thistn not in list
  int   prevStn(int thisstn); // return 0 if thistn not in list
  void  init(char *initstr); // comma separated list of stns by id or name
  int   listPos(int thisstn);// return vector pos for thisstn, -1 if not in list
  int   size() { return stnlist.size(); };
};

class stnSetRefCount
{
 public:
  string listStr;
  string name;
  map<int, int> stnrefmap;
  void clear() { stnrefmap.clear(); };
  bool isSet(int stn) 
    {
      return refCount(stn) > 0;
    }
  bool ref(int stn) // return true if it is a new stn ref
    { 
      //      map<int, int>::iterator iter = stnrefmap.find(stn);
      if (stnrefmap.find(stn) != stnrefmap.end())
	{
	  stnrefmap[stn]++;
	  return false;  // not new stn
	}
      else
	{
	  stnrefmap[stn] = 1;
	  //	  cout << "stnSetRefCount::ref Adding new stn=" 
	  //	       << stn << endl;
	  return true;   // new station - return true
	}
    };
  bool deref(int stn) 
    { 
      map<int, int>::iterator iter = stnrefmap.find(stn);
      if (iter != stnrefmap.end())
	{
	  stnrefmap[stn]--;
	  if (stnrefmap[stn] <= 0)
	    {
	      stnrefmap.erase(iter);
	      cout << "stnSetRefCount::deref Count=0, removing stn=" 
		   << stn << endl;
	      return true;
	    }
	} 
      else 
	cout << "stnSetRefCount::deref Failed - Stn not found=" 
	     << stn << endl;
      return false;
    };
  int refCount(int stn) 
    {
      map<int, int>::iterator iter = stnrefmap.find(stn);
      if (iter != stnrefmap.end())
	return stnrefmap[stn];
      else
	return 0;
    };
  int stnCount() { return stnrefmap.size(); };
  stnSetRefCount()
    { name = "Undefined"; };
  void stnList(string& stnlist, bool clearstr = true); 
  const char *stnListStr(string *stnlist = NULL);
  void getStnSet(stnSet &stnset) // return stnset of ref'd stns
    {
      stnset.clear();
      map<int, int>::iterator iter = stnrefmap.begin();
      map<int, int>::iterator iterend = stnrefmap.end();
      while (iter != iterend)
	{
	  stnset.set(iter->first);
	  iter++;
	}
    }
};

class radarFaultStatus {
 public:
  int radar;             // radar number
  int faultNo;           // rapic data fault number, 0=not faulty
  int prevFaultNo;       // rapic data fault number, 0=not faulty
  string lastFaultStr;   // fault description
  time_t lastStartTime,  // time last or current fault started
    lastResolvedTime,    // time last fault resolved
    lastUpdateTime;
  radarFaultStatus()
    {
      radar = faultNo = prevFaultNo = 0;
      lastStartTime = lastResolvedTime = lastUpdateTime = 0;
      lastFaultStr = "NO FAULT";
    };
  radarFaultStatus(int radarno)
    {
      setRadar(radarno);
      faultNo = 0;
      lastStartTime = lastResolvedTime = 0;
      lastFaultStr = "NO FAULT";
    };
  void setRadar(int radarno) { radar = radarno; };
  bool setFaulty(int faultno = 0,         // rapic data fault number
		 char* faultstr = NULL,   // default to "FAULT", should really be specified
		 time_t fault_time = 0);  // if not defined use now
  bool setFaultCleared(time_t resolved_time = 0); // if not defined use now
  int fault() { return faultNo; };
  string& faultString() { return lastFaultStr; };
  time_t lastFaultStartedTime() { return lastStartTime; };
  time_t lastFaultResolvedTime() { return lastResolvedTime; };
  void dumpStatus(time_t dumptime,
		  FILE *dumpfile = NULL,
		  FILE *mcdumpfile = NULL,
		  bool faulty_only = false,   // if false don't write if not faulty 
		  bool show_last = true);     // if true write last fault info if not faulty
};

class currentRadarFaultStatus {
  spinlock *lock;
  stnSet   faultySet;
 public:
  string statusFilePath;
  radarFaultStatus notdefinedStatus;
  map<int, radarFaultStatus> faultStatus;  // only populate as fault state is set per radar
  // return true if a new fault, else if pre-existing fault return false
  bool setFaulty(int rdr, 
		 int faultno = 0,         // rapic data fault number
		 char* faultstr = NULL,   // default to "FAULT", should really be specified
		 time_t fault_time = 0);  // if not defined use now
  void setFaultCleared(int rdr, 
		       time_t resolved_time = 0); // if not defined use now
  int fault(int rdr);
  string& faultString(int rdr);
  time_t faultStartedTime(int rdr);
  time_t faultResolvedTime(int rdr);
  void dumpStatus(char *dumpfilename = NULL);
  currentRadarFaultStatus();
  ~currentRadarFaultStatus();
  void getFaultyRadarSet(stnSet &faultyset);
};

extern currentRadarFaultStatus currentRadarFaults;

class cCoverage {
public:
    float   *horizon1;    // polar array of range to first horizon
    float   *horizon2;    // 0 to 359 degrees, range in km
    float   hor1_height, hor2_height;
    bool    valid;
    bool quiet;
    cCoverage(char *fname = 0)
    {
        horizon1 = horizon2 = NULL;
        valid = false;
	quiet = true;
        if (fname)
            readFile(fname);
    };
    ~cCoverage()
    {
        if (horizon1)
        {
            delete[] horizon1;
            horizon1 = 0;
        };
        if (horizon2)
        {
            delete[] horizon2;
            horizon2 = 0;
        };
    
    }
    bool readFile(char *fname);
};


typedef  RadarParamType RadarArrayType[RdrTypeCount];

typedef  char StnIDType;	// -1 signals invalid station

class  StnRecType {
private:
  float lat,lng;		// pos latitude south, pos long. east
  float	Alt;			// Altitude, km above sea level
public:
  char  Name[32];		// Radar Stn name
  char  fName[32];		// File Safe Radar Stn name
  int  	Rdr;			// type of radar at site
  bool  valid;
  cCoverage * coverage;
  StnRecType()
  {
    coverage = 0;
    valid = false;
    strcpy(Name, "Undefined");
  }
  ~StnRecType()
  {
    if (coverage)
    {
        delete coverage;
        coverage = 0;
    }
  }

  void  GetLatLngHt(LatLongHt *latlonght) {
    latlonght->Lat = lat;
    latlonght->Long = lng;
    latlonght->Ht = float(Alt);
    }

  float Lat() { return lat; };
  float Lng() { return lng; };
  float Ht() { return Alt; };
  void set(float lAT, float lNG, float hT)
    {
      lat = lAT; lng = lNG; Alt = hT;
    };


  void safeFileName(char *namebuff);
  }; 

inline bool stnIDOK(int stnid)
{
  return ((stnid >=0) && (stnid < StnDefCount));
};

char* stn_name(int stn, bool fsafe = false);
int decode_stnstr(const char *stnstr, bool CaseSensitive = false);
int   get_stn_id(const char *name, bool CaseSensitive = true);
float ElBeamWidth(int stn);
float AzBeamWidth(int stn);

typedef  StnRecType StnRecArray[StnDefCount];
typedef	 RadarParamType RdrRecArray[RdrTypeCount];
typedef  StnIDType StnArrayType[StnDefCount - 1];

extern StnRecArray StnRec;
extern RdrRecArray RdrRec;
/*
struct  SortedStnRecType { 
  short Size;
  StnArrayType StnArray;
	};
*/
/*
enum  RadarTypes{NoRadar,WF44,WF100_6,WF100_8,WSR81C_8,WSR74C_14,
                WSR81S,Spare2,Spare3,Spare4,Spare5,Spare6,
                Spare7,Spare8,Spare9,Spare10,Spare11,Spare12,
                Spare13,Spare14};
*/
/*
  enum Stations { blank  ,CampRd ,Melb   ,Sydney ,Wtown  , 
              Carnvn ,Gerlton,Bris   ,Kanign ,Gove   ,
              Darwin ,Adel   ,Perth  ,TEST   ,Gambier,
              Dampier,PHedld ,Broome ,Weipa  ,Cairns ,
              Twnsvl ,MtStrt ,Mackay ,Gladstn,Mascot ,
              AliceSp,PrthAP ,Woomera,ByrBay ,Lrmonth,
              Mildura,Albany ,Esprnce,Ceduna ,Lncestn,
              CoffsH ,GlfCarp,Hobart ,Charlvl,Moresby,
              RDRERR ,Willis ,Tindal ,BrisAP ,STN44  ,
              STN45  ,STN46  ,STN47  ,STN48  ,STN49  ,
              STN50  ,STN51  ,STN52  ,STN53  ,STN54  ,
              STN55  ,STN56  ,STN57  ,STN58  ,STN59  ,
              STN60  ,Subang ,WkShop ,TOGA   };
*/
/*
Stations SeqStnOrder[LastStn] of Stations =
  (Weipa,Moresby,Cairns,Willis,MtStrt,Twnsvl,Mackay,
   Gladstn,Kanign,
   Bris,BrisAP,ByrBay,CoffsH,Charlvl,Wtown,Mascot,Sydney,
   Lncestn,Hobart,WkShop,CampRd,Melb,
   Gambier,Mildura,Adel,Woomera,Ceduna,Esprnce,Albany,
   Perth,PrthAP,Gerlton,Carnvn,Lrmonth,Dampier,PHedld,
   Broome,AliceSp,Tindal,Darwin,TOGA,Gove,GlfCarp,TEST,blank,
   RDRERR ,STN44  ,
   STN45  ,STN46  ,STN47  ,STN48  ,STN49  ,
   STN50  ,STN51  ,STN52  ,STN53  ,STN54  ,
   STN55  ,STN56  ,STN57  ,STN58  ,STN59  ,
   STN60  ,Subang);
*/

void  GetSiteInfo(bool loadCoverage = true);
void  ReadCoverageData(int StnID);

bool radlVelLockedOut(int stnid);   // return if specified stn is "radlVel locked

#endif	/* __SITEINFO_H */
