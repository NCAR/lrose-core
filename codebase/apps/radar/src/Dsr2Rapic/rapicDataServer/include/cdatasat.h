/*
 * cdatasat.h
 *
 * CDataSat class
 *
 * Thread safe
 */

#ifndef __CDATA_SAT_H__
#define __CDATA_SAT_H__

// Standard include files
#include <stdio.h>
#include <time.h>

#include <vector>
using namespace std;
// Local include files
#include "textureProj.h"
//#include "csatnav_grid.h"
#include "cdata.h"
#include "csatnav.h"
#include "cblend.h"
#include "histogram.h"

enum EDataSatType {
	SVISSR_LINES, HRPT_LINES, SUBSET
};

class CDataSat:public CData {
private:
	// TODO: consider locking around here
	int appendLineNumber;	// Line to append at
	unsigned char *data;	// TODO: something good here
	long datasize;
	EDataSatType datatype;
	int linelength, numlines;

public:
	// Data
	CSatNav navigation;	// Generic navigation module
	int	man_offset_x, man_offset_y;	// manual x/y(elem/line) offsets
	histogramClass *Histogram;
	int histogram_channel;	// channel histogram was based on
/*
	int	*histogram,		// pointer to histogram result array
		histogram_size, // number of histogram elements
		histogram_total,// total number of pixels for histogram
		histogram_channel;	// channel histogram was based on
*/
	// Functions
	CDataSat();
	~CDataSat();
	virtual int AppendData(unsigned char *data, int size);
	virtual int writeFile(char *filename, long offset = 0);
	virtual int writeFile(int fildes, long offset = 0);
	virtual int readFile(char *filename, long offset = 0);
	virtual int readFile(int fildes, long offset = 0);
	virtual int readAsda(char *filename);
	virtual int readRaw(char *filename);
	virtual int writeJpeg(char *filename, unsigned char *buffer, 
			int width, int height);
	virtual int getSubset(CDataSat *newObj, CBlend *blend,
		double lat1, double lon1, double lat2, double lon2,
		int channel, int reduction);
	virtual int getSubset(CDataSat *newObj, int element1, int line1,
		int element2, int line2, int channel, int reduction);
	virtual int getSubset(unsigned char *buf, int element1, int line1,
		int element2, int line2, int channel, int reduction);
	virtual int getSubset(unsigned char *buf,
		double lat1, double lon1,
		double lat2, double lon2,
		int channel, int reduction);

	virtual int getDataSize(int *elements, int *lines);
	virtual int getDataSize(int channel, int *elements, int *lines);
	virtual int getTemp(int element, int line, int channel, double *temp);
	virtual int getTemp(unsigned char value, int channel, double *temp);

	virtual int convertNavigation(double *lat, double *lon,
		int element, int line);
	virtual int convertNavigation(int *element, int *line,
		double lat, double lon);
        virtual int getStartTime(time_t *startTime);
	virtual bool	ProjectLLtoXY(rpProjection proj, 
				float Lat, float Long, float Height, 
			    float *projX, float *projY, float *projZ);
	virtual bool	ProjectXYtoLL(rpProjection proj, 
				float *Lat, float *Long, float *Height, 
			    float projX, float projY, float projZ);
	virtual void getHistogram(int element1 = 0, int line1 = 0,
		int element2 = 0, int line2 = 0, int channel = -1);
	virtual void getHistEqTable256(int *hist_eq_table);
	virtual void SetManNavOffsets(int elem_ofs, int line_ofs);
	virtual void ClearManNavOffsets();
};

#endif

