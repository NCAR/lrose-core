#ifndef __uf_h
#define __uf_h
#include <stdio.h>
#include <stdlib.h>
#define UNDEFFIELD -32768
#include <time.h>
#include <math.h>
#include <malloc.h>
#include <unistd.h>
#include "rdrscan.h"
//#include "rapicradar.h"
#define OPENMODE "w"
#define UFBUFSIZE  16384
#define UFHEADSIZE    32
#define RAPIC_NOT -1

#define MAX_BINS 1024
#define NUMOFFSETS 4 
//make this 2 if possible
#define MAXAZS  360
#define MAX_SWEEPS 30
#define MAXELEVS  (MAX_SWEEPS)
#define LIGHTVEL 299792500.0

#ifndef MAXRAPICGATES 
#define MAXRAPICGATES MAX_BINS
#endif
#ifndef MAXFIELDS
#define MAXFIELDS NUMOFFSETS
#endif
#define FLDUREFL     "UNCORREFL"
#define FLDREFL      "REFL"
#define FLDVEL	      "VEL"
#define FLDWID       "WID"
#define FLDZDR	      "ZDR"
#define FLDNONE      ""

/* defines for sweep_type						*/
#define SWEEP_POINT 	0	/* point mode				*/
#define SWEEP_PPI	1	/* ppi mode				*/
#define SWEEP_RHI	2	/* rhi mode				*/
#define SWEEP_SEC	3	/* sector mode				*/
/* AUST kg added SWEEP_STOP */
#define SWEEP_STOP	7	/* Stop at end of scan mode		*/

#define VOL_POINT SWEEP_POINT	
#define VOL_PPI	  SWEEP_PPI	
#define VOL_RHI   SWEEP_RHI	
#define VOL_SEC   SWEEP_SEC 

extern bool writeRecByteLength;

enum errs  {ERRINSUFMEM,ERRREADOPEN,ERRWRITEOPEN,ERRDISKFULL,ERRINUSE,ERRLAST };
//enum e_scan_type { PPI, RHI, CompPPI, IMAGE, VOL, RHISet, MERGE }; 
//enum e_data_fmt  { RLE_6L_ASC, RLE_16L_ASC }; 
enum RAPICFIELDS { RAPICCZ = 0,RAPICVEL,RAPICWID,RAPICUZ,RAPICZDR,RAPICNONE = -1 };


static struct errorlist {
  errs index;
  char *msg;
  char *msg2;
}errors[] = 
{
  { ERRINSUFMEM		,"Not Enough Memory "		,NULL },
  { ERRREADOPEN		,"Failed to open for Read "	,NULL },
  { ERRWRITEOPEN		,"Failed to Create FIle "	,NULL },
  { ERRDISKFULL		,"Device Full		"	,NULL },
  { ERRINUSE 		,"Invalid Usage		"	,NULL },
  { ERRLAST			,""				,NULL }
};

/*
static float    NARRAY[16] = { 0	, 12.00	, 24.00	, 28.00	, 31.00	, 34.00	, 37.00	, 40.00,
                               43.00	, 46.00	, 49.00	, 52.00	, 55.00	, 58.00	, 61.00	, 64.00
                      };
*/

    #define max(x,y) ((x) > (y) ? (x) : (y))
    #define min(x,y) ((x) < (y) ? (x) : (y))

    #define UFUZ 	0
    #define UFCZ 	1
    #define UFVR 	2
    #define UFSW 	3
    #define UFZD 	4
    #define UFNONE 	-1

//    enum RAPICFIELDS { RAPICCZ = 0,RAPICVEL,RAPICWID,RAPICUZ,RAPICZDR,RAPICNONE = -1 };

    union ufHeader{
    
	char  cid[2];
	short id;	
    };
    struct opData {
    
	    char    projectName[8];
	    short   baselineAimuthx64;
	    short   baselineElevationx64;
	    short   startHour;
	    short   startMinute;
	    short   startSecond;
	    char    fieldName[8];
	    short   consistencyCode;
    
    };
    union opHeader{
    
	    short         words[14];
	    struct opData data;
    
    };
    
    struct luData {
    
	    short cwsectorLimit;
	    short ccwsectorLimit;
	    short topElevLimitx64;
	    short scanType;
	    short noof1stGate;
    
    };
    
    union luHeader{
    
	    short words[5];
	    struct luData  data;
    
    };
    
    struct daData {
    
	    short   noofFieldsinRay;
	    short   noofRecordsinRay;
	    short   noofFieldsinRecord;
    
    };
    
    struct fldHeader {
    
	    char    nameofField[2];
	    short   posofFieldHeader;
	    short   gateData[MAXRAPICGATES+1];
	    short   scale;
    
    };

    union daHeader{
    
	    short    words[3];
	    struct   daData data;
    
    };
    struct maData {
    
	    /* 1 */ 		short uifId;
	    /* 2 */ 		short size;
	    /* 3 */ 		short op01;
	    /* 4 */ 		short lu01;
	    /* 5 */ 		short da01;
	    /* 6 */ 		short recNo;
	    /* 7 */ 		short volumeScanNo;
	    /* 8 */ 		short rayNo;
	    /* 9 */ 		short rayRecNo;
	    /* 10 */ 		short sweepNo;
	    /* 11,12,13,14 */ 	char  radarName[8];
	    /* 15,16,17,18 */ 	char  siteName[8];
	    /* 19 */ 		short latDegree;
	    /* 20 */ 		short latMinute;
	    /* 21 */ 		short latSecondx64;
	    /* 22 */ 		short lonDegree;
	    /* 23 */ 		short lonMinute;
	    /* 24 */ 		short lonSecondx64;
	    /* 25 */ 		short antennaHeight;
	    /* 26 */ 		short dataYear;
	    /* 27 */ 		short dataMonth;
	    /* 28 */ 		short dataDay;
	    /* 29 */ 		short dataHour;
	    /* 30 */ 		short dataMinute;
	    /* 31 */ 		short dataSecond;
	    /* 32 */ 		short timezone;
	    /* 33 */ 		short azimuthx64;
	    /* 34 */ 		short elevationx64;
	    /* 35 */ 		short modeOfAntenna;
	    /* 36 */ 		short fixedAnglex64;
	    /* 37 */ 		short azimuthRate;
	    /* 38 */ 		short createYear;
	    /* 39 */ 		short createMonth;
	    /* 40 */ 		short createDay;
	    /* 41,42,43,44 */ 	char  who[8];
	    /* 45 */ 		short MissingDataFlag;
    
    };
    union maHeader {
    
	    short words[45];
	    struct maData data;
    
    };
    struct firstFieldData {
    
	    short posOfField;
	    short scaleFactor;
	    short kmRangeTo1stGate;
	    short mtRangeTo1stGate;
	    short spacingBetweenGates;
	    short noOfGates;
	    short sampleVolumeDepth;
	    short horizontalBeamwidth;
	    short verticalBeamwidth;
	    short receiverBandwidth;
	    short polarizationCode;
	    short waveLength;
	    short noOfPulsesPerRay;
	    char  thresholdFieldName[2];
	    short thresholdValue;
	    short thresholdScaleFactor;
	    char  editingCode[2];
	    short pulseRepetitionPeriod;
	    short noOfBitsPerDataum;
	    short radarConstant;
	    short noisePower;
	    short receiverGain;
	    short peakTransmittedPower;
	    short antennaGain;
	    short pulseDuration;
    
    };
    
    union firstFieldHeader{
    
	    short words[25];
	    struct firstFieldData data;
    
    } ;

    struct gate 
    {
     float         azimuth;
     float         elev;
     int           ngates;
     long          time;
     unsigned char data[MAXFIELDS][MAXRAPICGATES];
    };

    struct scan 
    {
     int  	seqno;
     long       imageno;
     int  	nscans;
     long  	imagesize;
     long  	thisscan;
     int  	eoi;
     char 	station[25];
     int  	stnid;
     int  	year,month,day;
     int   	hour,min,sec;
     int        timed;
     time_t	unix_time;
     float  	vers;
     int        startrange;
     long       endrange;     //  m.whimpey 7Apr99
     long   	rngres;
     float  	angres;
     int    	vidres;
     int    	pass;
     char   	product[15];
     char   	imgfmt[15];
     float  	elev;
     float  	dbzcor;
     float  	dbmlvl[256];  //SD 15/10/01, was 16
     int    	numdbmlvls;
     int    	nazimuths;
     int        nfields;
     int        scantype;
     float 	nyq;
     RAPICFIELDS field;
     int        lassenfield;
     int        numgates;
     char       timestamp[25];
     float      latitude;
     float      longitude;
     float      height;
     float      frequency;
     float      prf;
     float      pulselength;
     float      anglerate;
     char       unfolding[20];
     int 	fieldlevel[10]; //num levels for each field in gatedata
     struct gate *gatedata; 
    };

    struct rapicradar
    {
      float latitude;
      float longitude;
      char  stationname[35];
      char  radarname[35];
      int   stnid;
      int   altitude;
      int   prf;
      int   type;
      int   wavelength;
      int   npulses;
      int   pulsewidth;
    };

    struct voldata
    {
       int numfields;
       struct rapicradar rparams;
       int    nelevations;
       struct scan *scans;    
    };


    class uf {
    
    public:
    
	uf(bool debug);
	~uf();
	bool openVolume     (char *path,char *name);
	int  closeVolume    (void );
	void setUfParams(struct rapicradar *radPtr,struct scan *scanPtr,int ngates);
	void setnumFields(char *fldname,int &srcfld,int &dstfld);
	void setScanData    (struct scan *scanPtr);
	void debugFields();
	void dumpData(struct voldata *vol);
	
	
    private:
        float rapic2uz(struct scan *scanPtr,unsigned char dat);
	float rapic2vr(struct scan *scanPtr,unsigned char dat,int levels);
	float rapic2sw(unsigned char dat);
	float rapic2zdr(unsigned char dat);
        void todegminsec(float lat,int &deg,int &min,int &sec);
	void			writeData(int fdout,struct scan *scanPtr,long &rlen);
        void                    exitUf(errs ind,char *msg);
        void 			zeroData();
	union ufHeader          ufId;
        union maHeader  	maHeaderData;
	union opHeader  	opHeaderData;
	union luHeader  	luHeaderData;
	struct fldHeader 	fldHeaderData[MAXFIELDS+1];
	union daHeader  	daHeaderData;
	union firstFieldHeader  firstFieldHeaderData;
	int  			fdout;
	int			debug;
	//double			LIGHTVEL ;
	int			volType;
	double			Vu;
	long			spos;
	short			Buffer[UFBUFSIZE];
	short			Fhead [UFHEADSIZE];
	struct rapicradar	rdata;
	int			sweepno;
	int			fangind,vangind;
	int 			maxfields;
	
    };



#endif
