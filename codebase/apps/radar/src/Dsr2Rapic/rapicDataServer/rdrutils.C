#ifdef RAPICTXMODS
#include "..\stdafx.h"   // helps speed compile through pre-compiled headers
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

#include "rdr.h"
#include "rdrutils.h"
#ifndef NO_XWIN_GUI
#include "maps.h"
#endif

#ifndef sgi
#define fsin sin
#define fcos cos
#define fasin asin
#define facos acos
#define fhypot hypot
#endif

#ifdef WIN32
extern int strcasecmp( char *string1, char *string2 );
extern int strncasecmp( char *string1, char *string2, size_t count);
#endif


int ftoi(float input) {
  return(int)(input + 0.5);
}


LatLongConvert GPLatLongCnvt;

void LatLongHt::clear() {
    Lat = Long = Ht = 0.0;
}

bool LatLongHt::is_clear() {
    return ((Lat == 0.0) && (Long == 0.0) && (Ht == 0.0));
}

bool LatLongHt::is_same(LatLongHt *compare) {
    return (compare &&
	    (Lat == compare->Lat) && 
	    (Long == compare->Long) && 
	    (Ht == compare->Ht));
}

void LatLongHt::setval(LatLongHt *newval) {
    Lat = newval->Lat;
    Long = newval->Long;
    Ht = newval->Ht;
}

void RdrCart::clear() {
    kmN = kmE = kmHt = 0.0;
}

void WorldXYZ::clear() {
    x = y = z = 0.0;
}

void PasStr2ASCIIZ(char Str[]) {
	char	StrLen;

	StrLen = Str[0];
	memcpy(&Str[0],&Str[1],StrLen);
	Str[StrLen] = 0;
	}

// Convert LAT/Long/Height to XYZ
// Lat/Long. in degrees +ve NORTH +ve EAST
// XYZ is kms relative to the centre of the earth 
//	Y axis - polar axis, X axis - +/- 90deg Long axis, 
// 	Z axis - 0,180 deg Long axis
//
// CURRENT IMPLEMENTATION BASED ON SPHERICAL EARTH (6371.64 kms)

void LatLongHt2XYZ(float lat, float lng, float ht,
		     float &x, float &y, float &z, 
		     float radius) {
	float	sinlat,sinlng,coslat,coslng,rad;

	sinlat = fsin(lat*DEG2RAD);
	sinlng = fsin(lng*DEG2RAD);
	coslat = fcos(lat*DEG2RAD);
	coslng = fcos(lng*DEG2RAD);
	rad = radius + ht;
	y = rad * sinlat;
	x = rad * coslat * sinlng;
	z = rad * coslat * coslng;
	}

void XYZ2LatLongHt(float &lat, float &lng, float &ht,
		     float x, float y, float z, 
		     float radius) {
	float	sinlat,coslat,coslng,rad;

	rad = fsqrt((x*x)+(y*y)+(z*z));
	ht = rad - radius;
	sinlat = y / rad;
	lat = fasin(sinlat)*RAD2DEG;
	coslat = fcos(lat*DEG2RAD);
	coslng = z / (rad * coslat);
	lng = facos(coslng)*RAD2DEG;
	if (x < 0) lng = 360. - lng;
	}

// RdrCart is the radar origin relative cartesian co-ordinates
// kmN, kmE, Ht relative to a tangent plane cartesian coordinate system
// based on the radar origin
// 

bool RdrCart2LatLong(float RdrLat, float RdrLong, float RdrHt,
		     float kmN, float kmE, float kmHt,
		     float &Lat, float &Long, float &Ht) {
  GPLatLongCnvt.Set(RdrLat,RdrLong);
  bool result =
    GPLatLongCnvt.kmNE2LatLng(kmN,kmE,Lat,Long);
  Ht = RdrHt + kmHt + EarthCurvCorr(fhypot(kmE,kmN));
  return result;
}

bool RdrCart2LatLong(int stnid,
		     float kmN, float kmE, float kmHt,
		     float &Lat, float &Lng, float &Ht) {
  if ((stnid < 0) || (stnid > LastStn)) return false;
  GPLatLongCnvt.Set(StnRec[stnid].Lat(),StnRec[stnid].Lng());
  bool result =
    GPLatLongCnvt.kmNE2LatLng(kmN,kmE,Lat,Lng);
  Ht = StnRec[stnid].Ht() + kmHt + EarthCurvCorr(fhypot(kmE,kmN));
  return result;
}

bool RdrCart2LatLong(int stnid,
		     float kmN, float kmE, float kmHt,
		     LatLongHt *destLLH) {
  
  if ((stnid < 0) || (stnid > LastStn) || !destLLH) return false;
  GPLatLongCnvt.Set(StnRec[stnid].Lat(),StnRec[stnid].Lng());
  bool result =
    GPLatLongCnvt.kmNE2LatLng(kmN,kmE,destLLH->Lat,destLLH->Long);
  destLLH->Ht = StnRec[stnid].Ht() + kmHt + EarthCurvCorr(fhypot(kmE,kmN));
  return result;
}

bool LatLongKmNE2LatLong(LatLongHt *refLLH,
			 float kmN, float kmE, float kmHt,
			 LatLongHt *destLLH) 
{
  if (!refLLH || !destLLH) return false;
  GPLatLongCnvt.Set(refLLH->Lat,refLLH->Long);
  bool result =
    GPLatLongCnvt.kmNE2LatLng(kmN,kmE,destLLH->Lat,destLLH->Long);
  destLLH->Ht = refLLH->Ht + kmHt + EarthCurvCorr(fhypot(kmE,kmN));
  return result;
}

void LatLong2RdrCart(float RdrLat, float RdrLong, float RdrHt,
		     float &kmN, float &kmE, float &kmHt,
		     float Lat, float Long, float Ht) {
  GPLatLongCnvt.Set(RdrLat,RdrLong);
  GPLatLongCnvt.LatLng2kmNE(Lat,Long,kmN,kmE);
  kmHt = RdrHt + Ht + EarthCurvCorr(fhypot(kmE,kmN));
}

void LatLong2RdrCart(int stnid,
		     float &kmN, float &kmE, float &kmHt,
		     float Lat, float Long, float Ht) {
  GPLatLongCnvt.Set(StnRec[stnid].Lat(),StnRec[stnid].Lng());
  GPLatLongCnvt.LatLng2kmNE(Lat,Long,kmN,kmE);
  kmHt = StnRec[stnid].Ht() + Ht + EarthCurvCorr(fhypot(kmE,kmN));
}


/* calc APPROXIMATE distance btwn 2 Lat/Long locations */
float LatLongKmDiff(float Lat1, float Long1, float Lat2, float Long2, 
		    float *kmn, float *kme) {
  float lkmN, lkmE;

  GPLatLongCnvt.Set(Lat1,Long1);
  GPLatLongCnvt.LatLng2kmNE(Lat2,Long2,lkmN,lkmE);
  if (kmn) *kmn = lkmN;
  if (kme) *kme = lkmE;
  return fhypot(lkmN, lkmE);
}

float LatLongStnKmDiff(LatLongHt *Org, int stn) {
  float kmN, kmE;
  if ((stn < 0) || (stn > LastStn)) return 0;
  GPLatLongCnvt.Set(Org->Lat,Org->Long);
  GPLatLongCnvt.LatLng2kmNE(StnRec[stn].Lat(), StnRec[stn].Lng(), kmN,kmE);
  return fhypot(kmN, kmE);
}

void LatLongStnKmDiff(LatLongHt *Org, int stn, float &kmn, float &kme) {
  if ((stn < 0) || (stn > LastStn)) return;
  GPLatLongCnvt.Set(Org->Lat,Org->Long);
  GPLatLongCnvt.LatLng2kmNE(StnRec[stn].Lat(), StnRec[stn].Lng(), kmn,kme);
}

float StnKmDiff(int stn1, int stn2) {
  float kmN = 0, kmE = 0;
  StnKmDiff(stn1, stn2, kmN, kmE);
  return fhypot(kmN, kmE);
}

void StnKmDiff(int stn1, int stn2, float &kmn, float &kme) {
  if (
      (stn1 < 0) || (stn1 > LastStn) ||
      (stn2 < 0) || (stn2 > LastStn)) 
    return;
  GPLatLongCnvt.Set(StnRec[stn1].Lat(), StnRec[stn1].Lng());
  GPLatLongCnvt.LatLng2kmNE(StnRec[stn2].Lat(), StnRec[stn2].Lng(), kmn,kme);
  return;
}

// calculate min circular distance between two RDR_ANGLES
// ie everything in tenths of degrees
// return SIGNED angle. if +ve then ang1 is +ve clkwise rel to ang2

rdr_angle circ_angle(rdr_angle ang1, rdr_angle ang2) {
	rdr_angle temp;

	temp = ang1-ang2 % 3600;
	if (temp > 1800) temp -= 3600;	// choose closest direction
	if (temp < -1800) temp += 3600;	// choose closest direction
	return temp;
	}

LatLongtoMercator::LatLongtoMercator(float secangle, float reflong, 
    float latlimit) {
    secAngle = secangle;
    refLong = reflong;
    latLimit = latlimit;
    cosSecAngle = cos(secAngle * DEG2RAD);
    acosSecAngle = EARTH_RAD * cosSecAngle;
    XCoeff = DEG2RAD * acosSecAngle;
}

void LatLongtoMercator::SetParams(float secangle, float reflong, 
    float latlimit) {
    secAngle = secangle;
    refLong = reflong;
    latLimit = latlimit;
    cosSecAngle = cos(secAngle * DEG2RAD);
    acosSecAngle = EARTH_RAD * cosSecAngle;
    XCoeff = DEG2RAD * acosSecAngle;
}

bool LatLongtoMercator::LLtoMerc(float Lat, float Long, 
float *merc_x, float *merc_y) {

float merc_y2;
    
    if (fabs(Lat) > fabs(latLimit))
	return FALSE;
    *merc_x = XCoeff * (Long - refLong);
    *merc_y = acosSecAngle * 
	log((1 + sin(Lat * DEG2RAD)) / cos((Lat * DEG2RAD)));
    merc_y2 = acosSecAngle * 
	log(tan (0.5 * ((Lat * DEG2RAD) + HALF_PI)));
    return TRUE;
}
    
bool LatLongtoMercator::MerctoLL(float *Lat, float *Long, 
float merc_x, float merc_y) {
    
    *Long = (merc_x / XCoeff) + refLong;
    *Lat = ((atan(exp(merc_y / acosSecAngle)) * 2) - HALF_PI) * RAD2DEG;
    return TRUE;
}
    
bool LatLongtoMercator::ScaleAtLat(float Lat, float *scale) {
    if (fabs(Lat) > fabs(latLimit))
	return FALSE;
    *scale = cosSecAngle / cos(Lat * DEG2RAD);
    return TRUE;
}

bool CopyFile(char *SrcName, char *DestName) {
	char buffer[0x1000];
	FILE *SRC,*DEST;
	int rdcount,wrcount,total;

	if (!(SRC = fopen(SrcName,"r"))) {
		fprintf(stderr,"CopyFile FAILED opening SRC %s - ",SrcName);
		perror(0);
		return FALSE;
		}
	if (!(DEST = fopen(DestName,"w"))) {
		fprintf(stderr,"CopyFile FAILED opening DEST %s - ",DestName);
		perror(0);
		fclose(SRC);
		return FALSE;
		}
	total = 0;
	while ((rdcount = fread(buffer,1,0x1000,SRC)) &&
				 (wrcount = fwrite(buffer,1,rdcount,DEST)) &&
				 (rdcount == wrcount)) total += wrcount;
	fprintf(stderr,"CopyFile(%s,%s) completed (%d bytes written)\n",SrcName,DestName,total);
	fclose(SRC);
	fclose(DEST);
	return TRUE;
	}

bool AppendFile(char *SrcName, char *DestName) {
	char buffer[0x1000];
	FILE *SRC,*DEST;
	int rdcount,wrcount,total;

	if (!(SRC = fopen(SrcName,"r"))) return FALSE;
	if (!(DEST = fopen(DestName,"a"))) {
		fclose(SRC);
		return FALSE;
		}
	total = 0;
	while ((rdcount = fread(buffer,1,0x1000,SRC)) &&
				 (wrcount = fwrite(buffer,1,0x1000,DEST)) &&
				 (rdcount == wrcount)) total += wrcount;
	fprintf(stderr,"AppendFile(%s,%s) completed (%d bytes appended)\n",SrcName,DestName,total);
	fclose(SRC);
	fclose(DEST);
	return TRUE;
	}

void	SplitPName(char *inname, char *path, char *basename) {
char	*temp,*lastslash;

temp = inname;
lastslash = 0;
while	(*temp) {
	if (*temp == '/') lastslash = temp;
	temp++;
	}
if (lastslash) {
	strcpy(basename,lastslash + 1);;
	strncpy(path,inname,lastslash-inname+1);
	path[lastslash-inname+1] = 0;	//strncpy does not add null
	}
else {
	strcpy(basename,inname);
	strcpy(path,"");
	}
}

/*
 * Traverses the stringlist looking for a match against the instring
 * The special word "ANY" will return -1
 * listlast is the number of tha last string in stringlist[0..listlast]
 * case sensitive is self explanatory
 * If a match is found the integer posision in the list will be returned
 * Otherwise,  -1 is returned
 * 
 * NOTE: string matching is string position and length dependant
 *       Default is casesensitive = false
 */
int String2Int(char *instring, char **stringlist, 
		int listlast, bool casesensitive) {
bool match = FALSE;
int x = 0;

    if (!instring) return -1;
    StripTrailingWhite(instring);	    // remove any trailing white space chars
    if (strcasecmp(instring, "ANY") == 0)   // ANY returns -1
	return -1;
    if (casesensitive)
	while ((x <= listlast) && !match) {
	    match = strcmp(stringlist[x], instring) == 0;
	    if (!match) x++;
	    }
    else    // convert
	while ((x <= listlast) && !match) {
	    match = strcasecmp(stringlist[x], instring) == 0;
	    if (!match) x++;
	    }
    if (x <= listlast) return x;
    else return -1;
    }

void StripTrailingWhite(char *str) {
char *thischar;
int  length = strlen(str);
    if (!length) return;
    thischar = str + length - 1;
    while ((thischar >= str) && (isspace(*thischar))) {
	*thischar = 0;
	thischar--;
	}
    }

unsigned char dBz2char(float dBz)
{
  if(dBz > 95.5)
    return 0xff;
  if(dBz < -32.0)
    return 0;
  return ftoi((2*dBz)+64.);
}

float char2dBz(unsigned char value)
{
  return float((int)value - 64) / 2.;
}

float	dBZtoR(float dBZ, float a, float b, float rainrateclip) {   // return mms/hr
float r;					// Z=aR^b ie R=(Z/a)^1/b
	float	z = pow(10,dBZ/10);
	r = pow(z/a,1/b);
	if (rainrateclip && (r > rainrateclip))	// rainrateclip of 0 is turned off
	    r = rainrateclip;
	return(pow(z/a,1/b));
	}

float	RtodBZ(float R, float a, float b) {	// return dBZ

	if (R == 0.0) return 0;
	float z = a * pow(R, b);	        // Z=aR^b ie R=(Z/a)^1/b
	return(10 * log10(z));
	}

