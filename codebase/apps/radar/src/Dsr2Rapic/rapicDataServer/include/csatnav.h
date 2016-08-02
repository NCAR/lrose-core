/*
 * csatnav.h
 *
 * Base navigation class for satellite data
 */

#ifndef __CSATNAV_H__
#define __CSATNAV_H__

#include "spinlock.h"

class CSatNav {
private:
	bool	valid;
protected:
	int		man_elem_ofs, 
			man_line_ofs;
public:
	CSatNav();
	virtual ~CSatNav();
	virtual int convertNavigation(double *lat, double *lon,
		int element, int line, bool usecorrection = true);
	virtual int convertNavigation(int *element, int *line,
		double lat, double lon, bool usecorrection = true);

	// The filename and offset were a wank
	virtual int getFileSize() {return 0;};
	virtual int writeFile(int fildes);
	virtual int readFile(int fildes);
	virtual bool IsValid();
	virtual void SetValid(bool state = true);
	virtual void SetManOffsets(int elem_ofs, int line_ofs);
	virtual void ClearManOffsets();
};


#endif



