////////////////////////////////////////////////////////////////////
// sRadl class
//
// unpacked radial array class
//
// RAPIC code (c) BOM Australia
//
// Phil Purdam
//
// Jan 2000
//
/////////////////////////////////////////////////////////////////////

#ifndef	_SRADL_HH
#define _SRADL_HH

//(rjp 13 Jul 2009) Increase RADLBUFFSIZE to allow for T_Hills data. 
//const int RADLBUFFSIZE = 1024;
const int RADLBUFFSIZE = 1536;

typedef short rdr_angle; // radar angles 10ths of degrees

enum e_data_type {
  e_refl, 
  e_vel, 
  e_spectw, 
  e_diffz, 
  e_rawrefl, 
  e_rainaccum
};

enum bf_data_type {
  bf_none = 0, 
  bf_refl=1, 
  bf_vel=2, 
  bf_spectw=4, 
  bf_diffz=8, 
  bf_rawrefl=16, 
  bf_rainaccum = 32
};

enum data_mode {
  INDEX, 
  FLOATVALUE, 
  BYTEVALUE
}; // sRadl data type

enum e_unpackedradltype {
  SIMPLE, 
  MULTIMOMENT
};

enum SCANCLIENTTYPE {
  SC_UNDEFINED,
  SC_SEQ, 
  SC_DB, 
  SC_TXDEV, 
  SC_COMMMNG, 
  SC_RAINACC, 
  SC_SATDATAMNG, 
  SC_NEXRADMNG
};

class LevelTable {

public:
  
  LevelTable(short NUMLEVELS, float *InitTbl = 0);
  ~LevelTable();

  e_data_type moment; // moment this table represents
  LevelTable *next, *prev; // linked list of tables
  short numlevels;
  float *Levels; // Level in signed floating point units
  bool GlobalTable; // if true, this is a global table and MUST not be deleted

  // if InitTbl not defined, clear all values to 0
  void SetLevels(short NUMLEVELS, float *InitTbl = 0);

};

// unpacked radial array class

class sRadl {

public:

  sRadl(int BuffSize = 0);
  ~sRadl();

  // zero out this radial
  void Clear();

  // return the array index at the given km rng
  int IndexAtRange(float rng, float *idxrng = 0);

  // return the data index at the given km rng
  char DataAtRange(float rng);

  // convert to tangent plane radial (0deg)
  void TanPlaneRadl(float *cosel = 0);

  // pad radial to given range
  int PadRadl(int padrng);

  // simply threshold float *floatarray values into char *data array
  // other parameters of sRadl need to be set, e.g. rng res etc. 
  // if floatarray set to 0, try to use Values
  // This allows Values to be set to float array for thresholding
  void ThresholdFloat(float *floatarray = 0, int size = 0,
		      LevelTable *thresh_table = 0);

  // convert 16 level char *data array to ASCII_16 radial

  void Encode16lvlAz(char *outstring);
  void Encode16lvlEl(char *outstring);
  void Encode6lvlAz(char *outstring);
  void Encode6lvlEl(char *outstring);
  void EncodeAz(char *outstring);
  void EncodeEl(char *outstring);
  void RngRes2000to1000(); // convert 2000m res radial to 1000m res
  void TruncateData(); // set data_size to last nn-zero data

  // static functions

  static void dump_radl(sRadl* radl);

  static void dump_radl_full(sRadl* radl);

  static int rd_radl_angle(char* instring, rdr_angle& angle);
  
  static bool null_radl(char* instring);
  
  static int RLE_16L_radl(char* instring, sRadl* radl, int maxval);
  
  static int RLE_6L_radl(char* ipbuffer, sRadl* radl);
  
  static void DeltaASCII(unsigned char *Radial, char *OutputRadl, int length);
  
  static void SixLevelASCII(unsigned char *Radial,
			    char *OutputRadl, int length);
  
  static int EncodeBinaryRadl(unsigned char *Radial,
			      unsigned char *OutputRadl, int length);
  
  static int DecodeBinaryRadl(unsigned char* ipbuffer, sRadl* radl);
  
  static int DecodeBinaryRadl(unsigned char* opbuffer,
			      unsigned char *ipbuffer,
			      int maxsize, int &angle);

  // data members

  rdr_angle az,az1,az2; // 10ths of deg, az - centre, az1/az2 width to draw
  rdr_angle az_hr; // hi-res az, other values are rounded to angle res.
  rdr_angle el,el1,el2; // 10ths of deg, el - centre, el1/el2 height to draw
  int startrng; // metres, range of 1st data, normally 4000
  int rngres; // metres, rng res
  int data_size; // max used size of data in buffer
  int buffsize; // buffer size
  int undefinedrng; // rng to which data is undefined, primarily for CAPPI
  short numlevels;
  data_mode mode; // INDEX or VALUE mode
  e_data_type data_type; // moment type of data
  bf_data_type bfdata_type; // moment type of data
  e_unpackedradltype radltype;// data storage method 
  unsigned char *data; // uncompressed radial data (0.5dBZ units of refl)
  
  LevelTable *LvlTbl; // pointer to corresponding level table (0.5dBZ units of refl)

  float *Values; // optional value array (Actual (not 0.5units) dBZ Values)

  // static data members

  static const unsigned short XLAT_16L[256];
  static const char TestRadial[];
  static const char A2NXlat[49];
  static const unsigned char absolute[160];
  static const char Delta[256];
  static const char SixLevelEncode[64];

};


#endif

