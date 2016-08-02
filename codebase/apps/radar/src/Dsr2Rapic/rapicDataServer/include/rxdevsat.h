#ifndef __RXDEVSAT_H
#define __RXDEVSAT_H

#include "rpcomms.h"

class rxdevsat : public rxdevice {
    rxdevsat(Comm *CommHndl = 0, RPCommMng *comm_manager = 0);
    virtual void ~rxdevsat();
    CDataSat	*new_satdata;
        
};

#endif /*__RXDEVSAT_H */
