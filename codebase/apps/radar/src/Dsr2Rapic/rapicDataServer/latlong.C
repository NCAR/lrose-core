/*
	LatLong.c
*/

#include "latlong.h"
#include <math.h>
#ifndef sgi
#define fsin sin
#define fcos cos
#define fasin asin
#define facos acos
#define fhypot hypot
#ifdef linux 
#define fabsf fabs
#else
#define fabsf abs
#endif
#endif

extern const float EarthRad;
extern const float DegPerKM;
extern const float RdnPerDeg;
extern const float KMPerDeg;

LatLongConvert::LatLongConvert(LatLong *RefLL) {
//	RefLatLong = new  LatLong();
	kMNE = new kmNE();
	if (!RefLL) Clear();
	else Set(RefLL->Lat(),RefLL->Long());
	}

LatLongConvert::~LatLongConvert() {
//	delete RefLatLong;
	delete kMNE;
	}

/*
void LatLongConvert::Set(LatLong *reflatlong) {
	if (reflatlong)
		Set(reflatlong->Lat(),reflatlong->Long());
	else Clear();
	}
*/
/*
void LatLongConvert::Set(float lat, float lng) {
	RefLatLong->Set(lat,lng);
	}
*/
/*
void LatLongConvert::Clear() {
	RefLatLong->Clear();
	}
*/
bool LatLongConvert::kmNE2LatLng(float kmN, float kmE, float &rLat, float &rLong) {
//bool LatLongConvert::kmNE2LatLong(float kmN, float kmE, LatLong &result) {
/*
 Given RefLatLong and Kms North and East calculate result LatLong
 CALCULATIONS BASED ON
		LONGITUDE - POSITIVE EAST OF 0 DEGREES
		LATITUDE  - POSITIVE NORTH
*/

float	dLat,LatCorr,dLong,
			SinLat2,RCosL2,RCosL2SinL2;
float Lat1,Lat2,Long1,Long2;

	rLat = rLong = 0;
	if (!IsDefined()) {
		return false;
		}
	Lat1 = Lat();
	Long1 = Long();
	dLat = kmN*DegPerKM;
	Lat2 = Lat1+dLat;
	if ((Lat2 > 90) || (Lat2 < -90)) return false;
	SinLat2 = sin(Lat2*RdnPerDeg);
	RCosL2 = EarthRad * cos(Lat2*RdnPerDeg);
	RCosL2SinL2 = RCosL2 * SinLat2;
	if (fabsf(RCosL2) < fabsf(kmE)) return false;
	LatCorr = (SinLat2*fhypot(RCosL2,kmE))
						 - RCosL2SinL2;
	Lat2 -= (LatCorr*DegPerKM);
	if ((Lat2 > 90) || (Lat2 < -90)) return false;
	dLong = (kmE*DegPerKM)/cos(Lat2*RdnPerDeg);
	Long2 = fmod((Long1+dLong),360);
	if (Long2 > 180) Long2 = Long2 - 360;
	if (Long2 < -180) Long2 = Long2 + 360;
	rLat = Lat2;
	rLong = Long2;
	return true;
	}


bool LatLongConvert::LatLng2kmNE(float lat, float lng, float &kmN, float &kmE) {
/*
 Given LatLong1, LatLong2 calculate Kms North and East
*/
float
	dLat,dLong,SinLat2,CosL2,RCosL2,RCosL2SinL2,LatCorr;

	dLat = lat - Lat();
	dLong = lng - Long();
	kmN = dLat * KMPerDeg;
	CosL2 = cos(lat*RdnPerDeg);
	SinLat2 = sin(lat*RdnPerDeg);
	RCosL2 = EarthRad * CosL2;
	RCosL2SinL2 = RCosL2 * SinLat2;
	kmE = dLong * KMPerDeg * CosL2;
bool	LatLongOK = fabsf(RCosL2) > fabsf(kmE);
	LatCorr = 0;
	if (LatLongOK)
	  LatCorr = (SinLat2*fhypot(RCosL2,kmE))
	    - RCosL2SinL2;
	else return false;
	kmN += LatCorr;
	return true;
	}




