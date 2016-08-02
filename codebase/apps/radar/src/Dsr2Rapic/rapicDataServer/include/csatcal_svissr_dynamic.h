/*
 * csatcal_svissr_dynamic.h
 *
 * SVISSR dynamic calibration class
 */

#ifndef __CSATCAL_SVISSR_DYNAMIC_H__
#define __CSATCAL_SVISSR_DYNAMIC_H__

// Local include files
#include "csatcal.h"
#include "svissr_constants.h"

class CSatCalSvissrDynamic : public CSatCal {
private:
	// Private data
	unsigned char calibrationBlock[SVISSR_CALIBRATION_BLOCK_LENGTH*
		SVISSR_DECOMMUTATION_GROUPS];

	// Private functions
	double getR4(unsigned char *ptr, int m);

public:
	// Initialisation
	CSatCalSvissrDynamic();
	virtual ~CSatCalSvissrDynamic();

	// Save/Restore state
	virtual int getFileSize();
	virtual int writeFile(int fildes);
	virtual int readFile(int fildes);

	// Returns temperature in degrees kelvin
        virtual int getTemp(unsigned int value, int channel, double *temp);
	virtual int getAlbedo(unsigned int value, int sensor, double *albedo);


	int addDecommutatedBlock(int blockNumber, unsigned char *data);
	
	virtual void Copy(CSatCalSvissrDynamic *copycal);

};


#endif



