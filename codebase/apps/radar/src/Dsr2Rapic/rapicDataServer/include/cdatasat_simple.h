/*
 * cdatasat_simple.h
 *
 * CDataSatSimple class
 *
 * Thread safe
 */

#ifndef __CDATA_SAT_SIMPLE_H__
#define __CDATA_SAT_SIMPLE_H__

#include "cdatasat.h"
#include "cblend.h"

class CDataSatSimple:public CDataSat {
private:
	// nav info

public:
	CDataSatSimple(void);
	int appendData(unsigned char *data);
	int writeData(unsigned char *data);
	int getData(CBlend blend,
		double lat1, double lon1, double lat2, double lon2,
		int reduction);
	int getData(int element1, int line1, int element2, int line2,
		int reduction);
	int getDataSize(int *elements, int *lines);
	double getTemp(int element, int line);
	convertNavigation(double *lat, double *lon,
		int element, int line);
	convertNavigation(int *element, int *line,
		double lat, double lon);
};


#endif

