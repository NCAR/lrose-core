/*
 * csatcal.h
 *
 * Base calibration class for satellite data
 */

#ifndef __CSATCAL_H__
#define __CSATCAL_H__

#include "spinlock.h"

class CSatCal {
private:
	bool	valid;
public:
	// Initialisation
	CSatCal();
	virtual ~CSatCal();

	// Save/Restore state
	virtual int writeFile(int fildes);
	virtual int readFile(int fildes);

	// Returns temperature in degrees kelvin
        virtual int getTemp(unsigned int value, int channel, double *temp);
	virtual int getAlbedo(unsigned int value, int sensor, double *albedo);
	virtual bool IsValid();
	virtual void SetValid(bool state = true);
};


#endif



