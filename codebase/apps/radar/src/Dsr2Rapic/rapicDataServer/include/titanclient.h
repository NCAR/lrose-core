#ifndef __TITANCLIENT_H
#define __TITANCLIENT_H

/*
 * titan_client class - simply calls rapictotitan if scan matches 
 * criteria
 */

#include "rdrscan.h"

class titanClient : public scan_client 
{
public:
    titanClient();
    virtual ~titanClient();
    virtual int	FinishedDataAvail(rdr_scan *finishedscan); // add finished scan to new sca list. usually called by other thread
    virtual void ProcessCheckedData(int maxscans = -1);// process checked scans list
    virtual void workProc();
    virtual void DataModeChanged();	// try to set data mode
    bool ScanSourceOK(rdr_scan *srcscan);
private:
protected:
};

extern titanClient *primaryTitanClient;

#endif
