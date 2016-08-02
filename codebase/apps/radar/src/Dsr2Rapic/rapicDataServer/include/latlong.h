#ifndef	__LATLONG_H
#define __LATLONG_H

#include <bool.h>

class LatLong {
	float lAT,lONG;
	bool Defined;
public:
	LatLong(float lat = 0, float lng = 0) {
		lAT = lat;
		lONG = lng;
		Defined = false;
		};
	void Set(float lat, float lng) {
		lAT = lat;
		lONG = lng;
		Defined = true;
		};
	void Clear() {Defined = false;};
	bool IsDefined() {return Defined;};
	float Lat() {
		if (Defined) return lAT;
		else return 0;
		};
	float Long() {
		if (Defined) return lONG;
		else return 0;
		};
	};

class kmNE {
	float kMN, kME;
public:
	kmNE(float kmn = 0, float kme = 0) {
		kMN = kmn;
		kME = kme;
		};
	void Set(float kmn, float kme) {
		kMN = kmn;
		kME = kme;
		};
	float kmN() {return kMN;};
	float kmE() {return kME;};
	};

class LatLongConvert : public LatLong {
	kmNE *kMNE;
public:
	LatLongConvert(LatLong *RefLL=0);
	virtual ~LatLongConvert();
	virtual bool kmNE2LatLng(float kmN, float kmE, float &Lat, float &Long);
	virtual bool LatLng2kmNE(float lat, float lng, float &kmN, float &kmE);
	};

#endif	/* __LATLONG_H */

