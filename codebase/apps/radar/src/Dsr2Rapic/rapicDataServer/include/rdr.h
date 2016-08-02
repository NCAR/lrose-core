#ifndef __RDR_H
#define __RDR_H

// #define Windows

/*
 *  rdr.h
 *  Radar header file
 */

#ifdef sgi
#include <sys/bsd_types.h>
#include "bool.h"
#endif
typedef unsigned char uchar;

#ifndef Windows
#include <sys/select.h>  //SD add 21/12/99
#include <unistd.h>
#endif

#include <signal.h>
#define _SGI_MP_SOURCE

// extern const float	pi;
extern const float	DEG2RAD;
extern const float	RDRANG2RAD;
extern const float	RAD2DEG;
extern const float	HALF_PI;

extern const float	EARTH_RAD;
extern const float	EARTH_RAD_4_3;
extern const float      ECC_EARTHRAD43;
extern const float      ECC_EARTHRAD;
extern const float      BEAM_REFR;
extern const float      ReSqr;
extern const float      KFtperKm;
extern const float      DegPerKM;
extern const float      KMPerDeg;
extern const float      RdnPerDeg;
extern const float      DegPerRdn;
extern const float      EarthRad;
extern const float      MperStoKMH;
extern const int	JulDay1Jan70;		// julian day of Unix Day 1(1/1/70)
extern const int        SecsPerDay;			// seconds per day

#define FALSE 0
#define TRUE 1

#define CR 13
#define LF 10
#define HASH 35
#define PERCENT 37
#define	EscKey 27
#define CTRLZ	26
#define CTRLC 3
#define EOT '\004'
#define RADLLENGTH 256
#define MAXDBZRES 256		// current max supported dbz threshold resolution

#ifdef AXIS_ENUM_DEFINES
#define X 0
#define Y 1
#define Z 2
#define XY 2
#define XYZ 3
#endif

typedef short   rdr_angle;          // radar angles 10ths of degrees
//typedef unsigned char bool;  SD 21/12/99

inline rdr_angle fToRdrAngle(float fangle)
{
  return short((fangle * 10.0) + 0.5);
};

inline float rdrAngleToF(rdr_angle rangle)
{
  return float(rangle) / 10.0;
};

//typedef unsigned char BOOL;
//typedef int  BOOL;  //SD 21/12/99 contra /usr/include/X11/Xmd.h", line 153
typedef unsigned char BYTE;
typedef unsigned char uchar;

enum UNITS {METRIC,IMPERIAL,NAUTICAL,UNITS_UNDEF};

struct UnitStringStruct {
  char Rstr[8];
  float Rscale;
  char htstr[8];
  float htscale;
  char rngstr[8];
  float rngscale;
  char velstr[8];
  float velscale;
  char doppvelstr[8];
  float doppvelscale;
  void getScaledRString(char *strbuff);
  void getScaledHtString(char *strbuff);
  void getScaledRngString(char *strbuff);
  void getScaledVelString(char *strbuff);
  void getScaledDoppVelString(char *strbuff);
};

extern UnitStringStruct UnitData[];

extern char *cursorval1Labelstrings[];
extern char *cursorval1Unitstrings[];
extern char *cursorval2Labelstrings[];
extern char *cursorval2Unitstrings[];

extern int cursorDataUnits;   // global 

// default units floating pt. km unless specified
struct AzElRng {
		float	Az,El,Rng;				// polar co-ordinate system
		void	clear();
		};

struct LatLongHt {			// Ht in km
  float	Lat,Long,Ht;	// assume Lat +ve SOUTH unless specified
  void	clear();
  bool is_clear();
  bool is_same(LatLongHt *compare);
  void	setval(LatLongHt *newval);
  void	set(float lat, float lng, float ht = 0.0) 
  { Lat = lat; Long = lng; Ht = ht; };
  LatLongHt() { clear();} ;
  LatLongHt(float lat, float lng, float ht = 0.0) 
  { set(lat, lng, ht); } ;  
};			// Long +ve EAST

struct RdrCart {						// Radar relative kmN, kmE kmHt coords	
		float	kmN,kmE,kmHt;			// Ht in km
		void	clear();
		};

struct WorldXYZ {						// centre of world relative X,Y,Z coords
		float x,y,z;						// polar axis - Y axis (+ve North),
		void	clear();
		};											// centre to 0degLONG - +ve Z axis
														// centre to 90degLONG(E) - +ve X axis
struct NthEast {						// km north and east
		float north,east;
		};


extern float ZRa;			// Z-R co-efficients Z = aR^b
extern float ZRb;
void load_zr(char *zr_fname = NULL);
void load_new_zr(char *zr_fname = NULL);

extern int	MaxHt;	// height of max height color palette value
extern int	VILTop;	// height of max height color palette value
extern float terrainHtIncrement;  // set default terrainHtIncrement to 10m
// extern float	AzCorrection;
// bool UseSpaceBall = FALSE;

void drawimg(short mode);

void PasStr2ASCIIZ(char Str[]);

void setrange(float range);
bool end_img_str(char* instr);
bool null_radl(char* instring);

extern bool	QuitMain;
// DebugMode should be set while stepping multi threaded apps.
// It will make the locking class infinitely patient so that lock timeouts
// do not occur
extern bool	DebugMode;  
extern bool	AllowMerge;  
extern unsigned char AutoDBPreview;
extern unsigned char DBMultiStnLoad;
char* versionStr();
// extern int VersionNo;
// extern int VersionBranch;
extern char RapicTitle[];
extern char RapicTitle1[];
extern char RapicTitle2[];
extern int LocalTime;

// void rdr_read_data(int fdes,int imgs_to_load);
// void reset_bytefile();
// extern int   bytefile;

void	TitleScreen();

#ifdef sun
void init_signals();
void sighandler(int signo);
#else
#define _BSD_SIGNALS
void init_signals();
void sighandler(int signo, ... );
#endif

#endif		/* __RDR_H */
