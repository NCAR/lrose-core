/*

  readNexRad.h

  Declaration of readNexRad class

*/

#ifndef __READNEXRAD__H
#define __READNEXRAD__H

#include <vector>

#define NXR_NEXRAD 0
#define NXR_CINRAD 1

enum nexradType { NRT_NEXRAD, NRT_CINRAD };
enum nexradRadlStatus 
  { 
    NRS_NEW_ELEV, NRS_INTERM_RADL, NRS_END_ELEV, 
    NRS_BEGIN_VOL, NRS_END_VOL, NRS_UNDEF 
  };
struct nexradTitleBlock
{
  char                  fname_root[9]; // "ARCHIVE2."
  char                  fname_ext[3];  // "1" "2" etc
  unsigned int          julDate;       // Modified Julian Date (from 1/1/70) 
  unsigned int          mSeconds;      // file created msecs of day from 0000Z
  unsigned char         unused[4];
  void                  clear();
  nexradTitleBlock() { clear(); };
};

/*
  The first 64 half-words(hw) reserved for the header
  THIS IS A TEMPLATE STRUCTURE WITH VALUES BEING READ
  DIRECTLY INTO IT AS A BLOCK
  DO NOT ADD ANY VARIABLES TO THIS STRUCTURE
*/

class nexradHeader
{
 public:
  // Channel Terminal Manager (CTM) info hw 1-6 (bytes 0 - 11)
  unsigned short	unused1[2]; // unable to find usage of these bytes 
  unsigned short	mssgSizeBytes; // total mssg size in bytes from start?
  unsigned short	unused4[3]; // unable to find usage of these bytes 
  // Message Header hw 7-14 (bytes 12 - 27)
  // 
  unsigned short	mssgSizeHW;    // in hws from this hw(7) to end of record
                                       // excluding trailing FCS (4 bytes)??
                                       // total HWs = 6 + mssgSizeHW + 2
  unsigned short        chID_mssgType; // ??chID >> 8?, ??MssgType & $ff?
  unsigned short	seqID;         // I.D. Sequence 0-7FFF then roll back to 0
  unsigned short	mssgJulDate;   // Modified Julian Date (from 1/1/70) 
  unsigned int  	mssgMSeconds;  // mssg generation time msecs since 0000Z
  unsigned short	mssgSegments;  // Number of mssg segs, 
  unsigned short	mssgSegment;   // mssg segment number
  // Digital Radar Data Header hw 15-64 (bytes 28 - 127)
  unsigned int		radlMSeconds;  // radial time msecs since 00Z
  unsigned short	radlJulDate;   // Modified Julian Date (from 1/1/70) 
  unsigned short	_unambRange;    // unambiguous rng - (val/10.0 = km)
  unsigned short	_az;            // az angle ([val/8]*[180/4096] = deg)
  unsigned short	radlNumber;    // radial number in el scan
  unsigned short	_radlStatus;    // 0-new el, 1-inter radl, 2-end el
                                       // 3-new vol 4-end vol
  unsigned short	_el;            // (val/8)*(180/4096) = deg
  unsigned short	elNumber;      // el num in vol scan
  short			rngToFirstReflGate; // (m) can be negative
  short			rngToFirstDoppGate; // (m)can be negative
  unsigned short	reflGateSize;  // (m)
  unsigned short	doppGateSize;  // (m) radl vel & sp wid
  unsigned short	reflNumGates;
  unsigned short	doppNumGates;
  unsigned short	cutSectorNumber; // sector number within cut
  unsigned int		calConst;      //  system gain cal const (db biased)
  unsigned short	reflPtr;       // refl data ofs from start of DRDH (byte 28)
  unsigned short	velPtr;	       // vel data ofs from start of DRDH (byte 28)
  unsigned short	spwidPtr;      // spwid data ofs from start of DRDH (byte 28)
  unsigned short	_velRes;        // 2=0.5m/s 4=1.0m/s
  unsigned short	vcpNum;
  unsigned short	unused38[4];
  unsigned short	reflPtrArch;   // refl ptr for Archive II plajback
  unsigned short	velPtrArch;    // vel ptr for Archive II plajback
  unsigned short	spwidPtrArch;  // spwid ptr for Archive II plajback
  unsigned short	_nyquist;       // val/100 = m/s
  unsigned short        _atmAttenFactor;// val/1000.0 = db/km 
  unsigned short        _rngAmbigThresh;// thresh for min diff for rng ambig label (
                                       // (val/10  watts)
  unsigned short        unused48; 
  unsigned short        circleTotal;   // ??
  unsigned short 	unused50[15];  // rest of the Header block
  float                 unambRange() 
  { 
    return float(_unambRange)/10.0; 
  };
  float                 az() 
  { 
    return (float(_az)/8.)*(180./4096.); 
  };
  char                  *radlStatusStr();
  float                 el() 
  { 
    return (float(_el)/8.)*(180./4096.); 
  };
  float                 velRes() 
    { 
      if (_velRes == 2) return 0.5;
      else return 1.0;
    };
  float                 nyquist() 
  { 
    return (float(_nyquist)/100.); 
  };
  float                 atmAttenFactor() 
  { 
    return float((_atmAttenFactor)/1000.); 
  };
  float                 rngAmbigThresh() 
  { 
    return float((_rngAmbigThresh)/10.); 
  };    
  void dumpHeader(int debuglevel); 
  void fixByteOrder(nexradType nrtype, int maxNumGates = 2048);
  unsigned short ftohs(nexradType nrtype, short in);
  short ftohss(nexradType nrtype, short in);
  unsigned long ftohl(nexradType nrtype, long in);
  time_t radlTime()
  {
    return ((radlJulDate-1) * 24 * 60 * 60)
      + radlMSeconds/1000;
  };
  nexradRadlStatus radlStatus() 
    { 
      if (_radlStatus <= 4)
	return nexradRadlStatus(_radlStatus);
      else
	return NRS_UNDEF;
    };
  bool sanityCheckOK();  // conduct sanity check, return false if not OK
};

class nexradRadial
{
 public:
  nexradTitleBlock      titleBlock;
  bool                  titleBlockValid;
  nexradHeader          header;
  bool                  headerValid;

  unsigned char		*ReflData;
  int                   ReflBuffSize;
  unsigned char		*VelData;  
  int                   VelBuffSize;
  unsigned char		*SWData;  
  int                   SWBuffSize;
  long                  startRecordPos;
  int                   maxNumGates;

  nexradRadial();
  ~nexradRadial();
  nexradType nrtype;
  bool readHeader(FILE *nexradfile);
  bool readData(FILE *nexradfile);
  bool readRadial(FILE *nexradfile);
  void dumpHeader(int debuglevel); 
  bool endOfVol();
  time_t radlTime() { return header.radlTime(); };
  bool hasRefl() { return header.reflPtr != 0; };
  bool hasVel() { return header.velPtr != 0; };
  bool hasSpWid() { return header.spwidPtr != 0; };
};

typedef vector<float>::iterator float_vec_iter;

class nexradScanBuff
{
  static const int CODE_INVALID = 0;
  static const int CODE_RANFOLD = 1;

  static const float VALUE_INVALID = -999.;
  static const float VALUE_RANFOLD =  999.;

 public:
  vector<float> reflData;
  vector<float> velData;
  vector<float> spwidData;
  vector<bool> reflRadlFlags;
  vector<bool> velRadlFlags;
  vector<bool> spwidRadlFlags;
  float elev;          // assigned elevation of this
  float elevFromHist;  // elevation from histogram
  time_t elevTime;
  int reflBins, doppBins,
    numRadls;
  int reflGateSize, doppGateSize;
  int rngToFirstReflGate, rngToFirstDoppGate;
  float nyquist;
  nexradScanBuff(int reflbins, int doppbins, int numradls = 360);
  nexradScanBuff(nexradRadial &newradial, int numradls = 360);
  ~nexradScanBuff();
  void init(int reflbins, int doppbins, int numradls = 360);
  // if bins  or radls <= 0, don't change
  
  // calling resize with any vals 0 will not change that value
  void resize(int reflbins, int doppbins, int numradls = -1);
  void resizeNumRadls(int numradls);
  void resizeRefl(int reflbins, int numradls = -1);
  void resizeDopp(int doppbins, int numradls = -1);
  void reset(nexradRadial &newradial);
  float lastClearVal;
  void clear();
  void clear(float clearval);
  void addRadl(nexradRadial &addradl);
  float decodeSpWid(unsigned char code);
  float decodeVel(unsigned char code, float scale_factor);
  float decodeRefl(unsigned char code);
  void fillMissingRadials();
  float_vec_iter beginReflRadlAz(int azindex)
    {
      if ((azindex >= 0) && 
	  (azindex < numRadls))
	return reflData.begin() + (azindex * reflBins);
      else
	return reflData.end();
    };
  float_vec_iter beginVelRadlAz(int azindex)
    {
      if ((azindex >= 0) && 
	  (azindex < numRadls))
	return velData.begin() + (azindex * doppBins);
      else
	return velData.end();
    };
  float_vec_iter beginSpWidRadlAz(int azindex)
    {
      if ((azindex >= 0) && 
	  (azindex < numRadls))
	return spwidData.begin() + (azindex * doppBins);
      else
	return spwidData.end();
    };
};

class readNexRad
{
 public:
  static const int CODE_INVALID = 0;
  static const int CODE_RANFOLD = 1;

  static const float VALUE_INVALID = -999.;
  static const float VALUE_RANFOLD =  999.;

  int HIST_SIZE;

  float RADIAN;
  
  vector<nexradScanBuff*> nexradScans;  // use pointers 
  void newElevScan(int elindex, nexradRadial &newradial);

  int NumValidCuts;
  int debug;
  float elTolerance;
  
  readNexRad();
  ~readNexRad();
  //bool SavedataIntoFiles();
  // if createRapicScan set, create an "in-memory" rdr_scan instance
  // otherwise write a rapic format file based on the input filename
  bool ReadNexradData(char *filename,char *radtype,
		      int station, bool createRapicScan = false);
  unsigned short ftohs(int rtype, short in);
  short ftohss(int rtype, short in);
  unsigned long ftohl(int rtype, long in);
  void addElevHist(float elev, int elevHist[], float elevValues[], int *elevCount);
  float getBestElev(int elevHist[], float elevValues[], int elevCount);
/*   void fillMissingRadials(bool *flag, float *data, int elev_count, int numGates); */
  nexradRadial oneRadial;
  
  char nex_titleblock[24];  // nexrad title block is 24 bytes


  // 2 modes available for rapic scan conversion
  // rapic scan file creation - if createRapicScan flag is false
  // rdr_scan instance creation - if createRapicScan flag is true

  int createScanProducts(int rfd, 
			 int elindex, int total_tilts,
			 int scan_count, int total_scans,
			 int station, bool createRapicScan = false);
  
  void createScanProduct(int rfd, 
			 float_vec_iter data_iter,
			 e_data_type type, 
			 int numRadls, 
			 int numGates, int gateSize, 
			 int rngToFirstGate, 
			 float nyquist,
			 int tilt_num, int total_tilts,
			 int scan_count, int total_scans,
			 float ElevAngle,time_t scanTime,
			 int station, bool createRapicScan = false);

  rdr_scan      *createdScan;
  void          derefCreatedScan();

  // biggest required dest_array is kept until instance deleted
  unsigned char *dest_array;
  int           dest_array_size;
  int		dest_xdim, dest_ydim;
    
  //bool	valid;
  int		scan_vol_no;	    //scan no. in this volume
  int		minNeighbors;	    // set min neighbors for filter
  int		newRange;
/*   int		s1fd; //output file handle */
  int		scancount;
  int		dummy;
  time_t        startTime;
  time_t        lastTime;
};

#endif
