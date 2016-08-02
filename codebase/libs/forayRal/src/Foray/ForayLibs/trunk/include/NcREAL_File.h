#ifndef NCREAL_FILE_H
#define NCREAL_FILE_H

#include "NcRadarFile.h"

class NcREAL_File : public NcRadarFile 
{
 public:
    NcREAL_File() {
      NcRadarFile();
    }
#ifdef NOTDEF
    ~NcREAL_File() {
      ~NcRadarFile();
    }
#endif
    
 protected:
    void  write_nc_header_variables()                 throw(ForayUtility::Fault);
    void        define_nc_variables()                 throw(ForayUtility::Fault);
    void write_ground_ray()                           throw(ForayUtility::Fault);

};



#endif // NCREAL_FILE_H
