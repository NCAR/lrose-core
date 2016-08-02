/*
 * csatnav_svissr_sm.h
 *
 * SVISSR Simplified mapping mehod navigation object
 *
 * 13/11/97 abcd Initial version
 */

#ifndef __CSATNAV_SVISSR_SM_H__
#define __CSATNAV_SVISSR_SM_H__

// Local include files
#include "csatnav.h"
#include "svissr_constants.h"


class CSatNavSvissrSm : public CSatNav {
private:
	// Data
	int setLineNumber;	// Picture flag set line number
	unsigned char simplifiedMappingBlock2[SVISSR_SMAP_BLOCK_LENGTH*
		SVISSR_DECOMMUTATION_GROUPS];	// Decommutated simplified
						// mapping block

	int decommutatedBlockCount;	// Count of decommutated blocks
	// Map of decommutated blocks
	int decommutatedBlocks[SVISSR_DECOMMUTATION_GROUPS];

	// Functions
	int getI2(unsigned char *ptr);

public:
	CSatNavSvissrSm();
	virtual ~CSatNavSvissrSm();
	int convertNavigation(double *lat, double *lon,
		int element, int line, bool usemancorrection = true);
	int convertNavigation(int *element, int *line,
		double lat, double lon, bool usemancorrection = true);
	int getFileSize();
	virtual int writeFile(int fildes);
	virtual int readFile(int fildes);

	virtual void dumpNavigationGrid(
		float minLat = -60.0, float maxLat = 60.0, 
		float minLong = 80.0, float maxLong = 200.0, 
		float latRes = 5.0, float longRes = 5.0);   // make latlongProjGrid for rawbuffer

// friend???
	int addDecommutatedBlock(int blockNumber, unsigned char *data);
	int setSetLineNumber(int inputSetLineNumber);

	virtual void Copy(CSatNavSvissrSm *copynav);
};


#endif



