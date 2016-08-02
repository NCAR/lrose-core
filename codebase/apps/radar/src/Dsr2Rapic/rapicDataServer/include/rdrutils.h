#ifndef	__RDRUTILS_H
#define __RDRUTILS_H

#include <math.h>
#include <ctype.h>
#include <time.h>
#include "rdr.h"
#include "siteinfo.h"
#include "latlong.h"
#include <stdio.h>
#include <strings.h>


#ifndef sgi
#define fsqrt sqrt
#ifndef linux
inline float fabsf(float x) {return x > 0.0 ? x : -x;}
#endif
#endif


/*

	rdrutils.h

*/

/*
Tabulation of Errors due to approximate Earth Curvature Correction Algorithm                    
Precise:    ht = Re - SQRT((Re*Re) - (rng*rng))                 
Approx:      ht = rng*rng*0.5/Re                        
Re      6371.64 (Earth's Radius)        
All units in km                 
rng     ht      	ht      	Error
	Approx  	Precise 
0       0       0       0.000
50      0.1961818       0.1961849       0.000
100     0.7847273       0.7847757       0.000
150     1.7656365       1.7658812       0.000
200     3.1389093       3.1396828       0.001
250     4.9045458       4.9064349       0.002
300     7.0625459       7.0664644       0.004
350     9.6129097       9.6201722       0.007
400     12.555637       12.568032       0.012
450     15.890728       15.910593       0.020
500     19.618183       19.648478       0.030
550     23.738002       23.782386       0.044
600     28.250184       28.31309        0.063
650     33.154729       33.241441       0.087
*/

extern int ftoi(float input);

inline float circ_corr(float rng, float circ_rad) {
  return(rng * rng / (2 * circ_rad));
  }

/* rng as passed MUST BE km DEFINITELY NOT metres */
inline float EarthCurvCorr43(float rng) {
  return(rng * rng * ECC_EARTHRAD43);
  }

#define Re EARTH_RAD_4_3
/* rng as passed MUST BE km DEFINITELY NOT metres */
/*
 Formula from BMTC Met Fundamentals of Radar Wave Propagation
 Training Notes
*/
inline float BeamHeight(float rng, float SinEl) {
  return(fsqrt((rng * rng) + ReSqr + (2 * rng * Re * SinEl)) - Re);
  }

/* rng as passed MUST BE km DEFINITELY NOT metres */
inline float EarthCurvCorr(float rng) {
  return(rng * rng * ECC_EARTHRAD);
  }

/* rng as passed MUST BE km DEFINITELY NOT metres */
inline float BeamRefrCorr(float rng) {	// correct for beam refraction
  return(rng * rng * BEAM_REFR);	// dif btwn 1&4/3 Earth Curve
  }

inline void  UpperStr(char* Str, char *UpperStr = 0) {
  if (!UpperStr) UpperStr = Str;    // if no upperstr defined convert to Str
  while (*Str)
    *UpperStr++ = toupper(*Str++);
  }

inline int NonWhiteLength(const char *Str) {
    int length = 0;
    const char *temp = Str;
    
    while (*temp)
	if (!isspace(*temp++)) length++;
    return length;
}

extern bool end_img_str(char* instr);
void PasStr2ASCIIZ(char Str[]);

/* following conversion routines use units of kms */
void LatLongHt2XYZ(float lat, float lng, float ht,
		   float &x,  float &y,   float &z, 
		   float radius = EARTH_RAD);
void XYZ2LatLongHt(float &lat, float &lng, float &ht,
		   float x,  float y,   float z, 
		   float radius = EARTH_RAD);
bool RdrCart2LatLong(float RdrLat, float RdrLong, float RdrHt,
		     float kmN, float kmE, float kmHt,
		     float &Lat, float &Long, float &Ht);
bool RdrCart2LatLong(int stnid,
		     float kmN, float kmE, float kmHt,
		     float &Lat, float &Long, float &Ht);
bool RdrCart2LatLong(int stnid,
		     float kmN, float kmE, float kmHt,
		     LatLongHt *destLLH);
bool LatLongKmNE2LatLong(LatLongHt *refLLH,
			 float kmN, float kmE, float kmHt,
			 LatLongHt *destLLH);
void LatLong2RdrCart(float RdrLat, float RdrLong, float RdrHt,
		     float &kmN, float &kmE, float &kmHt,
		     float Lat, float Long, float Ht);
void LatLong2RdrCart(int stnid,
		     float &kmN, float &kmE, float &kmHt,
		     float Lat, float Long, float Ht);

float LatLongKmDiff(float Lat1, float Long1, float Lat2, float Long2, 
		    float *kmn=0, float *kme=0);
float LatLongStnKmDiff(LatLongHt *Org, int stn);
void LatLongStnKmDiff(LatLongHt *Org, int stn, float &kmn, float &kme);
float StnKmDiff(int stn1, int stn2);
void StnKmDiff(int stn1, int stn2, float &kmN, float &kmE);

/*
 * Mercator only meaningful to about +/-80 degrees Lat.
 * Conversions beyond defined limit will fail
 * 
 * New lat involves most calcs, keep last lat and related coeffs
 * Most efficient use is to traverse using const lat.
 * 
 */
class LatLongtoMercator {
    float secAngle, refLong, latLimit;
    float cosSecAngle, acosSecAngle, XCoeff;
public:
    LatLongtoMercator(float secangle = 0.0, float reflong = 140.0, 
	float latlimit = 80.0); 
    void    SetParams(float secangle = 0.0, float reflong = 140.0, 
	float latlimit = 80.0);
    bool LLtoMerc(float Lat, float Long, float *merc_x, float *merc_y); 
    bool MerctoLL(float *Lat, float *Long, float merc_x, float merc_y);
    bool ScaleAtLat(float Lat, float *scale);
};


void julian_date(int jul_day, int &year, int &month, int &day);
int julian_day(int year,int month,int day);
void rpdate2ymd(int rpdate, int &year, int &month, int &day);
int DoY(int year, int month, int day);
time_t DateTime2UnixTime(int year,int month,int day,int hour,
			 int min,int sec,  int InputLocalTime = 0);

void UnixTime2DateTime(time_t UTime,int &year, int &month, int &day,
		     int &hour, int &min, int &sec, int OutputLocalTime = 0);


void UnixTime2DateTime_mktime(time_t UTime,int &year, int &month, int &day,
		     int &hour, int &min, int &sec, int OutputLocalTime = 0);

void UnixTime2DateTimeStr(time_t UTime, int OutputLocalTime, char *DateStr = 0, char *TimeStr = 0);
void UnixTime2DateTimeString(time_t UTime, int OutputLocalTime, char *DateStr);

time_t DateTimeStr2UnixTime(int InputLocalTime, char *DateStr, char *TimeStr);
time_t DateTimeStr8601_2UnixTime(char *str8601);
time_t DateTimeStr2UnixTime(char *DateTimeStr, bool InputLocalTime = false);  // decode yyyymmddhhmm[ss] string to time_t [ss] optional
time_t timet_day(time_t tm, int OutputLocalTime = 0);		// returns day only part of time_t

time_t TimeZone(); // return timezone or altzone as appropriate
// CorrectLocalTime uses global LocalTime if localtime undefined
time_t CorrectLocalTime(int localtime = -1);  
// return appropriate UTC correction for Local time
// if localtime set, else return 0

float dBZtoR(float dBZ, float a, float b, float rainrateclip = 0);
float RtodBZ(float R, float a, float b);

rdr_angle circ_angle(rdr_angle ang1, rdr_angle ang2);

bool CopyFile(char *SrcName, char *DestName);
bool AppendFile(char *SrcName, char *DestName);

/*
	Splits a fully qualified file name into the path and base components
	NO CHECKS ARE DONE ON ARRAY SIZES.
	THE PATH AND BASENAME ARRAYS BETTER BE BIG ENOUGH
*/
void  SplitPName(char *inname, char *path, char *basename);

/*
 * Traverses the stringlist looking for a match against the instring
 * The special word "ANY" will return -1
 * If a match is found the integer posision in the list will be returned
 * Otherwise,  -1 is returned
 * 
 * NOTE: string matching is string position and length dependant
 * 
 */
int String2Int(char *instring, char **stringlist, 
		int listsize, bool casesensitive = false);

void StripTrailingWhite(char *str);

#define gte_lte(val, min, max) ((val >= min) && (val <= max))
#define USRIF_MOUSEMOVED ((lastval[X] != mval[X]) || (lastval[Y] != mval[Y]))
#endif	/* __RDRUTILS_H */
